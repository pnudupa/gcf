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

#include "Json.h"

#if QT_VERSION >= 0x050000
#include <QJsonDocument>

Json::Json() { }
Json::~Json() { }

QVariant Json::parse(const QByteArray &bytes) const
{
    if(bytes.isEmpty())
        return QVariant();

    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    return doc.toVariant();
}

QByteArray Json::serialize(const QVariant& value) const
{
    QJsonDocument doc = QJsonDocument::fromVariant(value);
    return doc.toJson();
}
#else
#include <QScriptEngine>
#include <QScriptValue>

struct JsonData
{
    QScriptEngine engine;
    QScriptValue parseFn;
    QScriptValue serializeFn;
};

Json::Json()
{
    d = new JsonData;

    const QString script = "function parse_json(string) { return JSON.parse(string); }\n"
                           "function serialize_json(object) { return JSON.stringify(object); }";
    QScriptValue result = d->engine.evaluate(script);

    d->parseFn = d->engine.globalObject().property("parse_json");
    d->serializeFn = d->engine.globalObject().property("serialize_json");
}

Json::~Json()
{
    delete d;
}

QVariant Json::parse(const QByteArray &bytes) const
{
    if(bytes.isEmpty())
        return QVariant();

    QScriptValue result = d->parseFn.call(QScriptValue(), QScriptValueList() << QScriptValue(QString(bytes)));
    return result.toVariant();
}

QScriptValue CreateValue(const QVariant& value, QScriptEngine& engine)
{
    if(value.type() == QVariant::Map)
    {
        QScriptValue obj = engine.newObject();

        QVariantMap map = value.toMap();
        QVariantMap::const_iterator it = map.begin();
        QVariantMap::const_iterator end = map.end();
        while(it != end)
        {
            obj.setProperty( it.key(), ::CreateValue(it.value(), engine) );
            ++it;
        }

        return obj;
    }

    if(value.type() == QVariant::List)
    {
        QVariantList list = value.toList();
        QScriptValue array = engine.newArray(list.length());
        for(int i=0; i<list.count(); i++)
            array.setProperty(i, ::CreateValue(list.at(i),engine));

        return array;
    }

    switch(value.type())
    {
    case QVariant::String:
        return QScriptValue(value.toString());
    case QVariant::Int:
        return QScriptValue(value.toInt());
    case QVariant::UInt:
        return QScriptValue(value.toUInt());
    case QVariant::Bool:
        return QScriptValue(value.toBool());
    case QVariant::ByteArray:
        return QScriptValue(QLatin1String(value.toByteArray()));
    case QVariant::Double:
        return QScriptValue((qsreal)value.toDouble());
    default:
        break;
    }

    if(value.isNull())
        return QScriptValue(QScriptValue::NullValue);

    return engine.newVariant(value);
}

QByteArray Json::serialize(const QVariant& value) const
{
    QScriptValue arg = ::CreateValue(value, d->engine);
    QScriptValue result = d->serializeFn.call(QScriptValue(), QScriptValueList() << arg);
    QString resultString = result.toString();
    return resultString.toLatin1();
}
#endif
