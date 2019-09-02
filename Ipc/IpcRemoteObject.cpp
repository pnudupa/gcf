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

#include "IpcRemoteObject.h"
#include "IpcCommon_p.h"
#include "IpcCall.h"

#include "../Core/Log.h"
#include "../Core/SignalSpy.h"
#include "../Core/ObjectMap.h"
#include "../Core/Application.h"

#include <QMetaMethod>
#include <QDataStream>

namespace GCF
{

struct NotificationPair;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-vtables"

class SignalDespatch : public GCF::SignalSpy
{
public:
    SignalDespatch(QObject *sender,
                   const char *signal,
                   const QString &method,
                   IpcRemoteObject *parent)
        : GCF::SignalSpy(sender, signal, parent),
          m_address(parent->remoteAddress()),
          m_port(parent->remotePort()),
          m_object(parent->remoteObjectPath()),
          m_method(method) {
        connect(sender, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    }

    SignalDespatch(QObject *sender, const char *signal,
                   const QHostAddress &address, quint16 port,
                   const QString &object, const QString &method,
                   QObject *parent=nullptr)
        : GCF::SignalSpy(sender, signal, parent),
          m_address(address), m_port(port), m_object(object),
          m_method(method) { }

    ~SignalDespatch() { }

    void handleSignalEmission(const QVariantList &args) {
        new GCF::IpcCall(m_address, m_port, m_object, m_method, args, this);
    }

private:
    QHostAddress m_address;
    quint16 m_port;
    char unused[6] __attribute__((unused)); // For padding
    QString m_object;
    QString m_method;
};

class RemoteObjectCall : public GCF::IpcCall
{
public:
    RemoteObjectCall(const QHostAddress &addr, quint16 port,
            const QString &object,
            const QString &method,
            const QVariantList &arguments,
            QObject *receiver,
            const char *member,
            QObject *parent=nullptr)
        : GCF::IpcCall(addr, port, object, method, arguments, parent),
          m_receiver(receiver) {
        if(member) {
            m_member = QString::fromLatin1(member+1);
            m_member = m_member.section('(', 0, 0);
        }
    }

    ~RemoteObjectCall() { }

protected:
    void done(bool success) {
        // This is a signal in reality. But we use it here for despatching
        // success to the receiver.
        if(!m_receiver.isNull() && !m_member.isEmpty()) {
            QVariantList args;
            args.append(this->result());
            args.append(success);
            args.append(this->errorMessage());
            gAppService->invokeMethod(m_receiver.data(), m_member, args, false);
            QMetaObject::invokeMethod(this->parent(), "requestFinished",
                                      Qt::DirectConnection, Q_ARG(int,this->messageId()));
        }

        // Base class signal implementation
        GCF::IpcCall::done(success);
    }

private:
    QPointer<QObject> m_receiver;
    QString m_member;
};

#pragma clang diagnostic pop

struct IpcRemoteObjectData
{
    IpcRemoteObjectData() : socket(nullptr), activated(NOT_ACTIVATED) { }
    ~IpcRemoteObjectData() {
        QList<NotificationPair*> npl = notificationPairs;
        notificationPairs.clear();
        qDeleteAll(npl);
    }

    QString object;
    QHostAddress address;
    quint16 port;

    char unused1[6]; // For padding
    GCF::IpcMessage currentMessage;
    QList<GCF::IpcMessage> messageQueue;

    GCF::IpcSocket *socket;
    enum {
        COULD_NOT_ACTIVATE = -1,
        NOT_ACTIVATED = 0,
        ACTIVATED = 1
    } activated;

    // Meta-Information about the remote object
    char unused2[4]; // For padding
    QVariantMap properties;
    QStringList signalMethods;
    QStringList invokableMethods;

    // Notification structures
    QList<NotificationPair*> notificationPairs;
    QMap< qint32, NotificationPair* > propertyNotifications;
    QMap< QString, QList<NotificationPair*> > signalDeliveryNotifications;
};

struct NotificationPair
{
    explicit NotificationPair(QObject *obj, const QString &member, IpcRemoteObjectData *parent)
        : m_parent(parent) {
        m_parent->notificationPairs.append(this);
        this->Object = obj;
        this->Member = member.section('(', 0, 0);
    }
    ~NotificationPair() { m_parent->notificationPairs.removeAll(this); }

