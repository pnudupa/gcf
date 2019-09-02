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

#include "Log.h"
#include "Version.h"

#include <QUrl>
#include <QDir>
#include <QFile>
#include <QStack>
#include <QMutex>
#include <QtDebug>
#include <QSysInfo>
#include <QDateTime>
#include <QStringList>
#include <QBasicTimer>
#include <QTextStream>
#include <QMutexLocker>

#if QT_VERSION >= 0x050900
#include <QOperatingSystemVersion>
#endif

///////////////////////////////////////////////////////////////////////////////

/**
\class GCF::LogMessageHandlerInterface Log.h <GCF3/Log>
\ingroup gcf_core

\brief This interface is used to represent the class that can do the actual logging.

A class that implements this interface can decide the way in which the application logs
the different log messages generated in the application via the \ref GCF::Log mechanism.
By default \ref GCF::Log implements this interface and logs the messages to an appropriate
log file.
<BR/>
You can implement this interface in a class and register it with \ref GCF::Log as the
message handler for the application, to control the way in which messages are logged. For example
you could write a LogMessageHandler that would write these log messages on to another application
via sockets, where the actual logging happens.
*/

/**
\fn void GCF::LogMessageHandlerInterface::handleLogMessage(GCF::LogMessage *msg)

\ref GCF::Log would call this function of the handler with the reported message so that the handler can
log it appropriately.
*/

/**
\fn void GCF::LogMessageHandlerInterface::print(GCF::LogMessage *msg, QTextStream &ts)

\ref GCF::Log would call this function of the handler with the reported message so that the handler can
stream the log in appropriate print format to the text stream passed.
*/

/**
\fn void GCF::LogMessageHandlerInterface::toSupport(const QString &msg)

\ref GCF::Log would call this function of the handler with the reported message so that the handler can
report the \c msg to support.
*/

/**
\fn void GCF::LogMessageHandlerInterface::toClipboard(const QString &msg)

\ref GCF::Log would call this function of the handler with the reported message so that the handler can
dump \c msg to clipboard.
*/

/**
\fn void GCF::LogMessageHandlerInterface::flush()

\ref GCF::Log would call this function of the handler with the reported message so that the handler can
perform a flush operation.
*/

/**
\class GCF::Log Log.h <GCF3/Log>
\brief This class offers means to log a hierarchy of messages and process them
\ingroup gcf_core


This is the only singleton class in all of GCF3. Through this single instance,
users can log warning, debug, error and info messages. Although actual logging
takes place via \ref GCF::LogMessageHandlerInterface, set using \ref setHandler()
method; this class offers a simple API for feeding log messages and passing
the buck to the handler when appropriate. By default an internal log-handler
is registered as handler with \ref GCF::Log; which dumps all log messages to
a log file. You can enable logging of \c qDebug(), \c qWarning(), \c qFatal()
and \c qCritical() messages by calling \ref setLogQtMessages() method.
Logging can be done from multiple threads as well. Forwarded calls to
\ref GCF::LogMessageHandlerInterface are serialized.

Usage:
\code
void function()
{
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Simple log message");
}
\endcode

The \c GCF_DEFAULT_LOG_CONTEXT constructs a default-context string based on the
current function and line-number. Contexts help in figuring out where the
log message has originated. You can use \c GCF_DEFAULT_LOG_CONTEXT or any other
context string of your choice.

Each message dumped via \ref info(), \ref debug(), \ref warning(), \ref error(), and
\ref fatal() is stored as a \ref GCF::LogMessage until the \ref handler() handles the
message. After the message is handled, it gets deleted.

Logs can be hierarchical. You can create a log-hierarchy by creating instances
of the \ref GCF::LogMessageBranch class. Branches are internally treated as log
messages. Example:

\code
void anotherFunction();
void yetAnotherFunction();

void function()
{
    GCF::LogMessageBranch branch("function() branch");
    ::anotherFunction();
    ::yetAnotherFunction();
}

void anotherFunction()
{
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Simple log message");
}

void yetAnotherFunction()
{
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Simple log message");
}
\endcode

In the above example, log messages from \c anotherFunction() and \c yetAnotherFunction()
are dumped into the log branch created within \c function(). When the log messages
are dumped on to a log-file (or shown in a dialog box); the hierarchy is captured using
indentation (or tree-view).

Hierarchichal logs are useful to understand the context in which a log message arose.
For example, it might be useful to know whether the log message generated by \c yetAnotherFunction()
happened in the context of a \c function() call or a direct call.

\note Branches that have no log messages are automatically deleted.
\note Branches or messages, once dumped into the log file are automatically deleted.
\sa GCF::LogMessage
 */

