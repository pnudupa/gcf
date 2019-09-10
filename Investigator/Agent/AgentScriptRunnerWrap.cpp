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

#include "AgentScriptRunner.h"

#include <QtTest>
#include <QWidget>
#include <QComboBox>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#if QT_VERSION >= 0x050000
#include <QWindow>
#endif

// Function pointer type for functions that wrap a particular class-type
typedef void (*WrapFunction)(QScriptValue &value);

void wrapQObject(QScriptValue &value);
void wrapQWidget(QScriptValue &value);
#if QT_VERSION >= 0x050000
void wrapQWindow(QScriptValue &value);
#endif
void wrapQComboBox(QScriptValue &value);
void wrapQGraphicsObject(QScriptValue &value);

void AgentScriptRunner::wrap(QScriptValue &value)
{
    static QMap<QByteArray,WrapFunction> wrapFunctionMap;
    if(wrapFunctionMap.isEmpty())
    {
        wrapFunctionMap["QObject"] = wrapQObject;
        wrapFunctionMap["QWidget"] = wrapQWidget;
        wrapFunctionMap["QComboBox"] = wrapQComboBox;
        wrapFunctionMap["QGraphicsObject"] = wrapQGraphicsObject;
#if QT_VERSION >= 0x050000
        wrapFunctionMap["QWindow"] = wrapQWindow;
#endif
    }

    if( !value.isQObject() )
        return;

    QObject *obj = value.toQObject();
    QMap<QByteArray,WrapFunction>::const_iterator it = wrapFunctionMap.begin();
    QMap<QByteArray,WrapFunction>::const_iterator end = wrapFunctionMap.end();
    while(it != end)
    {
        if( obj->inherits(it.key()) )
            it.value()(value);
        ++it;
    }
}

////////////////////////////////////////////////////////////////////////////////
// QObject functions
////////////////////////////////////////////////////////////////////////////////

static QScriptValue QObject_parent(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QObject *object = thisObject.toQObject();
    if(!object)
        return QScriptValue(engine, QScriptValue::NullValue);

    if(object->parent())
    {
        QScriptValue parentValue = engine->newQObject(object->parent());
        AgentScriptRunner::wrap(parentValue);
        return parentValue;
    }

    return QScriptValue(engine, QScriptValue::NullValue);
}

static QScriptValue QObject_object(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QObject *object = thisObject.toQObject();
    if(!object)
        return QScriptValue(engine, QScriptValue::NullValue);

    if(context->argumentCount() != 1)
        return QScriptValue(engine, QScriptValue::NullValue);

    QString path = context->argument(0).toString();
    QStringList comps = path.split('/', QString::SkipEmptyParts);
    if(comps.length() == 0)
        return QScriptValue(engine, QScriptValue::NullValue);

    QObject *childObject = object;
    Q_FOREACH(QString comp, comps)
    {
        childObject = AgentScriptRunner::findObject(childObject, comp);
        if(!childObject)
            return QScriptValue(engine, QScriptValue::NullValue);
    }

    if(childObject)
    {
        QScriptValue childObjectValue = engine->newQObject(childObject);
        AgentScriptRunner::wrap(childObjectValue);
        return childObjectValue;
    }

    return QScriptValue(engine, QScriptValue::NullValue);
}

void wrapQObject(QScriptValue &value)
{
    value.setProperty("parent", value.engine()->newFunction(QObject_parent),
                      QScriptValue::PropertyGetter);
    value.setProperty("object", value.engine()->newFunction(QObject_object));
}

////////////////////////////////////////////////////////////////////////////////
// QWidget/QWindow template functions
////////////////////////////////////////////////////////////////////////////////