    void notify(const QVariantList &args) {
        GCF::Result nr = gAppService->invokeMethod(this->Object.data(), this->Member, args, false);
        if(!nr.isSuccess()) {
            GCF::Log::instance()->warning(GCF_DEFAULT_LOG_CONTEXT,
                                          QString("Error while notifying %1::%2")
                                          .arg(this->Object->metaObject()->className())
                                          .arg(this->Member));
        }
    }
    void notify(const GCF::Result &result) {
        QVariantList args;
        args << result.data() << result.isSuccess() << result.message();
        this->notify(args);
    }

    QPointer<QObject> Object;
    QString Member;

private:
    IpcRemoteObjectData *m_parent;
};

}

/**
\class GCF::IpcRemoteObject IpcRemoteObject.h <GCF3/IpcRemoteObject>
\brief This class allows you to maintain a persistent connection with a remote object
\ingroup gcf_ipc


Once an instance of this class is created to connect to a remote object, it sends
an activation request. Upon \ref activation(); the object maintains a persistent
connection with the remote object. If the remote application shuts down, or the
remote object was deleted or otherwise goes out of communication, the \ref deactivated()
signal is emitted.

The following code snippet shows you how to create an instance of \ref GCF::IpcRemoteObject
and prepare to use it.

\code
GCF::IpcRemoteObject *remoteObj = new GCF::IpcRemoteObject(addr, port, "Application.MyService");

// Wait for the object to get activated
GCF::SignalSpy spy(remoteObj, SIGNAL(activated()));
spy.wait();

if( remoteObj->isActivated() ) {
    // Connection to the remote object is now activated
}
\endcode

You can make use of the \ref GCF::ipcConnect() methods to make signal/slot connections
between a local object and remote object. Example:

\code
GCF::IpcRemoteObject *remoteApp = new GCF::IpcRemoteObject(addr, port, "Application");

// Wait for the object to get activated
GCF::SignalSpy spy(remoteApp, SIGNAL(activated()));
spy.wait();

// When a local button is clicked, we want the remoteApp to quit.
QPushButton *quitButton = ...;
GCF::ipcConnect(quitButton, SIGNAL(clicked()), remoteAll, SLOT(quit()));
\endcode
 */

/**
 * Constructor.
 *
 * @param addr address of the computer on which the remote application is running
 * @param port port number of the \ref GCF::IpcServer in the remote application
 * @param object name of the remote object
 * @param parent pointer to the parent \c QObject of this class
 *
 * Sometime after the constructor returns the \ref activated() signal is emitted upon
 * successful activation. The \ref coundNotActivate() signal is emitted with an error
 * message describing the reason for failure.
 */
GCF::IpcRemoteObject::IpcRemoteObject(const QHostAddress &addr, quint16 port,
                                      const QString &object, QObject *parent)
    : QObject(parent)
{
    d = new IpcRemoteObjectData;
    d->object = object;
    d->address = addr;
    d->port = port;

    QMetaObject::invokeMethod(this, "onSetup", Qt::QueuedConnection);
}

/**
 * Destructor
 */
GCF::IpcRemoteObject::~IpcRemoteObject()
{
    delete d;
}

/**
 * @return true if this class has been activated. False otherwise.
 */
bool GCF::IpcRemoteObject::isActivated() const
{
    return d->activated == GCF::IpcRemoteObjectData::ACTIVATED;
}

/**
 * If this class was activated once and then got deactivated for whatever reason, you
 * can call this method to request a reactivation.
 */
void GCF::IpcRemoteObject::reactivate()
{
    if(this->isActivated() || d->socket)
        return;

    d->activated = GCF::IpcRemoteObjectData::NOT_ACTIVATED;
    this->onSetup();
}

/**
 * @return address of the computer on which the remote application is running
 */
QHostAddress GCF::IpcRemoteObject::remoteAddress() const
{
    return d->address;
}

/**
 * @return port number of the \ref GCF::IpcServer on the remote application
 */
quint16 GCF::IpcRemoteObject::remotePort() const
{
    return d->port;
}

/**
 * @return path of the object in the remote application's object tree
 */
QString GCF::IpcRemoteObject::remoteObjectPath() const
{
    return d->object;
}

/**
 * @return A map of all properties and their values in the remote application.
 *
 * \note this function returns an empty map until \ref isActivated() returns false.
 */
QVariantMap GCF::IpcRemoteObject::properties() const
{
    return d->properties;
}

/**
 * @return a list of signal method signatures in the remote object
 *
 * \note this function returns an empty list until \ref isActivated() returns false.
 */
QStringList GCF::IpcRemoteObject::signalMethods() const
{
    return d->signalMethods;
}

/**
 * @return a list of invokable (signal, public slots and Q_INVOKABLE) method signatures in the remote object
 *
 * \note this function returns an empty list until \ref isActivated() returns false.
 */
QStringList GCF::IpcRemoteObject::invokableMethods() const
{
    return d->invokableMethods;
}

/**
Sends a request to update the value of \c propertyName in the local \ref properties() map. The
function returns true if the update request was queued successfully, false with error message
otherwise.

The \c receiver and \c member parameters can be used to specify a object and its method that
should get invoked when the result of sending the update property request becomes available.
The \c member should be a function that accepts \c QVariant, \c bool and \c QString parameters.

@param propertyName name of the property to update
@param receiver pointer to the \c QObject that will get notified when the result of update property
request becomes available
@param member a member function in \c receiver that will get called when the result becomes available.
This parameter must be constructed using SIGNAL or SLOT macros.
@return true upon success. false and error message upon failure.

\sa propertyUpdated()
 */
GCF::Result GCF::IpcRemoteObject::updateProperty(const QString &propertyName,
                    QObject *receiver, const char *member)
{
    if(d->properties.contains(propertyName))
    {
        GCF::IpcMessage message(GCF::IpcMessage::GET_PROPERTY_VALUE);
        message.data()["propertyName"] = propertyName;

        if(receiver && member)
        {
            if(QMetaObject::checkConnectArgs(member, "function(QVariant,bool,QString)"))
            {
                NotificationPair *npair = new NotificationPair(receiver, QString::fromLatin1(member+1), d);
                d->propertyNotifications[message.id()] = npair;
            }
            else
                return GCF::Result(false, QString(),
                                   QString("Receiver's member function's parameter list should be "
                                           "(QVariant,bool,QString)."));
        }

        this->queueMessage(message);

        return GCF::Result(true, QString(), QString(), message.id());
    }

    return GCF::Result(false, QString(),
                       QString("Property '%1' doesnt exist in this object").arg(propertyName));
}

/**
Sends a request to change the value of \c propertyName to \c propertyValue. The function returns
true if the update request was queued successfully, false with error message otherwise.

The \c receiver and \c member parameters can be used to specify a object and its method that
should get invoked when the result of sending the change property request becomes available.
The \c member should be a function that accepts \c QVariant, \c bool and \c QString parameters.

@param propertyName name of the property in the remote object to change
@param propertyValue new value of the property to be set on the remote obkect
@param receiver pointer to the \c QObject that will get notified when the result of change property
request becomes available
@param member a member function in \c receiver that will get called when the result becomes available.
This parameter must be constructed using SIGNAL or SLOT macros.
@return true upon success. false and error message upon failure.

\sa propertyUpdated()
 */
GCF::Result GCF::IpcRemoteObject::changeProperty(const QString &propertyName, const QVariant &propertyValue,
                    QObject *receiver, const char *member)
{
    if(d->properties.contains(propertyName))
    {
        GCF::IpcMessage message(GCF::IpcMessage::SET_PROPERTY_VALUE);
        message.data()["propertyName"] = propertyName;
        message.data()["propertyValue"] = propertyValue;

        if(receiver && member)
        {
            if(QMetaObject::checkConnectArgs(member, "function(QVariant,bool,QString)"))
            {
                NotificationPair *npair = new NotificationPair(receiver, QString::fromLatin1(member+1), d);
                d->propertyNotifications[message.id()] = npair;
            }
            else
                return GCF::Result(false, QString(),
                                   QString("Receiver's member function's parameter list should be "
                                           "(QVariant,bool,QString)."));
        }

        this->queueMessage(message);

        return GCF::Result(true, QString(), QString(), message.id());
    }

    return GCF::Result(false, QString(),
                       QString("Property '%1' doesnt exist in this object").arg(propertyName));
}

/**
\internal
 */
GCF::Result GCF::IpcRemoteObject::ipcConnect(const char *signal, QObject *receiver, const char *member)
{
    if(signal && receiver && member)
    {
        QString signalFunction = QString::fromLatin1( QMetaObject::normalizedSignature(signal+1) );
        if(!d->signalMethods.contains(signalFunction))
            return GCF::Result(false, QString(), "Signal doesnt exist in the remote object");

        QByteArray memberFunction = QMetaObject::normalizedSignature(member+1);
        if(receiver->metaObject()->indexOfMethod(memberFunction) < 0)
            return GCF::Result(false, QString(), "Member doesnt exist in the receiver");

        // GCF 3.1: Remove this restriction in a future version of GCF
        QString memberFunctionStr = QString::fromLatin1(memberFunction);
        if(memberFunctionStr.section('(', 1, 1) != signalFunction.section('(', 1, 1))
            return GCF::Result(false, QString(), "Parameters accepted by the member must be same as those emitted by the signal");

        GCF::IpcMessage message(GCF::IpcMessage::REQUEST_CONNECTION);
        message.data()["signal"] = signalFunction;
        this->queueMessage(message);

        NotificationPair *npair = new NotificationPair(receiver, QString::fromLatin1(memberFunction), d);
        d->signalDeliveryNotifications[signalFunction].append(npair);

        return GCF::Result(true, QString(), QString(), message.id());
    }

    return GCF::Result(false, QString(), "signal, receiver and member must be provided as parameters to this function");
}

/**
\internal
 */
GCF::Result GCF::IpcRemoteObject::ipcConnect(QObject *sender, const char *signal, const char *member)
{
    if(sender && signal && member)
    {
        QString memberFunction = QString::fromLatin1( QMetaObject::normalizedSignature(member+1) );
        if(!d->invokableMethods.contains(memberFunction))
            return GCF::Result(false, QString(), "Member doesnt exist in remote object");

        // GCF 3.1: Remove this restriction in a future version of GCF
        QString signalFunction = QString::fromLatin1( QMetaObject::normalizedSignature(signal+1) );
        if(memberFunction.section('(', 1, 1) != signalFunction.section('(', 1, 1))
            return GCF::Result(false, QString(), "Parameters accepted by the member must be same as those emitted by the signal");

        memberFunction = memberFunction.section('(', 0, 0);

        SignalDespatch *signalDespatch = new SignalDespatch(sender, signal, memberFunction, this);
        if(!signalDespatch->isValid())
        {
            delete signalDespatch;
            return GCF::Result(false, QString(), "Signal doesnt exist in the sender");
        }

        return true;
    }

    return GCF::Result(false, QString(), "sender, signal and member must be provided as parameters to this function");
}

/**
 * Places a asynchronous call to a \c method on the remote object with \c args. When the result of the
 * method invocation becomes available notification is sent to the \c member function on \c receiver.
 *
 * @param method name of the remote method to invoke
 * @param args arguments to be passed to the remote method
 * @param receiver pointer to the receiver \c QObject that will be notified when the result of method
 * invocation becomes available
 * @param member a member function in \c receiver that will get called when the result becomes available.
 * This parameter must be constructed using SIGNAL or SLOT macros. This member function must accept a
 * \c QVariant, \c bool and \c QString parameter.
 * @return true upon successfully placing the call. False + error message upon failure.
 */
GCF::Result GCF::IpcRemoteObject::call(const QString &method, const QVariantList &args, QObject *receiver, const char *member)
{
    bool methodExists = false;
    for(int i=0; i<d->invokableMethods.count(); i++)
    {
        QString invokableMethod = d->invokableMethods.at(i).section('(', 0, 0);
        if(invokableMethod == method)
        {
            methodExists = true;
            break;
        }
    }

    if(!methodExists)
        return GCF::Result(false, QString(), "Method doesnt exist in the remote object");

    if(receiver && member)
    {
        QByteArray memberMethod = QMetaObject::normalizedSignature(member+1);
        if( receiver->metaObject()->indexOfMethod(memberMethod) < 0 )
            return GCF::Result(false, QString(), "Notification member doesnt exist in the receiver object");

        if( !QMetaObject::checkConnectArgs(memberMethod, "function(QVariant,bool,QString)") )
            return GCF::Result(false, QString(),
                               QString("Receiver's member function's parameter list should be "
                                       "(QVariant,bool,QString)"));
    }

    new GCF::RemoteObjectCall(d->address, d->port, d->object, method, args, receiver, member, this);
    return true;
}

/**
\fn void GCF::IpcRemoteObject::activated()

This signal is emitted when this class becomes activated. \ref GCF::IpcRemoteObject is considered
activated when it was able to establish communication with a remote object successfuly and receive
\ref properties(), \ref signalMethods() and \ref invokableMethods() data.
*/

/**
\fn void GCF::IpcRemoteObject::deactivated()

This signal is emitted when connection with a remote object is lost. After this signal is emitted
\ref properties(), \ref signalMethods() and \ref invokableMethods() return empty maps and lists.
*/

/**
\fn void GCF::IpcRemoteObject::requestFinished(int requestId)

\internal
*/

/**
\fn void GCF::IpcRemoteObject::signalOccurance(const QString &signal, const QVariantList &args)

This signal is emitted when a signal by name \c signal was emitted in the remote object. \c args contains
the parameters emitted in the remote signal.
*/

/**
\fn void GCF::IpcRemoteObject::propertyUpdated(const QString &propertyName, const QVariant &value)

This signal is emitted when the value of \c propertyName was updated to \c value in \ref properties().
This happens only upon explicit calls to \ref changeProperty() or \ref updateProperty()
*/

/**
\fn void GCF::IpcRemoteObject::couldNotActivate(const QString &error)

This signal is emitted when this class could not be activated. The \c error parameter describes the
reason for failure.
*/

/**
\fn void GCF::IpcRemoteObject::error(const QString &error)

This signal is emitted whenever any error is encountered in communicating with the remote object.
*/


void GCF::IpcRemoteObject::onSetup()
{
    if(d->socket)
        return;

    if(d->object.isEmpty())
    {
        d->activated = GCF::IpcRemoteObjectData::COULD_NOT_ACTIVATE;
        emitCouldNotActivate( tr("Cannot connect to remote object with unknown/empty name") );
        return;
    }

    d->socket = new GCF::IpcSocket(this);
    QObject::connect(d->socket, SIGNAL(connected()), this, SLOT(onSocketConnected()));
    QObject::connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(onSocketConnectError()));
    d->socket->connectToHost(d->address, d->port);
}