namespace GCF
{
struct LogData
{
    LogData() : rootMessage(nullptr), timestamp(QDateTime::currentDateTime()),
        logQtMessages(false), versionLogged(false) { }

    QMutex messageMutex;
    GCF::LogMessage *rootMessage;
    QStack<GCF::LogMessage*> stack;
    QMutex handlerMutex;
    LogMessageHandlerInterface *handler;
    QString logFile;
    QDateTime timestamp;
    bool logQtMessages;
    bool versionLogged;
#ifdef Q_OS_MAC
    char unused[6]; // Padding added to align this struct
#endif

    GCF::LogMessage *currentBranch() const {
        if(this->stack.count()) return this->stack.top();
        return this->rootMessage;
    }
};

GCF_INTERFACE_BEGIN
class Log2 : public Log
{
public:
    Log2() { }
    ~Log2() { }
};
GCF_INTERFACE_END

#if QT_VERSION >= 0x050000
void qtMsgToLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString logMsg = QString("%1 (%2:%3 in %4)").arg(localMsg.constData())
            .arg(context.file).arg(context.line).arg(context.function);

    switch (type)
    {
    case QtInfoMsg:
        GCF::Log::instance()->debug("QtInfoMsg", logMsg);
        break;
    case QtDebugMsg:
        GCF::Log::instance()->debug("QtDebugMsg", logMsg);
        break;
    case QtWarningMsg:
        GCF::Log::instance()->warning("QtWarningMsg", logMsg);
        break;
    case QtCriticalMsg:
        GCF::Log::instance()->debug("QtCriticalMsg", logMsg);
        break;
    case QtFatalMsg:
        GCF::Log::instance()->fatal("QtFatalMsg", logMsg);
        abort();
    }
}
#else
void qtMsgToLogHandler(QtMsgType type, const char *msg)
{
    QString logMsg = QString::fromLatin1(msg);
    switch (type)
    {
    case QtDebugMsg:
        GCF::Log::instance()->debug("QtDebugMsg", logMsg);
        break;
    case QtWarningMsg:
        GCF::Log::instance()->warning("QtWarningMsg", logMsg);
        break;
    case QtCriticalMsg:
        GCF::Log::instance()->debug("QtCriticalMsg", logMsg);
        break;
    case QtFatalMsg:
        GCF::Log::instance()->fatal("QtFatalMsg", logMsg);
        abort();
    }
}
#endif
}

Q_GLOBAL_STATIC(GCF::Log2, GCFGlobalLog)

/**
 * \return pointer to the only instance of this class in the application
 */
GCF::Log *GCF::Log::instance()
{
    return GCFGlobalLog();
}

/**
 * \internal
 */
GCF::Log::Log()
{
    d = new GCF::LogData;
    d->handler = this;
    d->rootMessage = new LogMessage;
}

/**
 * \internal
 */
GCF::Log::~Log()
{
    delete d;
}

/**
 * \internal
 */
QDateTime GCF::Log::timestamp() const
{
    /*
     * This function could have been implemented to return GCF::Application::launchTimestamp().
     * We are not doing that on purpose. What if someone wanted to log before creation of the
     * GCF::Application object?
     */
    return d->timestamp;
}

/**
 * Sets the handler to which all log messages are forwarded.
 *
 * \param handler pointer to the handler. If null, the default internal handler is used.
 */
void GCF::Log::setHandler(GCF::LogMessageHandlerInterface *handler)
{
    if(d->handler == handler)
        return;

    if(handler)
        d->handler = handler;
    else
        d->handler = this;

    d->versionLogged = false;
}

