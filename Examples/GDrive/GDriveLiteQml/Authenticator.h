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

#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <QObject>
#include <GCF3/IGDriveLite>

class Authenticator : public QObject
{
    Q_OBJECT

public:
    Authenticator(GCF::IGDriveLite *parent=0) : QObject(parent), m_gDriveLite(parent) { }
    ~Authenticator() { }

    Q_PROPERTY(QString authenticationPageUrl READ authenticationPageUrl CONSTANT)
    QString authenticationPageUrl() const {
        return m_gDriveLite->authenticator()->authenticationPageUrl().toString();
    }

    Q_INVOKABLE bool authenticateFromPageTitle(const QString &title) {
        GCF::Result result = m_gDriveLite->authenticator()->authenticateFromPageTitle(title, this,
            SLOT(authenticationDone(GCF::IGDriveLiteAuthenticator*)));
        return result.isSuccess();
    }

    Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY authenticatedChanged)
    bool isAuthenticated() const { return m_gDriveLite->isAuthenticated(); }
    Q_SIGNAL void authenticatedChanged();

private:
    Q_SLOT void authenticationDone(GCF::IGDriveLiteAuthenticator *) {
        emit authenticatedChanged();
    }

private:
    GCF::IGDriveLite *m_gDriveLite;
};

#endif // AUTHENTICATOR_H