void GCF::IpcRemoteObject::onSocketConnected()
{
    QObject::disconnect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(onSocketConnectError()));
    QObject::connect(d->socket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    QObject::connect(d->socket, SIGNAL(incomingMessage(GCF::IpcMessage)), this,
            SLOT(onSocketIncomingMessage(GCF::IpcMessage)));

    // Send handshake message
    GCF::IpcMessage message(GCF::IpcMessage::REQUEST_OBJECT);
    message.data()["object"] = d->object;
    d->socket->sendMessage(message); // This is the only message that is not
                                     // sent using this->sendMessage()
}

void GCF::IpcRemoteObject::onSocketDisconnected()
{
    GCF::Result failure(false, QString(), "Connection with remote object was lost before any response could be received");

    QMap< qint32, NotificationPair* >::const_iterator pn_it = d->propertyNotifications.begin();
    QMap< qint32, NotificationPair* >::const_iterator pn_end = d->propertyNotifications.end();
    while( pn_it != pn_end )
    {
        NotificationPair *npair = pn_it.value();
        npair->notify(failure);
        delete npair;
        ++pn_it;
    }

    d->propertyNotifications.clear();
    d->signalDeliveryNotifications.clear();
    qDeleteAll(d->notificationPairs);
    d->notificationPairs.clear();
    d->properties.clear();
    d->invokableMethods.clear();
    d->signalMethods.clear();
    d->socket->deleteLater();
    d->socket = nullptr;
    d->currentMessage = GCF::IpcMessage();
    d->messageQueue.clear();

    d->activated = GCF::IpcRemoteObjectData::NOT_ACTIVATED;

    emitDeactivated();
}