/**
 * \return pointer to the handler used by the log
 */
GCF::LogMessageHandlerInterface *GCF::Log::handler() const
{
    return d->handler;
}

/**
 * Sets the file-name into which all logs are dumped by the default handler.
 *
 * \param logFile name of the file into which log messages are dumped by the
 * default handler
 */
void GCF::Log::setLogFileName(const QString &logFile)
{
    d->logFile = logFile;
}

/**
 * \return complete path to the file into which log messages are dumped by the
 * default handler.
 */
QString GCF::Log::logFileName() const
{
    if(d->logFile.isEmpty())
    {
        QString dt = d->timestamp.toString("d_M_yyyy_h_m_s_z");
        d->logFile = QString("%1/Logs/%2.txt")
                .arg( GCF::applicationDataDirectoryPath() )
                .arg( dt );

        // Ensure that the Logs directory exists!
        QDir dir( GCF::applicationDataDirectoryPath() );
        if( !dir.cd("Logs") )
            dir.mkdir("Logs");
    }

    return d->logFile;
}

/**
 * This function can be used to enable/disable logging of \c qDebug(), \c qWarning(),
 * \c qFatal() and \c qCritical() messages to the log. By default logging of \c QtDebug
 * messages is disabled.
 *
 * \param val true if QtDebug messages need to be logged, false otherwise.
 */
void GCF::Log::setLogQtMessages(bool val)
{
    if(d->logQtMessages == val)
        return;

    d->logQtMessages = val;
    if(val)
#if QT_VERSION >= 0x050000
        qInstallMessageHandler(GCF::qtMsgToLogHandler);
#else
        qInstallMsgHandler(GCF::qtMsgToLogHandler);
#endif
    else
#if QT_VERSION >= 0x050000
        qInstallMessageHandler(nullptr);
#else
        qInstallMsgHandler(nullptr);
#endif
}

/**
 * \return true if \c QtDebug messages are logged. False otherwise.
 * \sa setLogQtMessages()
 */
bool GCF::Log::isLogQtMessages() const
{
    return d->logQtMessages;
}

/**
 * Logs a fatal message into the current branch
 *
 * \param context a string representing the context of the message. Use of
 * \c GCF_DEFAULT_LOG_CONTEXT for this parameter is recommended
 * \param errorCode a code associated with this message
 * \param message a brief one liner associated with this message
 * \param details a detailed multi-line text associated with this message
 */
void GCF::Log::fatal(const QString &context, const QByteArray &errorCode,
                     const QString &message, const QString &details)
{
    QMutexLocker locker(&d->messageMutex);
    GCF::LogMessage *msg = new GCF::LogMessage(GCF::LogMessage::Fatal,
                                               context, errorCode, message, details,
                                               d->currentBranch());
    this->dumpLogMessage(msg);
}

/**
 * Logs a error message into the current branch
 *
 * \param context a string representing the context of the message. Use of
 * \c GCF_DEFAULT_LOG_CONTEXT for this parameter is recommended
 * \param errorCode a code associated with this message
 * \param message a brief one liner associated with this message
 * \param details a detailed multi-line text associated with this message
 */
void GCF::Log::error(const QString &context, const QByteArray &errorCode,
                     const QString &message, const QString &details)
{
    QMutexLocker locker(&d->messageMutex);
    GCF::LogMessage *msg = new GCF::LogMessage(GCF::LogMessage::Error,
                                               context, errorCode, message, details,
                                               d->currentBranch());
    this->dumpLogMessage(msg);
}

/**
 * Logs a warning message into the current branch
 *
 * \param context a string representing the context of the message. Use of
 * \c GCF_DEFAULT_LOG_CONTEXT for this parameter is recommended
 * \param errorCode a code associated with this message
 * \param message a brief one liner associated with this message
 * \param details a detailed multi-line text associated with this message
 */
