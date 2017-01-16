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

#ifndef AGENT_H
#define AGENT_H

#include <QtGlobal>

#include <QScriptValue>
#include <QScriptEngine>
#include <QScriptContext>
#include <QScriptContextInfo>
#include <QScriptValueIterator>

#if QT_VERSION >= 0x050000
#include <QQmlEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QQmlEngine>
#else
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#endif

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsObject>

#include "AgentComponent.h"

#endif // AGENT_H
