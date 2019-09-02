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

#include "IpcCommon_p.h"
#include "../Core/Log.h"
#include "../Core/Application.h"

#include <QHostAddress>

/*
 * Additional UUIDs can be used from the list below and then further generated if needed
 *
 * F1AA0C35-629F-40A4-95D2-8FD046CBE290
 * B284618E-3283-4597-A0CB-EC9EC95027B9
 * 5DC421C2-4563-4BAE-8243-E0C1723F9D7E
 * 325FDB5F-3E0B-43A5-8D4B-B3A36B0700EB
 * 4E60701D-6638-4D82-B895-A5FA07C6E8AC
 * B733B12A-DE2F-4383-81C7-C0940DC00A42
 *
 * Use uuidgen on Linux or Mac to generate UUIDs.
 */

QByteArray GCF::IpcMessage::REQUEST_OBJECT
    = QByteArray("653FC1D5-5D74-4903-A113-4F8AFCB9F074");

QByteArray GCF::IpcMessage::GET_PROPERTY_VALUE
    = QByteArray("BC73CC41-EBB2-403F-B143-628675385D76");

QByteArray GCF::IpcMessage::SET_PROPERTY_VALUE
    = QByteArray("AFA6B515-DCCD-429A-B8DC-B64E5C01BE82");

QByteArray GCF::IpcMessage::REQUEST_CONNECTION
    = QByteArray("398A7574-9186-4BEF-9459-FEA040E07EE9");

QByteArray GCF::IpcMessage::SIGNAL_DELIVERY
    = QByteArray("F1B30D27-48DF-4439-8788-C58057683551");

QByteArray GCF::IpcMessage::IPC_CALL
    = QByteArray("274DC568-A5C2-4575-AE24-F4D001AC71CF");

GCF::IpcMessage GCF::IpcMessage::fromByteArray(const QByteArray &bytes)
{
    GCF::IpcMessage message;

    QDataStream ds(bytes);
    ds >> message.m_id;
    ds >> message.m_type;
    ds >> message.m_isResponse;
    ds >> message.m_data;

    bool success;
    QString code;
    QString error;
    QVariant result;
    ds >> success >> code >> error >> result;
    message.setResult( GCF::Result(success, code, error, result) );

    return message;
}

QByteArray GCF::IpcMessage::toByteArray(const IpcMessage &message)
{
    QByteArray bytes;

    QDataStream ds(&bytes, QIODevice::WriteOnly);
    ds << message.id();
    ds << message.type();
    ds << message.isResponse();
    ds << message.data();
    ds << message.result().isSuccess();
    ds << message.result().code();
    ds << message.result().message();
    ds << message.result().data();

    return bytes;
}

///////////////////////////////////////////////////////////////////////////////

GCF::IpcSocket::IpcSocket(QObject *parent)
    :QTcpSocket(parent), m_incomingMessageSize(0)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(this, SIGNAL(bytesWritten(qint64)), this, SLOT(onBytesWritten()));
}

GCF::IpcSocket::~IpcSocket()
{
}

void GCF::IpcSocket::sendMessage(const GCF::IpcMessage &message)
{
    if(this->state() != QTcpSocket::ConnectedState)
    {
        emit writeBufferEmpty();
        return;
    }

    QByteArray packet;
    QDataStream ds(&packet, QIODevice::WriteOnly);
    ds << message.toByteArray();

    connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));
    this->write(packet);

    QString msg = QString("Sending message-id %1 of type %2 int %3 bytes to %4:%5")
            .arg(message.id())
            .arg(QString::fromLatin1(message.type()))
            .arg(packet.size()-int(sizeof(qint32)))
            .arg(this->peerAddress().toString())
            .arg(this->peerPort());
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, msg);
}

void GCF::IpcSocket::onReadyRead()
{
    if(m_incomingMessageSize == 0)
    {
        QDataStream ds(this);
        ds >> m_incomingMessageSize;
    }

    if( qint32(this->bytesAvailable()) < m_incomingMessageSize )
        return; // When this slot is called next, maybe the whole message is available!

    QByteArray messageBytes = this->read(qint64(m_incomingMessageSize));
    GCF::IpcMessage message = GCF::IpcMessage::fromByteArray(messageBytes);
    m_incomingMessageSize = 0;

    QString msg = QString("Processing incoming message-id %1 of type %2 from %3 bytes from %4:%5")
            .arg(message.id())
            .arg(QString::fromLatin1(message.type()))
            .arg(messageBytes.size())
            .arg(this->peerAddress().toString())
            .arg(this->peerPort());
    GCF::Log::instance()->info(GCF_DEFAULT_LOG_CONTEXT, msg);

    emit incomingMessage(message);

    if(this->bytesAvailable() == 0)
        emit readBufferEmpty();
    else
        QMetaObject::invokeMethod(this, "onReadyRead", Qt::QueuedConnection);
}

void GCF::IpcSocket::onBytesWritten()
{
    if(this->bytesToWrite() == 0)
        emit writeBufferEmpty();
}

