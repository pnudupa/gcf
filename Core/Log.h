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

#ifndef LOG_H
#define LOG_H

#include "GCFGlobal.h"

#include <QString>
#include <QByteArray>
#include <QTextStream>
#include <QAbstractItemModel>

namespace GCF
{

class Log;
class LogMessage;
class LogMessageBranch;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-vtables"

class LogMessageHandlerInterface
{
public:
    virtual ~LogMessageHandlerInterface() { }
    virtual void handleLogMessage(GCF::LogMessage *msg) = 0;
    virtual void print(GCF::LogMessage *msg, QTextStream &ts) = 0;
    virtual void toSupport(const QString &msg) { Q_UNUSED(msg); }
    virtual void toClipboard(const QString &msg) { Q_UNUSED(msg); }
    virtual void flush() { }
};

#pragma clang diagnostic pop

}

Q_DECLARE_INTERFACE(GCF::LogMessageHandlerInterface, "com.vcreatelogic.LogMessageHandlerInterface/1.0")

namespace GCF
{

struct LogData;
class GCF_EXPORT Log : public QAbstractItemModel,
                       virtual public GCF::LogMessageHandlerInterface
{
    Q_OBJECT
    Q_INTERFACES(GCF::LogMessageHandlerInterface)

public:
    static Log *instance();
    ~Log();

    QDateTime timestamp() const;

    void setHandler(GCF::LogMessageHandlerInterface *handler);
    GCF::LogMessageHandlerInterface *handler() const;

    void setLogFileName(const QString &logFileName);
    QString logFileName() const;

    void setLogQtMessages(bool val);
    bool isLogQtMessages() const;

    void fatal(const QString &context, const QString &message,
               const QString &details=QString()) {
        this->fatal(context, QByteArray(), message, details);
    }
    void fatal(const QString &context, const QByteArray &errorCode,
               const QString &message, const QString &details);

    void error(const QString &context, const QString &message,
               const QString &details=QString()) {
        this->error(context, QByteArray(), message, details);
    }
    void error(const QString &context, const QByteArray &errorCode,
               const QString &message, const QString &details);

    void warning(const QString &context, const QString &message,
               const QString &details=QString()) {
        this->warning(context, QByteArray(), message, details);
    }
    void warning(const QString &context, const QByteArray &errorCode,
               const QString &message, const QString &details);

    void debug(const QString &context, const QString &message,
               const QString &details=QString()) {
        this->debug(context, QByteArray(), message, details);
    }
    void debug(const QString &context, const QByteArray &errorCode,
               const QString &message, const QString &details);

    void info(const QString &context, const QString &message,
               const QString &details=QString()) {
        this->info(context, QByteArray(), message, details);
    }
    void info(const QString &context, const QByteArray &errorCode,
               const QString &message, const QString &details);
    void clear();

    QList<GCF::LogMessage*> logMessages() const;
    QString toString() const;
    void copyToClipboard();
    void copyToSupport();

    // QAbstractItemModel interface
    GCF::LogMessage *logMessage(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex parent(const QModelIndex &child) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

protected:
    Log();
    void dumpLogMessage(GCF::LogMessage *message);

    // These methods will only be used via GCF::LogMessageBranch
    GCF::LogMessage *pushBranch(const QString &context);
    GCF::LogMessage *popBranch();
    void messageUpdated(GCF::LogMessage *msg);
    void messageCreated(GCF::LogMessage *msg);
    void messageDestroyed(GCF::LogMessage *msg);

public:
    // LogMessageHandlerInterface implementation
    void handleLogMessage(GCF::LogMessage *msg);
    void print(GCF::LogMessage *msg, QTextStream &ts);

public:
    static QString defaultLogContext(const QString &fnInfo, const QString &file, int line);

private:
    friend class LogMessageBranch;
    friend class LogMessage;
    LogData *d;
};

struct LogMessageData;
class GCF_EXPORT LogMessage
{
public:
    ~LogMessage();

    GCF::LogMessage *parent() const;
    QList<GCF::LogMessage*> children() const;

    enum LogLevel
    {
        Fatal,
        Error,
        Warning,
        Debug,
        Info,
        User
    };
    int logLevel() const;
    QString context() const;
    QByteArray logCode() const;
    QString message() const;
    QString details() const;

    void clear();

protected:
    LogMessage(int level,
                 const QString &context,
                 const QByteArray &logCode,
                 const QString &msg,
                 const QString &details,
                 LogMessage *parent=nullptr);
    void setMessage(const QString &msg);

private:
    // Provided for LogDump to create a dummy-root-message
    LogMessage();

private:
    friend class Log;
    friend class LogMessageBranch;
    LogMessageData *d;
};

class GCF_EXPORT LogMessageBranch
{
public:
    LogMessageBranch(const QString &context);
    ~LogMessageBranch();

    GCF::LogMessage *branchMessage() const { return m_branchMessage; }
    void setMessage(const QString &msg) {
        if(m_branchMessage)
            m_branchMessage->setMessage(msg);
    }
    bool hasLogs() const {
        if(m_branchMessage)
            return m_branchMessage->children().count();
        return false;
    }

private:
    friend class Log;
    GCF::LogMessage *m_branchMessage;
};

}

#define GCF_DEFAULT_LOG_CONTEXT GCF::Log::defaultLogContext(Q_FUNC_INFO,__FILE__,__LINE__)

#endif // LOG_H
