/****************************************************************************
**
** Copyright (C) VCreate Logic Private Limited, Bangalore
**
** Use of this file is limited according to the terms specified by
** VCreate Logic Private Limited, Bangalore.  Details of those terms
** are listed in licence.txt included as part of the distribution package
** of this file. This file may not be distributed without including the
** licence.txt file.
**
** Contact info@vcreatelogic.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "Application_p.h"
#include "Log.h"
#include "Application.h"

#include <QObject>
#include <QMetaObject>
#include <QMetaMethod>

GCF::Result GCF::InvokeMethodHelper::call(const QString &path, const QString &method, const QVariantList &args)
{
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                               QString("Calling %1::%2 with %3 args")
                               .arg(path).arg(method).arg(args.count()));

    QObject *object = gAppService->objectTree()->object(path);
    if(!object)
        return this->errorResult( QString("Object '%1' doesnt exist").arg(path) );

    if(method.isEmpty())
        return this->errorResult( QString("Method name not specified") );

    return this->call(object, method, args);
}

GCF::Result GCF::InvokeMethodHelper::call(QObject *object, const QString &methodName, const QVariantList &args)
{
    QByteArray method2 = methodName.toLatin1();
    const QMetaObject *mo = object->metaObject();
    for(int i=mo->methodCount()-1; i>=0; i--)
    {
        QMetaMethod method = mo->method(i);
#if QT_VERSION >= 0x050000
        if(method.name() == method2)
#else
        QByteArray signature = method.signature();
        signature = signature.left( signature.indexOf('(') );
        if(signature == method2)
#endif
            return this->call(object, method, args);
    }

    return this->errorResult( QString("Method '%1' was not found in object").arg(methodName) );
}

GCF::Result GCF::InvokeMethodHelper::call(QObject *object, const QMetaMethod &method, const QVariantList &args)
{
#if QT_VERSION >= 0x050000
    if(args.count() != method.parameterCount())
#else
    if(args.count() != method.parameterTypes().count())
#endif
        return this->errorResult( QString("Parameter count mismatch") );

    if(method.methodType() != QMetaMethod::Signal && method.access() != QMetaMethod::Public)
        return this->errorResult( QString("Cannot call a non-public method") );

    // Check for access permissions
    GCF::ObjectTreeNode *node = gAppService->objectTree()->node(object);
    if(!node && this->SecureCall)
        return this->errorResult( QString("Cannot determine access permissions for this object") );
    if(this->SecureCall && node->info().value("allowmetaaccess", false).toBool() == false)
        return this->errorResult( QString("Meta access for this object was denied") );

    return this->call2(object, method, args);
}

GCF::Result GCF::InvokeMethodHelper::isMethodInvokable(const QMetaMethod &method, QObject *object)
{
    if(!method.enclosingMetaObject())
        return GCF::Result(false, QString(), QString("Unknown method"));

    if(method.methodType() != QMetaMethod::Signal && method.access() != QMetaMethod::Public)
        return this->errorResult( QString("Non-public methods cannot be invoked") );

    if(object && this->SecureCall)
    {
        GCF::ObjectTreeNode *node = gAppService->objectTree()->node(object);
        if(!node && this->SecureCall)
            return this->errorResult( QString("Cannot determine access permissions for this object") );
        if(this->SecureCall && node->info().value("allowmetaaccess", false).toBool() == false)
            return this->errorResult( QString("Meta access for this object was denied") );
    }

    static QList<int> supportedTypes = QList<int>() << QMetaType::Int <<
        QMetaType::Bool << QMetaType::Double << QMetaType::QString <<
        QMetaType::QStringList << QMetaType::QVariant << QMetaType::QVariantList <<
        QMetaType::QVariantMap << QMetaType::QByteArray;

    QList<QByteArray> paramTypes = method.parameterTypes();
    Q_FOREACH(QByteArray paramType, paramTypes)
    {
        int typeId = QMetaType::type(paramType);
        if(supportedTypes.contains(typeId))
            continue;

        QString typeName = QString::fromLatin1(paramType);
        return this->errorResult(QString("Parameter type '%1' not supported").arg(typeName));
    }

    int returnTypeId = QMetaType::type(method.typeName());
    if(returnTypeId == QMetaType::Void ||
       supportedTypes.contains(returnTypeId) ||
       !qstrcmp(method.typeName(), "GCF::Result"))
        return true;

    return this->errorResult(QString("Return type '%1' not supported").arg(method.typeName()));
}

struct CallData
{
    CallData(int paramCount)
        : genericArgs(paramCount, nullptr) { }

    ~CallData() {
        qDeleteAll(ints);
        qDeleteAll(bools);
        qDeleteAll(doubles);
        qDeleteAll(strings);
        qDeleteAll(stringLists);
        qDeleteAll(variants);
        qDeleteAll(variantLists);
        qDeleteAll(variantMaps);
        qDeleteAll(byteArrays);
        qDeleteAll(results);
        qDeleteAll(genericArgs);
    }

    QList<int*> ints;
    QList<bool*> bools;
    QList<double*> doubles;
    QList<QString*> strings;
    QList<QStringList*> stringLists;
    QList<QVariant*> variants;
    QList<QVariantList*> variantLists;
    QList<QVariantMap*> variantMaps;
    QList<QByteArray*> byteArrays;
    QList<GCF::Result*> results;
    QVector<QGenericArgument*> genericArgs;
    // Note to developer / maintainer of this code:
    // If support for more types are added, then the GCF::InvokeMethodHelper::isMethodInvokable()
    // function should be updated as well.

    bool add(int index, int type, const QVariant &arg);
};

bool CallData::add(int index, int type, const QVariant &value)
{
    switch(type)
    {
    case QMetaType::Int:
        ints.append(new int);
        *(ints.last()) = value.toInt();
        genericArgs[index] = new QGenericArgument("int", ints.last());
        break;
    case QMetaType::Bool:
        bools.append(new bool);
        *(bools.last()) = value.toBool();
        genericArgs[index] = new QGenericArgument("bool", bools.last());
        break;
    case QMetaType::Double:
        doubles.append(new double);
        *(doubles.last()) = value.toDouble();
        genericArgs[index] = new QGenericArgument("double", doubles.last());
        break;
    case QMetaType::QString:
        strings.append(new QString);
        *(strings.last()) = value.toString();
        genericArgs[index] = new QGenericArgument("QString", strings.last());
        break;
    case QMetaType::QStringList:
        stringLists.append(new QStringList);
        *(stringLists.last()) = value.toStringList();
        genericArgs[index] = new QGenericArgument("QStringList", stringLists.last());
        break;
    case QMetaType::QVariant:
        variants.append(new QVariant);
        *(variants.last()) = value;
        genericArgs[index] = new QGenericArgument("QVariant", variants.last());
        break;
    case QMetaType::QVariantList:
        variantLists.append(new QVariantList);
        *(variantLists.last()) = value.toList();
        genericArgs[index] = new QGenericArgument("QVariantList", variantLists.last());
        break;
    case QMetaType::QVariantMap:
        variantMaps.append(new QVariantMap);
        *(variantMaps.last()) = value.toMap();
        genericArgs[index] = new QGenericArgument("QVariantMap", variantMaps.last());
        break;
    case QMetaType::QByteArray:
        byteArrays.append(new QByteArray);
        *(byteArrays.last()) = value.toByteArray();
        genericArgs[index] = new QGenericArgument("QByteArray", byteArrays.last());
        break;
    default:
        if(type == qMetaTypeId<GCF::Result>())
        {
            results.append(new GCF::Result);
            *(results.last()) = value.value<GCF::Result>();
            genericArgs[index] = new QGenericArgument("GCF::Result", results.last());
        }
        else
            genericArgs[index] = nullptr;
        break;
    }

    return genericArgs[index] != nullptr;
}

GCF::Result GCF::InvokeMethodHelper::call2(QObject *object, const QMetaMethod &method, const QVariantList &args)
{
    /*
     * We support only the following types in parameters.
     * - int, bool, double, QString, QStringList, QVariantList, QVariantMap, QByteArray, GCF::Result
     * Custom types must be represented using any of the supported types above.
     */
    CallData callData(args.count()+1);

    // Construct all parameters as QGenericArgument
    for(int i=0; i<args.count(); i++)
    {
        QVariant arg = args.at(i);
        int argType = QMetaType::User;
#if QT_VERSION >= 0x050000
        argType = method.parameterType(i);
#else
        argType = QMetaType::type(method.parameterTypes().at(i));
#endif

        if(argType != (int)arg.type() && argType != QMetaType::QVariant && !arg.canConvert(QVariant::Type(argType)))
            return this->errorResult( QString("Invalid parameter type. Expecting '%1' but found '%2'")
                             .arg(QMetaType::typeName(argType)).arg(arg.typeName()));

        if(argType != int(arg.type()) && argType != QMetaType::QVariant)
            arg.convert(QVariant::Type(argType));

        if(argType == qMetaTypeId<GCF::Result>() || !callData.add(i, argType, arg))
            return this->errorResult( QString("Argument type '%2' not supported")
                             .arg( QString::fromLatin1(method.parameterTypes().at(i)) ) );
    }

    // Construct return value as generic-argument
    int returnType = QMetaType::User;
