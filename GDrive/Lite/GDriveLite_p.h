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

#ifndef GDRIVELITE_P_H
#define GDRIVELITE_P_H

#include "GDriveLite.h"
#include "IGDriveLiteAuthenticator.h"

class QNetworkReply;

class GDriveLite;
class GDriveLiteAuthenticator : public QObject
{
    Q_OBJECT

public:
    GDriveLiteAuthenticator(const QString &successCode, GDriveLite *parent)
        : QObject(parent), m_successCode(successCode), m_gDriveLite(parent) {
        connect(this, SIGNAL(done(GCF::IGDriveLiteAuthenticator*)), this, SLOT(deleteLater()));
    }
    ~GDriveLiteAuthenticator() { }

    void requestAuthentication();

signals:
    void done(GCF::IGDriveLiteAuthenticator *auth);

private slots:
    void networkReplyFinished();

private:
    QString m_successCode;
    GDriveLite *m_gDriveLite;
};

class GDriveLiteAuthenticationRefresh : public QObject
{
    Q_OBJECT

public:
    GDriveLiteAuthenticationRefresh(GDriveLite *parent) : QObject(parent),
        m_gDriveLite(parent) { }
    ~GDriveLiteAuthenticationRefresh() { }

    void refreshAuthentication();

private slots:
    void networkReplyFinished();

private:
    GDriveLite *m_gDriveLite;
};

#endif // GDRIVELITE_P_H