void GCF::IpcRemoteObject::onSocketConnectError()
{
    QString errMsg = d->socket->errorString();
    emitCouldNotActivate(errMsg);
}

void GCF::IpcRemoteObject::onSocketIncomingMessage(const GCF::IpcMessage &message)
{
    if(message.isResponse() && message.type() == GCF::IpcMessage::REQUEST_OBJECT)
    {
        if(d->activated == GCF::IpcRemoteObjectData::ACTIVATED)
            emitError( tr("Out of sequence REQUEST_OBJECT message received.") );
        else if(message.result().isSuccess())
        {
            d->activated = GCF::IpcRemoteObjectData::ACTIVATED;
            d->properties = message.data().value("properties").toMap();
            d->signalMethods = message.data().value("signals").toStringList();
            d->invokableMethods = message.data().value("members").toStringList();
            emitActivated();
        }
        else
        {
            d->activated = GCF::IpcRemoteObjectData::COULD_NOT_ACTIVATE;
            emitCouldNotActivate(message.result().message());
        }

        this->sendNextMessage();
        return;
    }

    if( message.type() == GCF::IpcMessage::SIGNAL_DELIVERY )
    {
        QString signalMethod = message.data().value("signal").toString();
        QVariantList args = message.data().value("arguments").toList();
        QList<NotificationPair*> npairs = d->signalDeliveryNotifications.value(signalMethod);
        Q_FOREACH(NotificationPair *npair, npairs)
            npair->notify(args);

        emitSignalOccurance(signalMethod, args);
        this->sendNextMessage();
        return;
    }

    if(message.id() != d->currentMessage.id() || message.type() != d->currentMessage.type() )
    {
        emitError("Out of sequence message received");
        return;
    }

    // Process the response message to d->currentMessage
    if( message.type() == GCF::IpcMessage::GET_PROPERTY_VALUE ||
        message.type() == GCF::IpcMessage::SET_PROPERTY_VALUE )
    {
        QString propertyName = message.data().value("propertyName").toString();
        d->properties[propertyName] = message.result().data();

        NotificationPair *npair = d->propertyNotifications.value(message.id(), nullptr);
        if(npair)
        {
            d->propertyNotifications.remove(message.id());
            npair->notify(message.result());
            delete npair;
        }

        emitPropertyUpdated(propertyName, message.result().data());
        emitRequestFinished(message.id());
    }
    else if( message.type() == GCF::IpcMessage::REQUEST_CONNECTION )
    {
        // We basically get an ACK message whether the signal was connected
        // or not. We dont expect the signal-connection to fail ever; because
        // we will have done all the checks before sending connection request.
        emitRequestFinished(message.id());
    }
    else
        emitError( tr("Unknown message type '%1' received").arg(QString::fromLatin1(message.type())) );

    // Send the next message (if any)
    d->currentMessage = GCF::IpcMessage();
    this->sendNextMessage();
}

