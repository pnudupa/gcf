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

#ifndef CALLERTHREAD_H
#define CALLERTHREAD_H

#include <QThread>
#include <GCF3/IpcCall>

class Caller : public QObject
{
    Q_OBJECT

public:
    Caller(QObject *parent=0)
        : QObject(parent), m_done(false), m_success(false) { }
    ~Caller() { }

    bool isDone() const { return m_done; }
    bool wasCallSuccessful() const { return m_success; }
    QString callMessage() const { return m_message; }

signals:
    void done(bool);

public slots:
    void beginCall() {
        GCF::IpcCall *call = new GCF::IpcCall(QHostAddress::LocalHost, 49772,
                                              "Application.TestService",
                                              "largeReturnType",
                                              QVariantList() << 65536,
                                              this);
        connect(call, SIGNAL(done(bool)), this, SLOT(onCallDone(bool)));
    }

private slots:
    void onCallDone(bool success) {
        GCF::IpcCall *call = qobject_cast<GCF::IpcCall*>(this->sender());
        m_done = true;
        m_success = success;
        m_message = call->errorMessage();
        emit done(m_success);
        if(QThread::currentThread() != qApp->thread())
            QThread::currentThread()->quit();
    }

private:
    bool m_done;
    bool m_success;
    QString m_message;
};

#endif // CALLERTHREAD_H