void GCF::Log::warning(const QString &context, const QByteArray &errorCode,
                       const QString &message, const QString &details)
{
    QMutexLocker locker(&d->messageMutex);
    GCF::LogMessage *msg = new GCF::LogMessage(GCF::LogMessage::Warning,
                                               context, errorCode, message, details,
                                               d->currentBranch());
    this->dumpLogMessage(msg);
}

/**
 * Logs a warning message into the current branch
 *
 * \param context a string representing the context of the message. Use of
 * \c GCF_DEFAULT_LOG_CONTEXT for this parameter is recommended
 * \param errorCode a code associated with this message
 * \param message a brief one liner associated with this message
 * \param details a detailed multi-line text associated with this message
 */
void GCF::Log::debug(const QString &context, const QByteArray &errorCode,
                     const QString &message, const QString &details)
{
    QMutexLocker locker(&d->messageMutex);
    GCF::LogMessage *msg = new GCF::LogMessage(GCF::LogMessage::Debug,
                                               context, errorCode, message, details,
                                               d->currentBranch());
    this->dumpLogMessage(msg);
}

/**
 * Logs a information message into the current branch
 *
 * \param context a string representing the context of the message. Use of
 * \c GCF_DEFAULT_LOG_CONTEXT for this parameter is recommended
 * \param errorCode a code associated with this message
 * \param message a brief one liner associated with this message
 * \param details a detailed multi-line text associated with this message
 */
void GCF::Log::info(const QString &context, const QByteArray &errorCode,
                    const QString &message, const QString &details)
{
    QMutexLocker locker(&d->messageMutex);
    GCF::LogMessage *msg = new GCF::LogMessage(GCF::LogMessage::Info,
                                               context, errorCode, message, details,
                                               d->currentBranch());
    this->dumpLogMessage(msg);
}

/**
 * \fn void GCF::Log::info(const QString &context, const QString &message, const QString &details=QString())
 *
 * Logs a information message into the current branch
 *
 * \param context a string representing the context of the message. Use of
 * \c GCF_DEFAULT_LOG_CONTEXT for this parameter is recommended
 * \param message a brief one liner associated with this message
 * \param details a detailed multi-line text associated with this message
 */

/**
 * \fn void GCF::Log::debug(const QString &context, const QString &message, const QString &details=QString())
 *
 * Logs a debug message into the current branch
 *
 * \param context a string representing the context of the message. Use of
 * \c GCF_DEFAULT_LOG_CONTEXT for this parameter is recommended
 * \param message a brief one liner associated with this message
 * \param details a detailed multi-line text associated with this message
 */

/**
 * \fn void GCF::Log::warning(const QString &context, const QString &message, const QString &details=QString())
 *
 * Logs a warning message into the current branch
 *
 * \param context a string representing the context of the message. Use of
 * \c GCF_DEFAULT_LOG_CONTEXT for this parameter is recommended
 * \param message a brief one liner associated with this message
 * \param details a detailed multi-line text associated with this message
 */

/**
 * \fn void GCF::Log::error(const QString &context, const QString &message, const QString &details=QString())
 *
 * Logs a error message into the current branch
 *
 * \param context a string representing the context of the message. Use of
 * \c GCF_DEFAULT_LOG_CONTEXT for this parameter is recommended
 * \param message a brief one liner associated with this message
 * \param details a detailed multi-line text associated with this message
 */

/**
 * \fn void GCF::Log::fatal(const QString &context, const QString &message, const QString &details=QString())
 *
 * Logs a fatal message into the current branch
 *
 * \param context a string representing the context of the message. Use of
 * \c GCF_DEFAULT_LOG_CONTEXT for this parameter is recommended
 * \param message a brief one liner associated with this message
 * \param details a detailed multi-line text associated with this message
 */

/**
 * \internal
 */
void GCF::Log::clear()
{
    QMutexLocker locker(&d->messageMutex);
    d->rootMessage->clear();
    d->stack.clear();
}

/**
 * \return list of top-level log messages and branches.
 */
QList<GCF::LogMessage*> GCF::Log::logMessages() const
{
    return d->rootMessage->children();
}

/**
 * \return a string representation of the log
 */
