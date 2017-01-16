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

#include "IpcCall.h"
#include "IpcCommon_p.h"
#include "../Core/Log.h"

#include <QTimer>
#include <QTcpSocket>
#include <QDataStream>
#include <QSemaphore>

Q_GLOBAL_STATIC(QObject, GlobalIpcCallParent)

/**
\class GCF::IpcCall IpcCall.h <GCF3/IpcCall>
\brief Make calls on methods in a remote object
\ingroup gcf_ipc


You can use this class to asynchronously invoke a service method on a remote object
and respond to its result when signalled. Each instance of this class can be used
to make only one service call. Example

\code
QHostAddress addr = ...;
quint16 port = ...;
GCF::IpcCall *call = new GCF::IpcCall(addr, port, "Application.MyService",
            "method1", QVariantList() << ... << ...);
connect(call, SIGNAL(done(bool)), ..., SLOT(callDone(bool)));

void abcd::callDone(bool val) {
    GCF::IpcCall *call = qobject_cast<GCF::IpcCall *>(this->sender());
    if(val) {
        QVariant result = call->result();
    } else {
        QString errMsg = call->errorMessage();
        // Display errMsg
    }
}
\endcode
 */

namespace GCF
{

struct IpcCallData
{
    IpcCallData() : port(0),
        done(false), success(false),
        messageId(-1), autoDelete(false),
        socket(0), timeoutTimer(0) { }

    QHostAddress address;
    quint16 port;
    QString object;
    QString method;
    QVariantList arguments;

    bool done;

    bool success;
    QString errorMessage;
    QVariant result;
    int messageId;
    bool autoDelete;

    QTcpSocket *socket;
    QTimer *timeoutTimer;

    static QSemaphore CallSemaphore;
};

}

QSemaphore GCF::IpcCallData::CallSemaphore(20);

/**
 * Constructor
 *
 * @param addr address of the computer where the remote application is running
 * @param port port number on which the remote application's \ref GCF::IpcServer is listening
 * @param object name of the object that contains the \c method
 * @param method service method that needs to be invoked
 * @param args list of arguments that the method accepts
 * @param parent a \c QObject that would become the parent of this class
 *
 * By the time the constructor returns, the call will have been scheduled.
 */
GCF::IpcCall::IpcCall(const QHostAddress &addr, quint16 port,
                      const QString &object, const QString &method,
                      const QVariantList &args, QObject *parent)
    : QObject(0)
{
    if(parent && parent->thread() == this->thread())
        this->setParent(parent);
    else if(!parent && this->thread() == ::GlobalIpcCallParent()->thread())
        this->setParent(::GlobalIpcCallParent());

    d = new GCF::IpcCallData;
    d->address = addr;
    d->port = port;
    d->object = object;
    d->method = method;
    d->arguments = args;
    QMetaObject::invokeMethod(this, "onCall", Qt::QueuedConnection);
}

/**
 * Destructor
 */
GCF::IpcCall::~IpcCall()
{
    delete d;
}

/**
 * @return address of the computer on which the remote application is running
 */
QHostAddress GCF::IpcCall::address() const
{
    return d->address;
}

/**
 * @return port number of the remote application's \ref GCF::IpcServer
 */
quint16 GCF::IpcCall::port() const
{
    return d->port;
}

/**
 * @return name of the object whose method this class is calling
 */
QString GCF::IpcCall::object() const
{
    return d->object;
}

/**
 * @return name of the service method that is being called
 */
QString GCF::IpcCall::method() const
{
    return d->method;
}

/**
 * @return list of arguments passed to the service method
 */
QVariantList GCF::IpcCall::arguments() const
{
    return d->arguments;
}

/**
 * @return true if the call has been sent and the response has been received. False otherwise
 */
bool GCF::IpcCall::isDone() const
{
    return d->done;
}

/**
 * @return true if the call was successfully made and response was received with no
 * error. False otherwise.
 */
bool GCF::IpcCall::isSuccess() const
{
    return d->success;
}

/**
 * @return if \ref isSuccess() returns false, then this function returns an error
 * message describing what went wrong.
 */
QString GCF::IpcCall::errorMessage() const
{
    return d->errorMessage;
}

/**
 * @return value returned by the remote method. If the remote method returned a
 * \ref GCF::Result, then the value returned by this function would be the data
 * ( \ref GCF::Result::data() ) component of the \ref GCF::Result.
 */
QVariant GCF::IpcCall::result() const
{
    return d->result;
}

/**
 * Sets a timeout interval, afterwhich the call is considered to have failed.
 * If no timeout is set, then a default value of 10 seconds is used.
 *
 * @param val timeout in milliseconds
 */
void GCF::IpcCall::setTimeoutDuration(int val)
{
    this->setProperty("timeoutDuration", val);
}

/**
 * Configures the call enable/disable self-deletion after emitting \ref done().
 *
 * @param val if true, the call will delete itself after emiting \ref done()
 *
 * \note if the \ref isDone() returns true, then the function is a no-op.
 */
void GCF::IpcCall::setAutoDelete(bool val)
{
    if(d->done)
        return; // Whats the point of setting auto-delete?

    d->autoDelete = val;
}

/**
 * @return true if the call would delete itself after emitting \ref done()
 */
bool GCF::IpcCall::isAutoDelete() const
{
    return d->autoDelete;
}

/**
 * Blocks until the \ref done() signal is emitted or timeout
 * @return true if the call was successful, false otherwise.
 * \note The call will not freeze any UI while waiting
 */
bool GCF::IpcCall::waitForDone()
{
    QEventLoop eventLoop;
    while(!d->done)
        eventLoop.processEvents();
    return d->success;
}

/**
\fn void GCF::IpcCall::done(bool success)

This signal is emitted after the call is done. The \c success parameter is true
if the call was successfuly invoked and encountered no error.
 */