void GCF::IpcRemoteObject::queueMessage(const GCF::IpcMessage &message)
{
    if(d->activated == GCF::IpcRemoteObjectData::COULD_NOT_ACTIVATE)
        return; // ignore the message

    d->messageQueue.append(message);
    this->sendNextMessage();
}

void GCF::IpcRemoteObject::sendNextMessage()
{
    if(!d->socket ||
       d->activated != GCF::IpcRemoteObjectData::ACTIVATED ||
       d->socket->state() != GCF::IpcSocket::ConnectedState)
        return;

    if(d->currentMessage.isValid())
    {
        // We need to wait for the response on the current message first
        // before a next message can be sent. So, lets wait for it.
        return;
    }

    if(d->messageQueue.isEmpty())
        return;

    d->currentMessage = d->messageQueue.takeFirst();
    d->socket->sendMessage(d->currentMessage);
}

void GCF::IpcRemoteObject::emitActivated()
{
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                QString("IpcRemoteObject '%1' activated").arg(d->object) );
    emit activated();
}

void GCF::IpcRemoteObject::emitDeactivated()
{
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                                QString("IpcRemoteObject '%1' deactivated").arg(d->object) );
    emit deactivated();
}

void GCF::IpcRemoteObject::emitRequestFinished(int requestId)
{
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                               QString("IpcRemoteObject(%1) request '%2' finished").arg(d->object).arg(requestId) );
    emit requestFinished(requestId);
}