QString GCF::Log::toString() const
{
    QString text;

    {
        QTextStream ts(&text, QIODevice::WriteOnly);
        ts << "Application: " << qApp->applicationName() << "("
           << qApp->applicationVersion() << ") from "<< qApp->organizationName() << "\n";
        ts << "ProcessID: " << qApp->applicationPid() << "\n";
        ts << "Date/Time: \n"
           << "    Local: " << QDateTime::currentDateTime().toString() << "\n"
           << "    UTC  : " << QDateTime::currentDateTimeUtc().toString() << "\n";
        ts << "Platform: ";
#ifdef Q_WS_WIN
        switch(QSysInfo::windowsVersion())
        {
        case QSysInfo::WV_XP: ts << "Windows XP"; break;
        case QSysInfo::WV_2003: ts << "Windows 2003"; break;
        case QSysInfo::WV_VISTA: ts << "Windows Vista"; break;
        case QSysInfo::WV_WINDOWS7: ts << "Windows 7"; break;
        default: ts << "Windows";
        }
#endif
#ifdef Q_OS_LINUX
        ts << "GNU/Linux";
#endif
#ifdef Q_OS_MAC
        static const QStringList osNames = QStringList() << "Cheetah" << "Puma"
                                                         << "Jaguar" << "Panther" << "Tiger" << "Leapord"
                                                         << "Snow Leapord" << "Lion" << "Moutain Lion" << "Mavericks";
#if QT_VERSION >= 0x050000
    #if QT_VERSION >= 0x050900
            int osVersion = QOperatingSystemVersion::current().minorVersion();
    #else
            int osVersion = int( QSysInfo::macVersion() ) - int ( QSysInfo::MV_10_0 );
    #endif
#else
        int osVersion = int( QSysInfo::MacintoshVersion ) - int ( QSysInfo::MV_10_0 );
#endif
        ts << "Mac OSX 10." << osVersion;
        if(osVersion >= 0 && osVersion < osNames.count())
            ts << " (" << osNames.at(osVersion) << ")";
#endif
        ts << "\n\n";

        QMutexLocker locker(&d->handlerMutex);
        for(int i=0; i<d->rootMessage->children().count(); i++)
            d->handler->print(d->rootMessage->children().at(i), ts);
    }

    return text;
}

/**
 * Requests the handler to copy the string representation of this
 * log to the clipboard.
 *
 * \note Actual copy of log-messages to clipboard happens only if
 * the handler supports it.
 */
void GCF::Log::copyToClipboard()
{
    QString text = this->toString();

    QMutexLocker locker(&d->handlerMutex);
    d->handler->toClipboard(text);
}

/**
 * Requests the handler to submit the string representation of this
 * log support (via email or otherwise).
 *
 * \note Actual submit of log-messages to support happens only if
 * the handler supports it.
 */
void GCF::Log::copyToSupport()
{
    QString text = this->toString();

    QMutexLocker locker(&d->handlerMutex);
    d->handler->toSupport(text);
}

/**
 * \internal
 */
GCF::LogMessage *GCF::Log::logMessage(const QModelIndex &index) const
{
    if(index.isValid())
    {
        GCF::LogMessage *msg = static_cast<GCF::LogMessage*>(index.internalPointer());
        return msg;
    }

    return nullptr;
}

/**
 * \internal
 */
int GCF::Log::rowCount(const QModelIndex &parent) const
{
    QMutexLocker locker(&d->messageMutex);
    if(parent.isValid())
    {
        GCF::LogMessage *msg = static_cast<GCF::LogMessage*>(parent.internalPointer());
        return msg->children().count();
    }

    return d->rootMessage->children().count();
}

/**
 * \internal
 */
int GCF::Log::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
}

/**
 * \internal
 */
