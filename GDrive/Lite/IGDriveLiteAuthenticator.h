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

#ifndef IGDriveLiteAuthenticator_H
#define IGDriveLiteAuthenticator_H

#include <QUrl>
#include <GCF3/GCFGlobal>

namespace GCF
{

class IGDriveLiteAuthenticator
{
public:
    virtual QUrl authenticationPageUrl() const = 0;
    virtual GCF::Result authenticateFromPageTitle(const QString &title,
                                           QObject *notifyObject=0,
                                           const char *notifySlot=0) = 0;
    virtual GCF::Result refreshAuthentication() = 0;
};

}

#endif // IGDriveLiteAuthenticator_H