void GCF::IpcCall::emitDone(bool success, const QString &msg)
{
    if(d->done)
        return;

    d->CallSemaphore.release();

    d->done = true;
    d->success = success;
    d->errorMessage = msg;

    if(d->success)
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, "Success!");
    else
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                   QString("Call failed: %1").arg(d->errorMessage));

    emit done(d->success);

    if(d->autoDelete)
        this->deleteLater();
}

int GCF::IpcCall::timeoutDuration() const
{
    if(this->dynamicPropertyNames().contains("timeoutDuration"))
    {
        int val = this->property("timeoutDuration").toInt();
        val = qMax(100, val);
        val = qMin(60000, val);
        return val;
    }

    return 10000;
}

void GCF::IpcCall::onCall()
{
    if(d->socket) // The call is already underway.
        return;

    /*
     We are going to limit the number of active IpcCalls at a given point of time.
     This is dont to ensure that we dont overload the system and go beyond the
     pipe/socket limit.
     */
    if(!d->CallSemaphore.tryAcquire())
    {
        QTimer::singleShot(100, this, SLOT(onCall()));
        return;
    }

    GCF::LogMessageBranch branch( QString("Calling %1::%2 with %3 args on %4:%5")
                                  .arg(d->object).arg(d->method)
                                  .arg(d->arguments.count())
                                  .arg(d->address.toString())
                                  .arg(d->port));

    if(d->address.isNull())
    {
        emitDone(false, tr("Invalid host address"));
        return;
    }

    if(d->port == 0)
    {
        emitDone(false, tr("Invalid port number"));
        return;
    }

    if(d->method.isEmpty())
    {
        emitDone(false, tr("Method unspecified"));
        return;
    }

    d->socket = new QTcpSocket(this);
    connect(d->socket, SIGNAL(connected()), this, SLOT(onConnected()));
    d->socket->connectToHost(d->address, d->port);

    d->timeoutTimer = new QTimer(this);
    connect(d->timeoutTimer, SIGNAL(timeout()), this, SLOT(onConnectTimeout()));
    d->timeoutTimer->setInterval(this->timeoutDuration());
    d->timeoutTimer->start();
}

void GCF::IpcCall::onConnected()
{
    delete d->timeoutTimer;
    d->timeoutTimer = 0;

    disconnect(d->socket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(d->socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(d->socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(d->socket, SIGNAL(bytesWritten(qint64)), this, SLOT(onBytesWritten()));

    GCF::IpcMessage message(GCF::IpcMessage::IPC_CALL);
    message.data()["object"] = d->object;
    message.data()["method"] = d->method;
    message.data()["arguments"] = d->arguments;

    QByteArray packet;
    QDataStream ds(&packet, QIODevice::WriteOnly);
    ds << message.toByteArray();

    QString msg = QString("Sending message-id %1 of type %2 int %3 bytes to %4:%5")
            .arg(message.id())
            .arg(QString::fromLatin1(message.type()))
            .arg(packet.size()-sizeof(qint32))
            .arg(d->socket->peerAddress().toString())
            .arg(d->socket->peerPort());
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, msg);
    d->messageId = message.id();

    d->socket->write(packet);

    d->timeoutTimer = new QTimer(this);
    connect(d->timeoutTimer, SIGNAL(timeout()), this, SLOT(onCallTimeout()));
    d->timeoutTimer->setInterval(this->timeoutDuration());
    d->timeoutTimer->start();
}

void GCF::IpcCall::onReadyRead()
{
    if(d->timeoutTimer)
    {
        d->timeoutTimer->stop();
        d->timeoutTimer->start();
    }
}

void GCF::IpcCall::onBytesWritten()
{
    if(d->timeoutTimer)
    {
        d->timeoutTimer->stop();
        d->timeoutTimer->start();
    }
}

void GCF::IpcCall::onDisconnected()
{
    if(d->socket->bytesAvailable())
    {
        QDataStream ds(d->socket);
        quint32 size = 0;
        ds >> size;

        GCF::IpcMessage message;
        if(d->socket->bytesAvailable() >= qint64(size))
            message = GCF::IpcMessage::fromByteArray(d->socket->read(qint64(size)));

        QString msg = QString("Processing incoming message-id %1 of type %2 from %3 bytes from %4:%5")
                .arg(message.id())
                .arg(QString::fromLatin1(message.type()))
                .arg(size)
                .arg(d->socket->peerAddress().toString())
                .arg(d->socket->peerPort());
        GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, msg);

        if(message.type() == GCF::IpcMessage::IPC_CALL && message.isResponse())
        {
            d->success = message.result().isSuccess();
            d->errorMessage = message.result().message();
            d->result = message.result().data();
            d->done = true;
            d->CallSemaphore.release();
            emit done(d->success);
            if(d->autoDelete)
                this->deleteLater();
        }
        else
        {
            // This never happens. We cannot write a test case to
            // validate the emission of this error message.
            emitDone(false, tr("Invalid response sent by the server"));
        }
    }
    else
        emitDone(false, tr("The connection was cut before any response could be received"));
}

void GCF::IpcCall::onConnectTimeout()
{
    d->timeoutTimer->deleteLater();
    d->timeoutTimer = 0;
    disconnect(d->socket, SIGNAL(connected()), this, SLOT(onConnected()));
    d->socket->abort();
    emitDone(false, tr("A connection timeout occured"));
}

void GCF::IpcCall::onCallTimeout()
{
    d->timeoutTimer->deleteLater();
    d->timeoutTimer = 0;
    disconnect(d->socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    d->socket->abort();
    emitDone(false, tr("A call timeout occured"));
}

int GCF::IpcCall::messageId() const
{
    return d->messageId;
}