QVariant GCF::Log::data(const QModelIndex &index, int role) const
{
    QMutexLocker locker(&d->messageMutex);
    if(!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    GCF::LogMessage *msg = static_cast<GCF::LogMessage*>(index.internalPointer());
    switch(index.column())
    {
    case 0: return msg->logLevel();
    case 1: return msg->context();
    case 2: return msg->logCode();
    case 3: return msg->message();
    case 4: return msg->details();
    default: break;
    }

    return QVariant();
}

/**
 * \internal
 */
QModelIndex GCF::Log::parent(const QModelIndex &child) const
{
    QMutexLocker locker(&d->messageMutex);
    if(!child.isValid() || child.column() != 0)
        return QModelIndex();

    GCF::LogMessage *msg = static_cast<GCF::LogMessage*>(child.internalPointer());
    if(msg->parent() == d->rootMessage)
        return QModelIndex();

    GCF::LogMessage *parentMsg = msg->parent();
    int parentRow = parentMsg->parent()->children().indexOf(parentMsg);
    return this->createIndex(parentRow, 0, parentMsg);
}

/**
 * \internal
 */
QModelIndex GCF::Log::index(int row, int column, const QModelIndex &parent) const
{
    QMutexLocker locker(&d->messageMutex);
    GCF::LogMessage *parentMsg = nullptr;
    if(parent.isValid())
        parentMsg = static_cast<GCF::LogMessage*>(parent.internalPointer());
    else
        parentMsg = d->rootMessage;

    if(row < 0 || row >= parentMsg->children().count())
        return QModelIndex();

    return this->createIndex(row, column, parentMsg->children().at(row));
}

/**
 * \internal
 */
QVariant GCF::Log::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch(section)
        {
        case 0: return tr("Error Level");
        case 1: return tr("Context");
        case 2: return tr("Code");
        case 3: return tr("Message");
        case 4: return tr("Details");
        default: break;
        }
    }

    return QVariant();
}

/**
 * \internal
 */
void GCF::Log::dumpLogMessage(GCF::LogMessage *message)
{
    QMutexLocker locker(&d->handlerMutex);
    if(!message)
        return;

    LogMessageHandlerInterface *handler = d->handler;
    if(handler)
    {
        if(!d->versionLogged)
        {
            QString ctx = QString("GCF Logger");
            QString msg = QString("Logging using GCF-%1").arg(GCF::version());
            GCF::LogMessage *versionMsg
                    = new GCF::LogMessage(GCF::LogMessage::Info, ctx,
                                          QByteArray(), msg, QString(),
                                          d->rootMessage);
            handler->handleLogMessage(versionMsg);
            d->versionLogged = true;
        }

        handler->handleLogMessage(message);
    }
}

/**
 * \internal
 */
GCF::LogMessage *GCF::Log::pushBranch(const QString &context)
{
    QMutexLocker locker(&d->messageMutex);
    GCF::LogMessage *msg = new GCF::LogMessage(GCF::LogMessage::Info, context,
                                               QByteArray(), QString(), QString(),
                                               d->currentBranch());
    d->stack.push(msg);
    return msg;
}

/**
 * \internal
 */
GCF::LogMessage *GCF::Log::popBranch()
{
    QMutexLocker locker(&d->messageMutex);
    if(d->stack.isEmpty())
        return nullptr;

    return d->stack.pop();
}

/**
 * \internal
 */
void GCF::Log::messageUpdated(GCF::LogMessage*)
{
    // FIXME: #pragma message("Upgrade GCF::Log::messageUpdated() to do a smart update of the model")
#if QT_VERSION >= 0x050000
    this->beginResetModel();
    this->endResetModel();
#else
    this->reset();
#endif
}

/**
 * \internal
 */
void GCF::Log::messageCreated(GCF::LogMessage*)
{
    // FIXME: #pragma message("Upgrade GCF::Log::messageCreated() to do a smart update of the model")
#if QT_VERSION >= 0x050000
    this->beginResetModel();
    this->endResetModel();
#else
    this->reset();
#endif
}

/**
 * \internal
 */
void GCF::Log::messageDestroyed(GCF::LogMessage *msg)
{
    int index = d->stack.indexOf(msg);
    if(index >= 0)
        d->stack.remove(index, 1);

    // FIXME: #pragma message("Upgrade GCF::Log::messageDestroyed() to do a smart update of the model")
#if QT_VERSION >= 0x050000
    this->beginResetModel();
    this->endResetModel();
#else
    this->reset();
#endif
}

