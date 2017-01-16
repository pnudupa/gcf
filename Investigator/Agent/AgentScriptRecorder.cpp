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

#include "AgentScriptRecorder.h"
#include <QApplication>
#include "../Core/Application.h"
#include "../Core/ObjectMap.h"

#include <QWidget>
#if QT_VERSION >= 0x050000
#include <QWindow>
#endif
#include <QMetaEnum>
#include <QMetaMethod>
#include <QMetaObject>
#include <QTextStream>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

struct AgentScriptRecorderData
{
    AgentScriptRecorderData() : isRecording(false),
        eventTimeGapThreshold(750) { }

    QString code;
    bool isRecording;
    QString indent;

    QDateTime lastEventTimestamp;
    const qint64 eventTimeGapThreshold; // event more than this treshold have to sleep!
    qint64 roundOff(qint64 gap, qint64 nearest=500) {
        qint64 msecs = gap;
        qint64 diff = (msecs % nearest);
        if(diff < (nearest >> 1))
            msecs -= diff;
        else
            msecs += ( nearest-diff );
        return msecs;
    }

    QString lastKeyEventCode; // to optimize press-release into click
    QString lastMouseEventCode;

    GCF::ObjectMap<QString> variableNameMap;
    QString variableName(QObject *object);

    QString modifiers(Qt::KeyboardModifiers mods) const;
    QString button(Qt::MouseButton btn) const;
    QString key(int keyCode) const;
};

AgentScriptRecorder::AgentScriptRecorder(QObject *parent) :
    QObject(parent)
{
    d = new AgentScriptRecorderData;
}

AgentScriptRecorder::~AgentScriptRecorder()
{
    delete d;
}

QString AgentScriptRecorder::recordedTestCaseCode() const
{
    if(d->isRecording)
        return QString();

    return d->code;
}

QString AgentScriptRecorder::objectPath(QObject *object)
{
    GCF::ObjectTreeNode *node = gAppService->objectTree()->node(object);
    if(node)
        return node->path();

    QObject *parent = object->parent();
    if(!parent)
        return QString();

    QString objectName;
    if(object->objectName().isEmpty())
    {
        QObjectList siblings = parent->children();
        QObjectList classSiblings;
        Q_FOREACH(QObject *sibling, siblings)
        {
            if( sibling->inherits(object->metaObject()->className()) )
                classSiblings.append(sibling);
        }

        int index = classSiblings.indexOf(object);
        if(index < 0)
            return QString();

        objectName = QString("%1[%2]").arg(object->metaObject()->className()).arg(index);
    }
    else
        objectName = object->objectName();

    if(objectName.isEmpty())
        return QString();

    QString parentPath = AgentScriptRecorder::objectPath(parent);
    if(parentPath.isEmpty())
        return QString();

    QString retPath = QString("%1/%2").arg(parentPath).arg(objectName);
    return retPath;
}

void AgentScriptRecorder::startRecording()
{
    if(d->isRecording)
        return;

    d->code.clear();
    d->lastKeyEventCode.clear();
    d->lastMouseEventCode.clear();
    d->variableNameMap.removeAll();
    d->lastEventTimestamp = QDateTime();
    d->isRecording = true;

    QTextStream ts(&d->code, QIODevice::WriteOnly);
    ts << "function test_function() {\n";
    d->indent = QString("    ");

    QCoreApplication::instance()->installEventFilter(this);
    d->lastEventTimestamp = QDateTime::currentDateTime();
}

void AgentScriptRecorder::stopRecording()
{
    if(!d->isRecording)
        return;

    QCoreApplication::instance()->removeEventFilter(this);

    d->isRecording = false;
    d->lastKeyEventCode.clear();
    d->lastMouseEventCode.clear();
    d->variableNameMap.removeAll();
    d->lastEventTimestamp = QDateTime();
    d->indent.clear();

    QTextStream ts(&d->code, QIODevice::WriteOnly);
    ts << "}\n";
}

