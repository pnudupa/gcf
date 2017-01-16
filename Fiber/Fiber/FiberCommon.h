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

#ifndef FIBERCOMMON_H
#define FIBERCOMMON_H

#include "Json.h"

#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <GCF3/Log>

namespace Fiber {

const QByteArray FiberMessageFooter = "<-FIBERFOOTER->";

inline QByteArray ErrorStringToJSON(const QString &errorMsg)
{
    GCF::Log::instance()->error(GCF_DEFAULT_LOG_CONTEXT, errorMsg);
    QVariantMap jObj;
    jObj["result"] = QVariant();
    jObj["error"] = errorMsg;
    jObj["success"] = false;
    QByteArray data = Json().serialize(jObj);
    return data;
}

}

#endif // FIBERCOMMON_H
