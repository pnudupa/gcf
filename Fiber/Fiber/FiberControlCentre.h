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

#ifndef FIBERCONTROLCENTRE_H
#define FIBERCONTROLCENTRE_H

#include <QObject>
#include <QPointer>
#include <QProcess>

class QLocalSocket;

struct FiberControlCentreData;
class FiberControlCentre : public QObject
{
    Q_OBJECT

public:
    static FiberControlCentre *instance();
    ~FiberControlCentre();
    
    QString createSessionForIP(const QString &clientIP);
    bool terminateSession(const QString &sessionName);

    QString handlerForSession(const QString &sessionName);
    QString localSocketForHandler(const QString &handlerName) const;

protected slots:
    void fiberControlServerNewConnection();
    void fiberControlServerReadyRead();
    void handlerDeactivated();

protected:
    FiberControlCentre(QObject *parent = 0);

signals:
    void handlerSocketUpdated(const QString &handlerName,
                                const QString &handlerSocket);

private:
    FiberControlCentreData *d;
};

class RequestHandlerAgent : public QObject
{
    Q_OBJECT

public:
    RequestHandlerAgent(QObject *parent=0);
    ~RequestHandlerAgent();

protected slots:
    void handlerProcessExited(int exitCode, QProcess::ExitStatus exitStatus);

protected:
    void activate();
    void deactivate();

signals:
    void activated();
    void deactivated();

private:
    friend class FiberControlCentre;
    friend struct FiberControlCentreData;
    QString m_session;
    QString m_handlerName;
    QPointer<QProcess> m_handlerProcess;
    QString m_handlerSocket;
    QStringList m_handledSessions;
    bool m_deactivating;
    QPointer<QLocalSocket> m_handhshakeSocket;
};


#endif // FIBERCONTROLCENTRE_H
