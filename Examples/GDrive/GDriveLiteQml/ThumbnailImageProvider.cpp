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

#include "ThumbnailImageProvider.h"

#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include <GCF3/SignalSpy>

Q_GLOBAL_STATIC(QNetworkAccessManager, ThumbnailNetworkAccess)

inline QImage scaledImage(const QImage &image, QSize *size, const QSize &requestedSize)
{
    QImage ret;
    if(image.size().isEmpty())
        ret = QImage(":/Icons/icon-404-error.jpg");
    else
        ret = image;
    if(!requestedSize.isEmpty())
        ret = ret.scaled(requestedSize);
    if(size)
        *size = ret.size();
    return ret;
}

QImage ThumbnailImageProvider::requestImage(const QString &urlStr, QSize *size, const QSize &requestedSize)
{
    if(urlStr.isEmpty())
        return ::scaledImage(QImage(), size, requestedSize);

    QString authString = QString("Bearer %1").arg(m_gDriveLite->accessToken());

    QUrl url(urlStr);
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Authorization", authString.toLatin1());

    QNetworkReply *reply = ::ThumbnailNetworkAccess()->get(networkRequest);
    GCF::SignalSpy spy(reply, SIGNAL(finished()));

    if(spy.wait(20000))
    {
        QString type = reply->header(QNetworkRequest::ContentTypeHeader).toString();
        QByteArray bytes = reply->readAll();
        delete reply;

        QString format = type.split('/').last();
        if(format.isEmpty())
            format = "png";
        format = format.toUpper();

        QImage image;
        image.loadFromData(bytes, qPrintable(format));
        return ::scaledImage(image, size, requestedSize);
    }

    delete reply;
    if(size)
        *size = QSize();
    return QImage();
}

