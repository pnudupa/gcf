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

#include "HTTPCaller.h"
#include "Json.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

HTTPCaller::HTTPCaller(const QVariantMap &jsonRequest,
                       QNetworkAccessManager &nam,
                       const QString &serverAddress,
                       QObject *parent)
    :QObject(parent), m_nam(nam), m_serverAddress(serverAddress)
{
    m_jsonRequest = Json().serialize(jsonRequest);
}

void HTTPCaller::call()
{
    QUrl url(m_serverAddress + "/FiberScript.php");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    QNetworkReply *reply = m_nam.post(request, m_jsonRequest);

    connect(reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
    connect(reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    connect(reply, SIGNAL(aboutToClose()), this, SLOT(onReplyFinished()), Qt::QueuedConnection);

    emit requestSent(m_jsonRequest);
}

void HTTPCaller::onReplyReadyRead()
{
}

void HTTPCaller::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    QByteArray data = reply->readAll();

    emit responseReceived(data);
}