/**
 * \internal
 */
void GCF::Log::handleLogMessage(GCF::LogMessage *msg)
{
    if(msg->parent() != d->rootMessage)
        return;

    QString fName = this->logFileName();
    QFile file(fName);
    if( file.open(QFile::Append) == false )
    {
        qDebug() << "Cannot write into log file " << fName;
        return;
    }

    QTextStream ts(&file);
    this->print(msg, ts);

    delete msg;
}

/**
 * \internal
 */
void GCF::Log::print(GCF::LogMessage *msg, QTextStream &ts)
{
    int indent = -2;
    GCF::LogMessage *tmp = msg;
    while(tmp)
    {
        ++indent;
        tmp = tmp->parent();
    }

    ts << QString(indent*2, QChar(' ')) << msg->context()
       << " : LogLevel(" << msg->logLevel() << ") "
       << msg->message() << msg->details() << "\n";

    for(int i=0; i<msg->children().count(); i++)
        d->handler->print(msg->children().at(i), ts);
}

/**
 * \internal
 */
QString GCF::Log::defaultLogContext(const QString &fnInfo, const QString &file, int line)
{
    QString fileName = QFileInfo(file).baseName();
    QString fnName = fnInfo.section('(', 0, 0).split("::").last();
    QString retString = QString("%1:%2-%3").arg(fileName).arg(line).arg(fnName);
    return retString;
}

///////////////////////////////////////////////////////////////////////////////

/**
\class GCF::LogMessage Log.h <GCF3/Log>
\brief Represents a single message in the log
\ingroup gcf_core


Instances of this class are created by \ref GCF::Log::info(), \ref GCF::Log::warning(),
\ref GCF::Log::debug(), \ref GCF::Log::error(), \ref GCF::Log::fatal() and stored
in the log message hierarchy, until the message is handled by the log message handler
(\ref GCF::LogMessageHandlerInterface). As soon as the message is handled; it gets
destroyed.

This class is internal. You will need to use it only if you are writing your own
handler.
*/

namespace GCF
{

struct LogMessageData
{
    LogMessageData() : logLevel(GCF::LogMessage::Info), parent(nullptr) { }

#ifdef Q_OS_MAC
    char unused[4]; // Padding added to align this struct
#endif
    int logLevel;
    QString context;
    QByteArray logCode;
    QString message;
    QString details;
    QList<LogMessage*> children;
    LogMessage *parent;
};

}

/**
 * \internal
 */
GCF::LogMessage::LogMessage(int level,
                            const QString &context,
                            const QByteArray &logCode,
                            const QString &msg,
                            const QString &details,
                            LogMessage *parent)
{
    d = new GCF::LogMessageData;
    d->logLevel = level;
    d->context = context;
    d->logCode = logCode;
    d->message = msg;
    d->details = details;
    d->parent = parent;
    if(d->parent)
        d->parent->d->children.append(this);

    if(GCF::Log::instance())
        GCF::Log::instance()->messageCreated(this);
    else
        qDebug() << Q_FUNC_INFO << " - NULL GCF::Log instance!!";
}

/**
 * \internal
 */
GCF::LogMessage::LogMessage()
{
    d = new GCF::LogMessageData;
}

/**
 * Destroys the log message and all its children
 */
GCF::LogMessage::~LogMessage()
{
    if(GCF::Log::instance())
        GCF::Log::instance()->messageDestroyed(this);
    else
        qDebug() << Q_FUNC_INFO << " - NULL GCF::Log instance!!";

    if(d->parent)
        d->parent->d->children.removeAll(this);
    d->parent = nullptr;

    this->clear();

    delete d;
}

/**
 * \return pointer to the parent log message
 */
GCF::LogMessage *GCF::LogMessage::parent() const
{
    return d->parent;
}

/**
 * \return list of pointers to children messages
 */
QList<GCF::LogMessage*> GCF::LogMessage::children() const
{
    return d->children;
}

/**
 * \enum GCF::LogMessage::LogLevel
 * Enumeration of log message levels
 */

