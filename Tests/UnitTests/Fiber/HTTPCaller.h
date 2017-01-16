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

#ifndef HTTPCALLER_H
#define HTTPCALLER_H

#include <QObject>
#include <QVariantMap>
#include <QNetworkAccessManager>

class HTTPCaller : public QObject
{
    Q_OBJECT

public:
    HTTPCaller(const QVariantMap &jsonRequest,
               QNetworkAccessManager &nam,
               const QString &serverAddress="http://localhost",
               QObject *parent=0);

    void call();

protected slots:
    void onReplyReadyRead();
    void onReplyFinished();

signals:
    void requestSent(const QByteArray &requestData);
    void responseReceived(const QByteArray &responseData);

private:
    QByteArray m_jsonRequest;
    QNetworkAccessManager &m_nam;
    QString m_serverAddress;
};

#endif // HTTPCALLER_H
