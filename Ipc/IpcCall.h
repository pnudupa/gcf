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

#ifndef IPC_H
#define IPC_H

#include "IpcCommon.h"
#include <QHostAddress>
#include <QVariantList>

namespace GCF
{

struct IpcCallData;
class GCF_IPC_EXPORT IpcCall : public QObject
{
    Q_OBJECT

public:
    IpcCall(const QHostAddress &addr, quint16 port,
            const QString &object,
            const QString &method,
            const QVariantList &arguments,
            QObject *parent=nullptr);
    ~IpcCall();

    QHostAddress address() const;
    quint16 port() const;
    QString object() const;
    QString method() const;
    QVariantList arguments() const;

    bool isDone() const;

    bool isSuccess() const;
    QString errorMessage() const;
    QVariant result() const;

    void setTimeoutDuration(int val);

    bool waitForDone();

    void setAutoDelete(bool val);
    bool isAutoDelete() const;

signals:
    virtual void done(bool success);

private:
    void emitDone(bool success, const QString &msg);
    int timeoutDuration() const;

private slots:
    void onCall();
    void onConnected();
    void onReadyRead();
    void onBytesWritten();
    void onDisconnected();
    void onConnectTimeout();
    void onCallTimeout();

protected:
    int messageId() const;

private:
    IpcCallData *d;
};

}

#endif // IPC_H
