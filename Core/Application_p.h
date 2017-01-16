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

#ifndef APPLICATION_P_H
#define APPLICATION_P_H

#include "GCFGlobal.h"

namespace GCF
{

class InvokeMethodHelper
{
public:
    InvokeMethodHelper(bool sc=true) : SecureCall(sc) { }
    bool SecureCall;

    GCF::Result call(const QString &path, const QString &method, const QVariantList &args);
    GCF::Result call(QObject *object, const QString &method, const QVariantList &args);
    GCF::Result call(QObject *object, const QMetaMethod &method, const QVariantList &args);
    GCF::Result isMethodInvokable(const QMetaMethod &method, QObject *object=0);

private:
    GCF::Result call2(QObject *object, const QMetaMethod &method, const QVariantList &args);
    GCF::Result errorResult(const QString &msg) const { return GCF::Result(false, QString(), msg, QVariant()); }
    GCF::Result result(const QVariant &v) { return GCF::Result(true, QString(), QString(), v); }
};

}

#endif // APPLICATION_P_H