#if QT_VERSION >= 0x050000
    returnType = method.returnType();
#else
    returnType = method.typeName() ? QMetaType::type( method.typeName() ) : QMetaType::Void;
#endif
    if(returnType != QMetaType::Void)
    {
        if( !callData.add(args.count(), returnType, QVariant()) )
            return this->errorResult( QString("Return type '%1' not supported")
                             .arg( QString::fromLatin1(method.typeName())) );
    }

#define G_RETURN_ARG *( (QGenericReturnArgument*)(callData.genericArgs.last()) )
#define G_ARG(index) (index >= callData.genericArgs.count()-1) ? QGenericArgument() : *( callData.genericArgs.at(index) )

    // Make the call using QMetaMethod::invokeMethod()
    bool success = false;
    if(callData.genericArgs.last())
        // This function provides a return value
        success = method.invoke(object, G_RETURN_ARG, G_ARG(0), G_ARG(1), G_ARG(2),
                                G_ARG(3), G_ARG(4), G_ARG(5), G_ARG(6), G_ARG(7),
                                G_ARG(8), G_ARG(9));
    else
        // This function doesnt provide a return value
        success = method.invoke(object, G_ARG(0), G_ARG(1), G_ARG(2),
                                G_ARG(3), G_ARG(4), G_ARG(5), G_ARG(6), G_ARG(7),
                                G_ARG(8), G_ARG(9));

    if(!success)
    {
        /*
         * This can happen if the arguments passed to a method are more or less than
         * what is expected by the method OR are presented in a different order than
         * what is expected by the method.
         *
         * We are checking for both these error conditions before itself. So, ideally
         * we should never reach here.
         *
         * We can never write a test case to check whether the following error message
         * is raised or not.
         */
#if QT_VERSION >= 0x050000
        return this->errorResult( QString("Error while invoking method %1").arg(QString::fromLatin1(method.methodSignature())) );
#else
        return this->errorResult( QString("Error while invoking method %1").arg(QString::fromLatin1(method.signature())) );
#endif
    }

    // If we are here, then the call was successful.
    switch(returnType)
    {
    case QMetaType::Int:
        return this->result( *(callData.ints.last()) );

    case QMetaType::Bool:
        return this->result( *(callData.bools.last()) );

    case QMetaType::Double:
        return this->result( *(callData.doubles.last()) );

    case QMetaType::QString:
        return this->result( *(callData.strings.last()) );

    case QMetaType::QStringList:
        return this->result( *(callData.stringLists.last()) );

    case QMetaType::QVariant:
        return this->result( *(callData.variants.last()) );

    case QMetaType::QVariantList:
        return this->result( *(callData.variantLists.last()) );

    case QMetaType::QVariantMap:
        return this->result( *(callData.variantMaps.last()) );

    case QMetaType::QByteArray:
        return this->result( *(callData.byteArrays.last()) );

    case QMetaType::Void:
        return this->result(QVariant());

    default:
        if(returnType == qMetaTypeId<GCF::Result>())
        {
            GCF::Result &result = *(callData.results.last());
            if(result.isSuccess())
                return this->result(result.data());

            QString errMsg = QString("%1: %2").arg(result.code()).arg(result.message());
            return this->errorResult(errMsg);
        }
    }

    // This should ideally never happen!
    return this->errorResult( QString("Return type '%1' not supported")
                     .arg( QString::fromLatin1(method.typeName())) );
}