/**
 * \var GCF::LogMessage::LogLevel GCF::LogMessage::Fatal
 */

/**
 * \var GCF::LogMessage::LogLevel GCF::LogMessage::Error
 */

/**
 * \var GCF::LogMessage::LogLevel GCF::LogMessage::Warning
 */

/**
 * \var GCF::LogMessage::LogLevel GCF::LogMessage::Debug
 */

/**
 * \var GCF::LogMessage::LogLevel GCF::LogMessage::Info
 */

/**
 * \var GCF::LogMessage::LogLevel GCF::LogMessage::User
 */

/**
 * \return log-level value. It can be any of the \ref GCF::LogMessage::LogLevel values.
 */
int GCF::LogMessage::logLevel() const
{
    return d->logLevel;
}

/**
 * \return context of the log message
 */
QString GCF::LogMessage::context() const
{
    return d->context;
}

/**
 * \return code of the log message
 */
QByteArray GCF::LogMessage::logCode() const
{
    return d->logCode;
}

/**
 * \return one-line text of the log message
 */
QString GCF::LogMessage::message() const
{
    return d->message;
}

/**
 * \return multi-line text of the log message
 */
QString GCF::LogMessage::details() const
{
    return d->details;
}

/**
 * \internal
 */
void GCF::LogMessage::clear()
{
    QList<GCF::LogMessage*> children(d->children);
    d->children.clear();
    qDeleteAll(children);
}

/**
 * \internal
 */
void GCF::LogMessage::setMessage(const QString &msg)
{
    d->message = msg;

    if(GCF::Log::instance())
        GCF::Log::instance()->messageUpdated(this);
    else
        qDebug() << Q_FUNC_INFO << " - NULL GCF::Log instance!!";

}

///////////////////////////////////////////////////////////////////////////////

/**
\class GCF::LogMessageBranch Log.h <GCF3/Log>
\brief Creates a branch in the log-message hierarchy
\ingroup gcf_core


This class is meant to be used on the stack for creating logical message branches
for a function-context. The best way to use this class, for creating message branches,
is to create an instance of this class on the 1st line of the function. After that
log messages can be added to this branch by calling \ref GCF::Log::info(),
\ref GCF::Log::warning(), \ref GCF::Log::debug(), \ref GCF::Log::error() or
\ref GCF::Log::fatal().

\code
void anotherFunction();
void yetAnotherFunction();

void function()
{
    GCF::LogMessageBranch branch("function() branch");
    ::anotherFunction();
    ::yetAnotherFunction();
}

void anotherFunction()
{
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Simple log message");
}

void yetAnotherFunction()
{
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Simple log message");
}
\endcode

If no message was logged after creation on the branch, then the branch's log message
will automatically be deleted when the branch object is deleted.

\note Never create an instance of this class on the heap.
*/

/**
 * Creates a log message branch. A new instance of \ref GCF::LogMessage
 * is created for representing this branch in the log-message hierarchy.
 * The branch-message will be automatically deleted by the destructor, if
 * no log messages were added to it.
 *
 * @param context a string representing the context of the branch
 */
GCF::LogMessageBranch::LogMessageBranch(const QString &context)
{
    if(GCF::Log::instance())
        m_branchMessage = GCF::Log::instance()->pushBranch(context);
    else
    {
        m_branchMessage = nullptr;
        qDebug() << Q_FUNC_INFO << " - NULL GCF::Log instance!!";
    }
}

/**
 * Destructor. The branch message (created in the constructor) will
 * automatically be destroyed, if no log-messages were added to it.
 */
GCF::LogMessageBranch::~LogMessageBranch()
{
    if(m_branchMessage)
    {
        GCF::LogMessage *branch = GCF::Log::instance()->popBranch();
        if(branch == m_branchMessage)
        {
            if(m_branchMessage->children().count() == 0)
                delete m_branchMessage;
            else if(m_branchMessage->parent()->parent() == nullptr)
                GCF::Log::instance()->dumpLogMessage(m_branchMessage);
        }

        m_branchMessage = nullptr;
    }
}