void GCF::IpcRemoteObject::emitSignalOccurance(const QString &signal, const QVariantList &args)
{
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                               QString("IpcRemoteObject '%1::%2' signal occurance").arg(d->object).arg(signal) );
    emit signalOccurance(signal, args);
}

void GCF::IpcRemoteObject::emitPropertyUpdated(const QString &name, const QVariant &value)
{
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT,
                               QString("IpcRemoteObject property %1::%2 updated to %3")
                               .arg(d->object).arg(name).arg(value.toString()) );
    emit propertyUpdated(name, value);
}

void GCF::IpcRemoteObject::emitCouldNotActivate(const QString &error)
{
    GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, error);
    emit couldNotActivate(error);
}

void GCF::IpcRemoteObject::emitError(const QString &errMsg)
{
    GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, errMsg);
    emit error(errMsg);
}

GCF::Result GCF::ipcConnect(QObject *sender, const char *signal, const QHostAddress &addr,
                quint16 port, const QString &object, const QString &method)
{
    if(!sender)
        return GCF::Result(false, QString(), QString("Sender not specified"));

    if(sender && signal)
    {
        int signalMethodIdx = sender->metaObject()->indexOfMethod(signal+1);
        if(signalMethodIdx < 0)
            return GCF::Result(false, QString(), QString("Signal doesnt exist in the sender"));

        QMetaMethod signalMethod = sender->metaObject()->method(signalMethodIdx);
        GCF::Result result = gAppService->isMethodInvokable(signalMethod);
        if(result.isSuccess() == false)
            return result;
    }

    GCF::SignalDespatch *signalDespatch
            = new GCF::SignalDespatch(sender, signal, addr, port, object, method);
    if(!signalDespatch->isValid())
    {
        delete signalDespatch;
        signalDespatch = nullptr;
        return GCF::Result(false, QString(), QString("Signal doesnt exist in the sender"));
    }

    return true;
}

/**
\fn GCF::Result GCF::ipcConnect(QObject *sender, const char *signal, GCF::IpcRemoteObject *receiver, const char *member)
\ingroup gcf_ipc

Connects \c signal of local \c QObject \c sender to \c member of remote object \c receiver.

This function returns true upon success. False+error message upon failure.
*/

/**
\fn GCF::Result GCF::ipcConnect(GCF::IpcRemoteObject *sender, const char *signal, QObject *receiver, const char *member)
\ingroup gcf_ipc

Connects \c signal of remote object \c sender to \c member of local \c QObject \c receiver.

This function returns true upon success. False+error message upon failure.
*/

/**
\fn GCF::Result GCF::ipcConnect(QObject *sender, const char *signal, const QHostAddress &addr,
                quint16 port, const QString &object, const QString &method);
\ingroup gcf_ipc

Connects \c signal of local \c QObject \c sender to \c method of a remote object by name \c object
available at \c port and \c addr address.

This function returns true upon success. False+error message upon failure.
*/