template <class T>
static QScriptValue UI_setGeometry(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    // We must have 4 arguments to this function
    if(context->argumentCount() != 4)
        return QScriptValue(engine, false);

    qint32 x = context->argument(0).toInt32();
    qint32 y = context->argument(1).toInt32();
    qint32 width = context->argument(2).toInt32();
    qint32 height = context->argument(3).toInt32();
    w->setGeometry(x, y, width, height);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_resize(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    // We must have 2 arguments to this function
    if(context->argumentCount() != 2)
        return QScriptValue(engine, false);

    qint32 width = context->argument(0).toInt32();
    qint32 height = context->argument(1).toInt32();
    w->resize(width, height);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_mousePress(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    // Must have 2 arguments: button and modifiers
    if(context->argumentCount() != 2)
        return QScriptValue(engine, false);

    Qt::MouseButton button = Qt::MouseButton( context->argument(0).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(1).toInt32() );
    QTest::mousePress(w, button, modifiers, QRect(QPoint(0,0),w->size()).center(), -1);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_mousePressAtPos(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    // Must have 4 arguments: button and modifiers
    if(context->argumentCount() != 4)
        return QScriptValue(engine, false);

    qint32 x = context->argument(0).toInt32();
    qint32 y = context->argument(1).toInt32();
    Qt::MouseButton button = Qt::MouseButton( context->argument(2).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(3).toInt32() );
    QTest::mousePress(w, button, modifiers, QPoint(x,y), -1);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_mouseRelease(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    // Must have 2 arguments: button and modifiers
    if(context->argumentCount() != 2)
        return QScriptValue(engine, false);

    Qt::MouseButton button = Qt::MouseButton( context->argument(0).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(1).toInt32() );
    QTest::mouseRelease(w, button, modifiers, QRect(QPoint(0,0),w->size()).center(), -1);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_mouseReleaseAtPos(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    // Must have 4 arguments: button and modifiers
    if(context->argumentCount() != 4)
        return QScriptValue(engine, false);

    qint32 x = context->argument(0).toInt32();
    qint32 y = context->argument(1).toInt32();
    Qt::MouseButton button = Qt::MouseButton( context->argument(2).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(3).toInt32() );
    QTest::mouseRelease(w, button, modifiers, QPoint(x,y), -1);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_mouseMove(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    // Must have 4 arguments: button and modifiers
    if(context->argumentCount() != 2)
        return QScriptValue(engine, false);

    qint32 x = context->argument(0).toInt32();
    qint32 y = context->argument(1).toInt32();
    QTest::mouseMove(w, QPoint(x,y));

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_mouseClick(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    // Must have 2 arguments: button and modifiers
    if(context->argumentCount() != 2)
        return QScriptValue(engine, false);

    Qt::MouseButton button = Qt::MouseButton( context->argument(0).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(1).toInt32() );
    QTest::mouseClick(w, button, modifiers, QRect(QPoint(0,0),w->size()).center(), -1);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_mouseClickAtPos(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    // Must have 4 arguments: button and modifiers
    if(context->argumentCount() != 4)
        return QScriptValue(engine, false);

    qint32 x = context->argument(0).toInt32();
    qint32 y = context->argument(1).toInt32();
    Qt::MouseButton button = Qt::MouseButton( context->argument(2).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(3).toInt32() );
    QTest::mouseClick(w, button, modifiers, QPoint(x,y), -1);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_mouseDClick(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    // Must have 2 arguments: button and modifiers
    if(context->argumentCount() != 2)
        return QScriptValue(engine, false);

    Qt::MouseButton button = Qt::MouseButton( context->argument(0).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(1).toInt32() );
    QTest::mouseDClick(w, button, modifiers, QRect(QPoint(0,0),w->size()).center(), -1);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_mouseDClickAtPos(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    // Must have 4 arguments: button and modifiers
    if(context->argumentCount() != 4)
        return QScriptValue(engine, false);

    qint32 x = context->argument(0).toInt32();
    qint32 y = context->argument(1).toInt32();
    Qt::MouseButton button = Qt::MouseButton( context->argument(2).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(3).toInt32() );
    QTest::mouseDClick(w, button, modifiers, QPoint(x,y), -1);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_keyPress(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    if(context->argumentCount() != 2)
        return QScriptValue(engine, false);

    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(1).toInt32() );
    if(context->argument(0).isString())
    {
        QString text = context->argument(0).toString();
        if(text.isEmpty())
            return QScriptValue(engine, false);
        QTest::keyPress(w, text.at(0).toLatin1(), modifiers);
    }
    else
        QTest::keyPress(w, Qt::Key(context->argument(0).toInt32()), modifiers);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_keyRelease(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    if(context->argumentCount() != 2)
        return QScriptValue(engine, false);

    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(1).toInt32() );
    if(context->argument(0).isString())
    {
        QString text = context->argument(0).toString();
        if(text.isEmpty())
            return QScriptValue(engine, false);
        QTest::keyRelease(w, text.at(0).toLatin1(), modifiers);
    }
    else
        QTest::keyRelease(w, Qt::Key(context->argument(0).toInt32()), modifiers);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_keyClick(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    if(context->argumentCount() != 2)
        return QScriptValue(engine, false);

    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(1).toInt32() );
    if(context->argument(0).isString())
    {
        QString text = context->argument(0).toString();
        if(text.isEmpty())
            return QScriptValue(engine, false);
        QTest::keyClick(w, text.at(0).toLatin1(), modifiers);
    }
    else
        QTest::keyClick(w, Qt::Key(context->argument(0).toInt32()), modifiers);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_keyClicks(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    if(context->argumentCount() != 2)
        return QScriptValue(engine, false);

    QString text = context->argument(0).toString();
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(1).toInt32() );
    for(int i=0; i<text.length(); i++)
        QTest::keyClick(w, text.at(i).toLatin1(), modifiers);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_waitForShown(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

#if QT_VERSION >= 0x050000
    const bool success = QTest::qWaitForWindowActive(w);
#else
    const bool success = QTest::qWaitForWindowShown(w);
#endif

    return QScriptValue(engine, success);
}

template <class T>
static QScriptValue UI_wheel(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    if(context->argumentCount() != 2)
        return QScriptValue(engine, false);

    qint32 factor = context->argument(0).toInt32();
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers(context->argument(1).toInt32());

    QWheelEvent we( QRect(QPoint(0,0),w->size()).center(), factor, Qt::NoButton, modifiers );
    qApp->notify(w, &we);

    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_wheelAtPos(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    if(context->argumentCount() != 4)
        return QScriptValue(engine, false);

    qint32 x = context->argument(0).toInt32();
    qint32 y = context->argument(1).toInt32();
    qint32 factor = context->argument(2).toInt32();
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers(context->argument(3).toInt32());

    QWheelEvent we( QPoint(x,y), factor, Qt::NoButton, modifiers );
    qApp->notify(w, &we);

    return QScriptValue(engine, true);
}

static QScriptValue QWidget_keyboardFocus(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QWidget *w = qobject_cast<QWidget*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    if(context->argumentCount() == 1)
    {
        bool focus = context->argument(0).toBool();
        if(focus)
            w->setFocus();
        else
            w->clearFocus();
    }

    return QScriptValue(engine, w->hasFocus());
}

#if QT_VERSION >= 0x050000
static QScriptValue QWindow_keyboardFocus(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QWindow *w = qobject_cast<QWindow*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    return QScriptValue(engine, QGuiApplication::focusWindow() == w);
}
#endif

template <class T>
static QScriptValue UI_show(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    w->show();
    return QScriptValue(engine, true);
}

template <class T>
static QScriptValue UI_hide(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    T *w = qobject_cast<T*>(thisObject.toQObject());
    if(!w)
        return QScriptValue(engine, false);

    w->show();
    return QScriptValue(engine, true);
}

////////////////////////////////////////////////////////////////////////////////
// QWidget functions
////////////////////////////////////////////////////////////////////////////////

void wrapQWidget(QScriptValue &value)
{
    value.setProperty("setGeometry", value.engine()->newFunction(UI_setGeometry<QWidget>));
    value.setProperty("resize", value.engine()->newFunction(UI_resize<QWidget>));
    value.setProperty("mousePress", value.engine()->newFunction(UI_mousePress<QWidget>));
    value.setProperty("mousePressAtPos", value.engine()->newFunction(UI_mousePressAtPos<QWidget>));
    value.setProperty("mouseRelease", value.engine()->newFunction(UI_mouseRelease<QWidget>));
    value.setProperty("mouseReleaseAtPos", value.engine()->newFunction(UI_mouseReleaseAtPos<QWidget>));
    value.setProperty("mouseMove", value.engine()->newFunction(UI_mouseMove<QWidget>));
    value.setProperty("mouseClick", value.engine()->newFunction(UI_mouseClick<QWidget>));
    value.setProperty("mouseClickAtPos", value.engine()->newFunction(UI_mouseClickAtPos<QWidget>));
    value.setProperty("mouseDClick", value.engine()->newFunction(UI_mouseDClick<QWidget>));
    value.setProperty("mouseDClickAtPos", value.engine()->newFunction(UI_mouseDClickAtPos<QWidget>));
    value.setProperty("keyPress", value.engine()->newFunction(UI_keyPress<QWidget>));
    value.setProperty("keyRelease", value.engine()->newFunction(UI_keyRelease<QWidget>));
    value.setProperty("keyClick", value.engine()->newFunction(UI_keyClick<QWidget>));
    value.setProperty("keyClicks", value.engine()->newFunction(UI_keyClicks<QWidget>));
    value.setProperty("waitForShown", value.engine()->newFunction(UI_waitForShown<QWidget>));
    value.setProperty("wheel", value.engine()->newFunction(UI_wheel<QWidget>));
    value.setProperty("wheelAtPos", value.engine()->newFunction(UI_wheelAtPos<QWidget>));
    value.setProperty("keyboardFocus", value.engine()->newFunction(QWidget_keyboardFocus),
                      QScriptValue::PropertySetter|QScriptValue::PropertyGetter);
    value.setProperty("show", value.engine()->newFunction(UI_show<QWidget>));
    value.setProperty("hide", value.engine()->newFunction(UI_hide<QWidget>));
}

////////////////////////////////////////////////////////////////////////////////
// QWindow functions
////////////////////////////////////////////////////////////////////////////////

#if QT_VERSION >= 0x050000
void wrapQWindow(QScriptValue &value)
{
    value.setProperty("setGeometry", value.engine()->newFunction(UI_setGeometry<QWindow>));
    value.setProperty("resize", value.engine()->newFunction(UI_resize<QWindow>));
    value.setProperty("mousePress", value.engine()->newFunction(UI_mousePress<QWindow>));
    value.setProperty("mousePressAtPos", value.engine()->newFunction(UI_mousePressAtPos<QWindow>));
    value.setProperty("mouseRelease", value.engine()->newFunction(UI_mouseRelease<QWindow>));
    value.setProperty("mouseReleaseAtPos", value.engine()->newFunction(UI_mouseReleaseAtPos<QWindow>));
    value.setProperty("mouseMove", value.engine()->newFunction(UI_mouseMove<QWindow>));
    value.setProperty("mouseClick", value.engine()->newFunction(UI_mouseClick<QWindow>));
    value.setProperty("mouseClickAtPos", value.engine()->newFunction(UI_mouseClickAtPos<QWindow>));
    value.setProperty("mouseDClick", value.engine()->newFunction(UI_mouseDClick<QWindow>));
    value.setProperty("mouseDClickAtPos", value.engine()->newFunction(UI_mouseDClickAtPos<QWindow>));
    value.setProperty("keyPress", value.engine()->newFunction(UI_keyPress<QWindow>));
    value.setProperty("keyRelease", value.engine()->newFunction(UI_keyRelease<QWindow>));
    value.setProperty("keyClick", value.engine()->newFunction(UI_keyClick<QWindow>));
    value.setProperty("keyClicks", value.engine()->newFunction(UI_keyClicks<QWindow>));
    value.setProperty("waitForShown", value.engine()->newFunction(UI_waitForShown<QWindow>));
    value.setProperty("wheel", value.engine()->newFunction(UI_wheel<QWindow>));
    value.setProperty("wheelAtPos", value.engine()->newFunction(UI_wheelAtPos<QWindow>));
    value.setProperty("keyboardFocus", value.engine()->newFunction(QWindow_keyboardFocus),
                      QScriptValue::PropertySetter|QScriptValue::PropertyGetter);
    value.setProperty("show", value.engine()->newFunction(UI_show<QWindow>));
    value.setProperty("hide", value.engine()->newFunction(UI_hide<QWindow>));
}
#endif

////////////////////////////////////////////////////////////////////////////////
// QComboBox functions
////////////////////////////////////////////////////////////////////////////////

static QScriptValue QComboBox_textProperty(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QComboBox *comboBox = qobject_cast<QComboBox*>(thisObject.toQObject());
    if(!comboBox)
        return QScriptValue();

    if(context->argumentCount() == 1) // setting current text
    {
        int index = comboBox->findText( context->argument(0).toString() );
        if(index >= 0)
            comboBox->setCurrentIndex(index);
    }

    return QScriptValue(engine, comboBox->currentText());
}

static QScriptValue QComboBox_indexOf(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QComboBox *comboBox = qobject_cast<QComboBox*>(thisObject.toQObject());
    if(!comboBox || context->argumentCount() != 1)
        return QScriptValue(engine, -1);

    int index = comboBox->findText( context->argument(0).toString() );
    return QScriptValue(engine, index);
}

void wrapQComboBox(QScriptValue &value)
{
    value.setProperty("text", value.engine()->newFunction(QComboBox_textProperty),
                      QScriptValue::PropertySetter|QScriptValue::PropertyGetter);
    value.setProperty("indexOf", value.engine()->newFunction(QComboBox_indexOf));
}

////////////////////////////////////////////////////////////////////////////////
// QGraphicsObject functions
////////////////////////////////////////////////////////////////////////////////

static QScriptValue QGraphicsObject_center(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(thisObject.toQObject());
    QScriptValue center = engine->newObject();

    if(!graphicsObject || context->argumentCount())
    {
        center.setProperty("x", -1);
        center.setProperty("y", -1);
        return center;
    }

    QRectF rect = graphicsObject->mapToScene( graphicsObject->boundingRect() ).boundingRect();
    center.setProperty("x", rect.center().x());
    center.setProperty("y", rect.center().y());
    return center;
}

static QScriptValue QGraphicsObject_rect(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(thisObject.toQObject());
    QScriptValue rect = engine->newObject();

    if(!graphicsObject || context->argumentCount())
    {
        rect.setProperty("x", -1);
        rect.setProperty("y", -1);
        rect.setProperty("width", -1);
        rect.setProperty("height", -1);
        return rect;
    }

    QRectF gObjRect = graphicsObject->mapToScene( graphicsObject->boundingRect() ).boundingRect();
    rect.setProperty("x", gObjRect.x());
    rect.setProperty("y", gObjRect.y());
    rect.setProperty("width", gObjRect.width());
    rect.setProperty("height", gObjRect.height());
    return rect;
}

static QScriptValue QGraphicsObject_mousePress(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(thisObject.toQObject());
    if(!graphicsObject || context->argumentCount() != 2)
        return QScriptValue(engine, false);

    bool success = true;
    Qt::MouseButton button = Qt::MouseButton( context->argument(0).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(1).toInt32() );

    QPointF itemCenter = graphicsObject->boundingRect().center();
    itemCenter = graphicsObject->mapToScene( itemCenter );

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMousePress);
    mouseEvent.setScenePos( itemCenter );
    mouseEvent.setButtons( button );
    mouseEvent.setButton( button );
    mouseEvent.setModifiers( modifiers );
    success &= qApp->notify(graphicsObject->scene(), &mouseEvent);

    return QScriptValue(engine, success);
}

static QScriptValue QGraphicsObject_mousePressAtPos(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(thisObject.toQObject());
    if(!graphicsObject || context->argumentCount() != 4)
        return QScriptValue(engine, false);

    bool success = true;
    qreal x = context->argument(0).toNumber();
    qreal y = context->argument(1).toNumber();
    Qt::MouseButton button = Qt::MouseButton( context->argument(2).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(3).toInt32() );

    QPointF pos = graphicsObject->mapToScene( QPointF(x,y) );

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMousePress);
    mouseEvent.setScenePos( pos );
    mouseEvent.setButtons( button );
    mouseEvent.setButton( button );
    mouseEvent.setModifiers( modifiers );
    success &= qApp->notify(graphicsObject->scene(), &mouseEvent);

    return QScriptValue(engine, success);
}

static QScriptValue QGraphicsObject_mouseRelease(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(thisObject.toQObject());
    if(!graphicsObject || context->argumentCount() != 2)
        return QScriptValue(engine, false);

    bool success = true;
    Qt::MouseButton button = Qt::MouseButton( context->argument(0).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(1).toInt32() );

    QPointF itemCenter = graphicsObject->boundingRect().center();
    itemCenter = graphicsObject->mapToScene( itemCenter );

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseRelease);
    mouseEvent.setScenePos( itemCenter );
    mouseEvent.setButtons( button );
    mouseEvent.setButton( button );
    mouseEvent.setModifiers( modifiers );
    success &= qApp->notify(graphicsObject->scene(), &mouseEvent);

    return QScriptValue(engine, success);
}

static QScriptValue QGraphicsObject_mouseReleaseAtPos(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(thisObject.toQObject());
    if(!graphicsObject || context->argumentCount() != 4)
        return QScriptValue(engine, false);

    bool success = true;
    qreal x = context->argument(0).toNumber();
    qreal y = context->argument(1).toNumber();
    Qt::MouseButton button = Qt::MouseButton( context->argument(2).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(3).toInt32() );

    QPointF pos = graphicsObject->mapToScene( QPointF(x,y) );

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseRelease);
    mouseEvent.setScenePos( pos );
    mouseEvent.setButtons( button );
    mouseEvent.setButton( button );
    mouseEvent.setModifiers( modifiers );
    success &= qApp->notify(graphicsObject->scene(), &mouseEvent);

    return QScriptValue(engine, success);
}

static QScriptValue QGraphicsObject_mouseMove(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(thisObject.toQObject());
    if(!graphicsObject || context->argumentCount() != 2)
        return QScriptValue(engine, false);

    bool success = true;
    qreal x = context->argument(0).toNumber();
    qreal y = context->argument(1).toNumber();

    QPointF pos = graphicsObject->mapToScene( QPointF(x,y) );

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
    mouseEvent.setScenePos( pos );
    success &= qApp->notify(graphicsObject->scene(), &mouseEvent);

    return QScriptValue(engine, success);
}

static QScriptValue QGraphicsObject_mouseClick(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(thisObject.toQObject());
    if(!graphicsObject || context->argumentCount() != 2)
        return QScriptValue(engine, false);

    bool success = true;
    Qt::MouseButton button = Qt::MouseButton( context->argument(0).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(1).toInt32() );

    QPointF itemCenter = graphicsObject->boundingRect().center();
    itemCenter = graphicsObject->mapToScene( itemCenter );

    QGraphicsSceneMouseEvent mouseEvent1(QEvent::GraphicsSceneMousePress);
    mouseEvent1.setScenePos( itemCenter );
    mouseEvent1.setButtons( button );
    mouseEvent1.setButton( button );
    mouseEvent1.setModifiers( modifiers );
    success &= qApp->notify(graphicsObject->scene(), &mouseEvent1);

    QTest::qWait(100); // To simulate a proper click

    QGraphicsSceneMouseEvent mouseEvent2(QEvent::GraphicsSceneMouseRelease);
    mouseEvent2.setScenePos( itemCenter );
    mouseEvent2.setButtons( button );
    mouseEvent2.setButton( button );
    mouseEvent2.setModifiers( modifiers );
    success &= qApp->notify(graphicsObject->scene(), &mouseEvent2);

    return QScriptValue(engine, success);
}

static QScriptValue QGraphicsObject_mouseClickAtPos(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(thisObject.toQObject());
    if(!graphicsObject || context->argumentCount() != 4)
        return QScriptValue(engine, false);

    bool success = true;
    qreal x = context->argument(0).toNumber();
    qreal y = context->argument(1).toNumber();
    Qt::MouseButton button = Qt::MouseButton( context->argument(2).toInt32() );
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(3).toInt32() );

    QPointF pos = graphicsObject->mapToScene( QPointF(x,y) );

    QGraphicsSceneMouseEvent mouseEvent1(QEvent::GraphicsSceneMousePress);
    mouseEvent1.setScenePos( pos );
    mouseEvent1.setButtons( button );
    mouseEvent1.setButton( button );
    mouseEvent1.setModifiers( modifiers );
    success &= qApp->notify(graphicsObject->scene(), &mouseEvent1);

    QTest::qWait(100); // To simulate a proper click

    QGraphicsSceneMouseEvent mouseEvent2(QEvent::GraphicsSceneMouseRelease);
    mouseEvent2.setScenePos( pos );
    mouseEvent2.setButtons( button );
    mouseEvent2.setButton( button );
    mouseEvent2.setModifiers( modifiers );
    success &= qApp->notify(graphicsObject->scene(), &mouseEvent2);

    return QScriptValue(engine, success);
}

static QScriptValue QGraphicsObject_wheel(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(thisObject.toQObject());
    if(!graphicsObject || context->argumentCount() != 2)
        return QScriptValue(engine, false);

    qint32 factor = context->argument(0).toInt32();
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers(context->argument(1).toInt32());

    QPointF itemCenter = graphicsObject->boundingRect().center();
    itemCenter = graphicsObject->mapToScene( itemCenter );

    QGraphicsSceneWheelEvent we;
    we.setScenePos(itemCenter);
    we.setModifiers(modifiers);
    we.setDelta(factor);
    bool success = qApp->notify(graphicsObject->scene(), &we);

    return QScriptValue(engine, success);
}

static QScriptValue QGraphicsObject_wheelAtPos(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue thisObject = context->thisObject();
    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(thisObject.toQObject());
    if(!graphicsObject || context->argumentCount() != 2)
        return QScriptValue(engine, false);

    bool success = true;
    qreal x = context->argument(0).toNumber();
    qreal y = context->argument(1).toNumber();
    qint32 factor = context->argument(2).toInt32();
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers( context->argument(3).toInt32() );

    QPointF pos = graphicsObject->mapToScene( QPointF(x,y) );

    QGraphicsSceneWheelEvent we;
    we.setScenePos(pos);
    we.setModifiers(modifiers);
    we.setDelta(factor);
    success = qApp->notify(graphicsObject->scene(), &we);

    return QScriptValue(engine, success);
}

void wrapQGraphicsObject(QScriptValue &value)
{
    value.setProperty("center", value.engine()->newFunction(QGraphicsObject_center),
                      QScriptValue::PropertyGetter);
    value.setProperty("rect", value.engine()->newFunction(QGraphicsObject_rect),
                      QScriptValue::PropertyGetter);
    value.setProperty("mousePress", value.engine()->newFunction(QGraphicsObject_mousePress));
    value.setProperty("mousePressAtPos", value.engine()->newFunction(QGraphicsObject_mousePressAtPos));
    value.setProperty("mouseRelease", value.engine()->newFunction(QGraphicsObject_mouseRelease));
    value.setProperty("mouseReleaseAtPos", value.engine()->newFunction(QGraphicsObject_mouseReleaseAtPos));
    value.setProperty("mouseMove", value.engine()->newFunction(QGraphicsObject_mouseMove));
    value.setProperty("mouseClick", value.engine()->newFunction(QGraphicsObject_mouseClick));
    value.setProperty("mouseClickAtPos", value.engine()->newFunction(QGraphicsObject_mouseClickAtPos));
    value.setProperty("wheel", value.engine()->newFunction(QGraphicsObject_wheel));
    value.setProperty("wheelAtPos", value.engine()->newFunction(QGraphicsObject_wheelAtPos));
}

