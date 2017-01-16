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

#ifndef LOGMESSAGEHANDLER_H
#define LOGMESSAGEHANDLER_H

#include <GCF3/Log>

class EmptyLogMessageHandler : public GCF::LogMessageHandlerInterface
{
public:
    virtual void handleLogMessage(GCF::LogMessage*) { }
    virtual void print(GCF::LogMessage*, QTextStream &) { }
    virtual void toSupport(const QString &) { }
    virtual void toClipboard(const QString &) { }
    virtual void flush() { }
};

class MessageToStringHandler : public GCF::LogMessageHandlerInterface
{
    QString m_string;

public:
    void clearString() { m_string.clear(); }
    QString string() const { return m_string; }

    virtual void handleLogMessage(GCF::LogMessage *msg) {
        if( !(msg->parent() && msg->parent()->parent()) ) {
            QTextStream ts(&m_string, QIODevice::Append);
            this->print(msg, ts, 0);
        }
        GCF::Log::instance()->handleLogMessage(msg);
    }
    virtual void print(GCF::LogMessage *msg, QTextStream &ts) {
        GCF::Log::instance()->print(msg, ts);
    }

    virtual void toSupport(const QString &) { }
    virtual void toClipboard(const QString &) { }
    virtual void flush() { }

    virtual void print(GCF::LogMessage *msg, QTextStream &ts, int indent) {
        ts << QString(indent*2, QChar(' ')) << msg->context() << " : LogLevel(" << msg->logLevel() << ") "
           << msg->message() << msg->details() << "\n";
        for(int i=0; i<msg->children().count(); i++)
            this->print(msg->children().at(i), ts, indent+1);
    }
};

#endif // LOGMESSAGEHANDLER_H
