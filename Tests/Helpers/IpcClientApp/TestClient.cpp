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

#include "TestClient.h"

#include <GCF3/IpcCall>
#include <QTimer>
#include <QDate>

void TestClient::testCall(const QString &fn, quint16 port)
{
    QString methodName;
    QVariantList args;

    if(fn == "noParams")
    {
        methodName = fn;

        m_expectedResult = QVariant();
        m_expectedErrorMessage.clear();
        m_expectedSuccess = true;
    }
    else if(fn == "integer")
    {
        methodName = fn;
        args.append(10);

        m_expectedResult = QVariant(10);
        m_expectedErrorMessage.clear();
        m_expectedSuccess = true;
    }
    else if(fn == "boolean")
    {
        methodName = fn;
        args.append(true);

        m_expectedResult = QVariant(true);
        m_expectedErrorMessage.clear();
        m_expectedSuccess = true;
    }
    else if(fn == "real")
    {
        methodName = fn;
        args.append(456.87);

        m_expectedResult = QVariant(456.87);
        m_expectedErrorMessage.clear();
        m_expectedSuccess = true;
    }
    else if(fn == "string")
    {
        methodName = fn;
        args.append("this is a simple string");

        m_expectedResult = QVariant( QString("this is a simple string") );
        m_expectedErrorMessage.clear();
        m_expectedSuccess = true;
    }
    else if(fn == "stringList")
    {
        methodName = fn;

        QStringList strList;
        for(int i=0; i<10; i++)
            strList << QString("String %1").arg(i+1);
        args.append( QVariant(strList) );

        m_expectedResult = QVariant(strList);
        m_expectedErrorMessage.clear();
        m_expectedSuccess = true;
    }
    else if(fn == "variant")
    {
        methodName = fn;
        args.append( QVariant(QDate::currentDate()) );

        m_expectedResult = QVariant( QDate::currentDate() );
        m_expectedErrorMessage.clear();
        m_expectedSuccess = true;
    }
    else if(fn == "variantList")
    {
        methodName = fn;

        QVariantList varList;
        varList.append( QDate::currentDate() );
        varList.append( QString("this is a simple string") );
        varList.append( true );
        varList.append( 69123.123 );

        args.append( QVariant(varList) );

        m_expectedResult = QVariant(varList);
        m_expectedErrorMessage.clear();
        m_expectedSuccess = true;
    }
    else if(fn == "variantMap")
    {
        methodName = fn;

        QVariantMap varMap;
        varMap["january"] = 31;
        varMap["february"] = 28;
        varMap["march"] = 31;
        varMap["april"] = 30;
        varMap["may"] = 31;

        args.append( QVariant(varMap) );

        m_expectedResult = QVariant(varMap);
        m_expectedErrorMessage.clear();
        m_expectedSuccess = true;
    }
    else if(fn == "byteArray")
    {
        methodName = fn;

        QByteArray bytes(4096, 'A');
        args.append(bytes);

        m_expectedResult = QVariant(bytes);
        m_expectedErrorMessage.clear();
        m_expectedSuccess = true;
    }
    else if(fn == "allParams")
    {
        methodName = fn;

        args.append(10);
        args.append(false);
        args.append(123.34);
        args.append(QString("string value"));
        args.append(QStringList() << "a" << "b" << "C");
        args.append(QVariant(QDate::currentDate()));

        QVariantList varList;
        varList.append( QDate::currentDate() );
        varList.append( QString("this is a simple string") );
        varList.append( true );
        varList.append( 69123.123 );
        args.append( QVariant(varList) );

        QVariantMap varMap;
        varMap["january"] = 31;
        varMap["february"] = 28;
        varMap["march"] = 31;
        varMap["april"] = 30;
        varMap["may"] = 31;

        args.append( QVariant(varMap) );

        QByteArray bytes(4096, 'A');
        args.append(bytes);

        m_expectedResult = QVariant(args);
        m_expectedErrorMessage.clear();
        m_expectedSuccess = true;
    }

    GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, port,
                                        "Application.TestService", methodName, args);
    connect(call, SIGNAL(done(bool)), this, SLOT(onCallDone(bool)));
}

void TestClient::onCallDone(bool success)
{
    GCF::IpcCall *call = qobject_cast<GCF::IpcCall*>(this->sender());
    if(m_expectedSuccess == success &&
       m_expectedResult == call->result() &&
       m_expectedErrorMessage == call->errorMessage())
        qApp->exit(0);
    else
        qApp->exit(1);
}