bool AgentScriptRecorder::eventFilter(QObject *object, QEvent *event)
{
    static QList<QEvent::Type> capturedEvents = QList<QEvent::Type>()
            << QEvent::MouseButtonDblClick << QEvent::MouseButtonPress
            << QEvent::MouseButtonRelease << QEvent::MouseMove
            << QEvent::KeyPress << QEvent::KeyRelease << QEvent::Wheel;

#if QT_VERSION >= 0x050000
    if((!object->isWidgetType() && !object->isWindowType()) || !capturedEvents.contains(event->type()))
#else
    if(!object->isWidgetType() || !capturedEvents.contains(event->type()))
#endif
        return false;

    if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
    {
        QObject *focusObject = qApp->focusWidget();

#if QT_VERSION >= 0x050000
        if(!focusObject && qApp->focusWindow())
            focusObject = qApp->focusWindow();
#endif

        if(object != focusObject)
            return false;
    }

    QTextStream ts(&d->code, QIODevice::WriteOnly);

    /*
     * We need not capture events for instances of the following classnames
     */
    const QList<QByteArray> excludeKeyEventClasses = QList<QByteArray>()
            << QByteArray("QGroupBox") << QByteArray("QFrame")
            << QByteArray("QStackedWidget") << QByteArray("QWidgetWindow");
    for(int i=0; i<excludeKeyEventClasses.count(); i++)
    {
        if( QByteArray(object->metaObject()->className()) == excludeKeyEventClasses.at(i) )
            return false;
    }

    QString objPath = this->objectPath(object);
    if(objPath.isEmpty())
    {
        ts << d->indent << "// Object of class " << object->metaObject()->className() << " has "
                 << "no object-name. Cannot capture events for this object!\n";
        return false;
    }

    // If more than eventTimeGapThreshold ms have passed since the last event capture
    // then lets introduce an appropriate sleep.
    QDateTime timestamp = QDateTime::currentDateTime();
    qint64 timegap = d->lastEventTimestamp.msecsTo(timestamp);
    if( timegap > d->eventTimeGapThreshold )
        ts << d->indent << "wait(" << d->roundOff(timegap, d->eventTimeGapThreshold) << ")\n\n";
    d->lastEventTimestamp = timestamp;

    /*
     * Establish a unique variable name for this widget/window first
     * Call to d->variableName() also generates the corresponding var ... = ... line
     * in the code.
     */
    QString varName = d->variableName(object);

    /*
     * Now lets capture the event in the script
     */
    switch(event->type())
    {
    case QEvent::KeyPress:
        {
            // KeyPress code is written into lastEventCode
            // If a KeyRelease happens within d->eventTimeGapThreshold then
            // we compress them into a keyClick.
            QKeyEvent *ke = (QKeyEvent*)event;
            QString tmpStr;
            QTextStream ts2(&tmpStr, QIODevice::WriteOnly);
            // if(ke->text().isEmpty())
            if(ke->key() != Qt::NoModifier)
                ts2 << d->indent << varName << ".keyPress("
                   << d->key(ke->key()) << ", " << d->modifiers(ke->modifiers()) << ")\n";
            else
                ts2 << d->indent << varName << ".keyPress(\""
                   << ke->text().at(0) << "\", " << d->modifiers(ke->modifiers()) << ")\n";
            d->lastKeyEventCode = tmpStr;
        } break;
    case QEvent::KeyRelease:
        {
            QKeyEvent *ke = (QKeyEvent*)event;
            if(timegap < d->eventTimeGapThreshold)
            {
                d->lastKeyEventCode.clear();
                if(ke->text().isEmpty())
                    ts << d->indent << varName << ".keyClick("
                       << d->key(ke->key()) << ", " << d->modifiers(ke->modifiers()) << ")\n";
                else
                    ts << d->indent << varName << ".keyClicks(\""
                       << ke->text() << "\", " << d->modifiers(ke->modifiers()) << ")\n";
            }
            else
            {
                ts << d->lastKeyEventCode;
                d->lastKeyEventCode.clear();

                if(ke->text().isEmpty())
                    ts << d->indent << varName << ".keyRelease("
                       << d->key(ke->key()) << ", " << d->modifiers(ke->modifiers()) << ")\n";
                else
                    ts << d->indent << varName << ".keyRelease(\""
                       << ke->text().at(0) << "\", " << d->modifiers(ke->modifiers()) << ")\n";
            }
        } break;
    case QEvent::MouseButtonPress:
        {
            // MousePress code is written into lastEventCode
            // If a MouseRelease happens within d->eventTimeGapThreshold then
            // we compress them into a mouseClick.
            QMouseEvent *me = (QMouseEvent*)event;
            QString tmpStr;
            QTextStream ts2(&tmpStr, QIODevice::WriteOnly);
            ts2 << "\n" << d->indent << varName << ".mousePressAtPos(" << me->x()
                << ", " << me->y() << ", " << d->button(me->button()) << ", "
                << d->modifiers(me->modifiers())
                << ")\n";
            d->lastMouseEventCode = tmpStr;
        } break;
    case QEvent::MouseMove:
        {
            // If the mouse has moved - and there was a code for press before, then
            // we should write the last mouse-event-code and the move code.
            if(!d->lastMouseEventCode.isEmpty())
            {
                // Capture mouse-movement - only after a press!
                ts << d->lastMouseEventCode;
                d->lastMouseEventCode.clear();

                QMouseEvent *me = (QMouseEvent*)event;
                ts << d->indent << varName << ".mouseMove(" << me->x() << ", "
                   << me->y() << ")\n";
            }
        } break;
    case QEvent::MouseButtonRelease:
        {
            QMouseEvent *me = (QMouseEvent*)event;
            if(d->lastMouseEventCode.isEmpty())
                ts << d->indent << varName << ".mouseReleaseAtPos(" << me->x()
                    << ", " << me->y() << ", " << d->button(me->button()) << ", "
                    << d->modifiers(me->modifiers()) << ")\n";
            else
            {
                if(timegap > d->eventTimeGapThreshold)
                {
                    ts << d->lastMouseEventCode;
                    d->lastMouseEventCode.clear();
                    ts << d->indent << varName << ".mouseReleaseAtPos(" << me->x()
                        << ", " << me->y() << ", " << d->button(me->button()) << ", "
                        << d->modifiers(me->modifiers()) << ")\n";
                }
                else
                {
                    ts << d->indent << varName << ".mouseClickAtPos(" << me->x()
                        << ", " << me->y() << ", " << d->button(me->button()) << ", "
                        << d->modifiers(me->modifiers()) << ")\n";
                }
            }

            ts << "\n";
        } break;
    case QEvent::MouseButtonDblClick:
        {
            QMouseEvent *me = (QMouseEvent*)event;
            ts << d->indent << varName << ".mouseDClickAtPos(" << me->x()
                << ", " << me->y() << ", " << d->button(me->button()) << ", "
                << d->modifiers(me->modifiers()) << ")\n";
        } break;
    case QEvent::Wheel:
        {
            QWheelEvent *we = (QWheelEvent*)event;
            if(we->delta() > 0)
                ts << d->indent << varName << ".wheelAtPos(" << we->x()
                   << ", " << we->y() << ", " << we->delta() << ", "
                   << d->modifiers(we->modifiers()) << ")\n";
            else
                ts << d->indent << varName << ".wheelAtPos(" << varName << ", " << we->x()
                   << ", " << we->y() << ", " << we->delta() << ", "
                   << d->modifiers(we->modifiers()) << ")\n";
        } break;
    default:
        break;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

QString AgentScriptRecorderData::variableName(QObject *object)
{
    QString varName = this->variableNameMap.value(object, QString());
    if(varName.isEmpty())
    {
        varName = object->objectName();
        varName = varName.simplified();
        if(varName.isEmpty())
        {
#if QT_VERSION >= 0x050000
            if( (object->isWidgetType() && ((QWidget*)(object))->isWindow()) ||
                (object->isWindowType() && ((QWindow*)(object))->isTopLevel()) )
#else
            if( ((QWidget*)(object))->isWindow() )
#endif
                varName = "window";
            else
                varName = QString::fromLatin1(object->metaObject()->className()).toLower();
        }
        else
        {
            if(varName.at(0).isDigit())
                varName.prepend("w_");
            varName.replace(" ", "_");
        }

        // Replace . with _
        varName.replace('.', '_');

        // Replace :: with _
        varName.replace("::", "_");

        QStringList values = this->variableNameMap.values();
        QString baseName = varName;
        int count = 1;
        while(1)
        {
            if( values.contains(varName) )
                varName = QString("%1%2").arg(baseName).arg(count++);
            else
                break;
        }

        if(varName.at(0).isUpper())
            varName[0] = varName.at(0).toLower();

        QTextStream ts(&code, QIODevice::WriteOnly);
        ts << this->indent << "var " << varName << " = " << "object(\""
           << AgentScriptRecorder::objectPath(object) << "\")\n";
        this->variableNameMap.insert(object, varName);
        this->variableNameMap[object] = varName;
    }

    return varName;
}

QString AgentScriptRecorderData::modifiers(Qt::KeyboardModifiers mods) const
{
    static QMap<Qt::KeyboardModifier,QString> map;
    if( map.isEmpty() )
    {
        map[Qt::ShiftModifier] = "Qt.ShiftModifier";
        map[Qt::ControlModifier] = "Qt.ControlModifier";
        map[Qt::AltModifier] = "Qt.AltModifier";
        map[Qt::MetaModifier] = "Qt.MetaModifier";
        map[Qt::KeypadModifier] = "Qt.KeypadModifier";
        map[Qt::GroupSwitchModifier] = "Qt.GroupSwitchModifier";
    }

    QList<Qt::KeyboardModifier> keys = map.keys();
    QString retStr;
    if(mods == Qt::NoModifier)
        retStr = "Qt.NoModifier";
    else
    {
        Q_FOREACH(Qt::KeyboardModifier key, keys)
        {
            if(mods&key)
            {
                if(retStr.isEmpty())
                    retStr = map.value(key);
                else
                    retStr += "|" + map.value(key);
            }
        }
    }

    return retStr;
}

QString AgentScriptRecorderData::button(Qt::MouseButton btn) const
{
    static QMap<Qt::MouseButton,QString> map;
    if(map.isEmpty())
    {
        map[Qt::NoButton] = "Qt.NoButton";
        map[Qt::LeftButton] = "Qt.LeftButton";
        map[Qt::RightButton] = "Qt.RightButton";
        map[Qt::MiddleButton] = "Qt.MiddleButton";
    }

    return map.value(btn);
}

struct StaticQtMetaObject : public QObject
{
    static inline const QMetaObject &get() { return staticQtMetaObject; }
};

QString AgentScriptRecorderData::key(int keyCode) const
{
    const QMetaObject &mo = StaticQtMetaObject::get();
    QMetaEnum keyEnum = mo.enumerator( mo.indexOfEnumerator("Key") );
    return QString("Qt.%1").arg(keyEnum.valueToKey(keyCode));
}
