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

#ifndef IPCCOMMON_P_H
#define IPCCOMMON_P_H

#include <QTcpSocket>
#include <QByteArray>
#include <QDataStream>
#include <QMetaType>
#include "../Core/GCFGlobal.h"

namespace GCF
{

class IpcMessage
{
public:
    static QByteArray REQUEST_OBJECT;
    static QByteArray GET_PROPERTY_VALUE;
    static QByteArray SET_PROPERTY_VALUE;
    static QByteArray REQUEST_CONNECTION;
    static QByteArray SIGNAL_DELIVERY;
    static QByteArray IPC_CALL;

    IpcMessage() : m_id(-1) { }

    IpcMessage(const IpcMessage &other)
        : m_id(other.m_id),
          m_type(other.m_type),
          m_isResponse(other.m_isResponse),
          m_data(other.m_data),
          m_result(other.m_result) { }

    IpcMessage &operator = (const IpcMessage &other) {
        m_id = other.m_id;
        m_type = other.m_type;
        m_isResponse = other.m_isResponse;
        m_data = other.m_data;
        m_result = other.m_result;
        return *this;
    }

    IpcMessage(const QByteArray &type) : m_type(type), m_isResponse(false) {
        static qint32 id = 1;
        m_id = id++;
    }
    IpcMessage(qint32 id, const QByteArray &type) : m_id(id), m_type(type),
        m_isResponse(true) { }

    bool operator == (const IpcMessage &other) const {
        return m_id == other.m_id &&
                m_type == other.m_type &&
                m_isResponse == other.m_isResponse &&
                m_data == other.m_data &&
                m_result == other.m_result;
    }

    bool isValid() const { return m_id >= 0 && m_type.size() == 36; }
    qint32 id() const { return m_id; }
    QByteArray type() const { return m_type; }
    bool isRequest() const { return m_isResponse == false; }
    bool isResponse() const { return m_isResponse == true; }

    QVariantMap &data() { return m_data; }
    const QVariantMap &data() const { return m_data; }

    void setResult(const GCF::Result &result) { m_result = result; }
    GCF::Result result() const { return m_result; }

    static IpcMessage fromByteArray(const QByteArray &bytes);

    QByteArray toByteArray() const { return toByteArray(*this); }
    static QByteArray toByteArray(const IpcMessage &message);

private:
    qint32 m_id;
    QByteArray m_type;
    bool m_isResponse;
    QVariantMap m_data;
    GCF::Result m_result;
};

class IpcSocket : public QTcpSocket
{
    Q_OBJECT

public:
    IpcSocket(QObject *parent=0);
    ~IpcSocket();

    void sendMessage(const GCF::IpcMessage &message);

private slots:
    void onReadyRead();
    void onBytesWritten();

signals:
    void incomingMessage(const GCF::IpcMessage &message);
    void readBufferEmpty();
    void writeBufferEmpty();

private:
    qint32 m_incomingMessageSize;
};

}

Q_DECLARE_METATYPE(GCF::IpcMessage)

#endif // IPCCOMMON_P_H
