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

#include <QString>
#include <QtTest>

#include <GCF3/Version>
#include <GCF3/GuiApplication>
#include <GCF3/GuiComponent>

#include <QMenu>
#include <QLabel>
#include <QWidget>
#include <QToolBar>
#include <QMenuBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QWidgetAction>
#include <QStackedWidget>

#include "TestGuiComponent.h"

class GuiComponentTest : public QObject
{
    Q_OBJECT
    
public:
    GuiComponentTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testLoadUnloadWidget();
    void testLoadUnloadAction();
    void testLoadUnloadActionGroup();
    void testLoadUnloadMenu();
    void testLoadUnloadMenuBar();
    void testLoadUnloadToolBar();

    void testMergeUnmergeWithObject();
    void testMergeUnmergeWithWidget();
    void testMergeUnmergeWithActionGroup();
    void testMergeUnmergeWithMenu();
    void testMergeUnmergeWithToolBar();
    void testMergeUnmergeWithMenuBar();

    void testActivateDeactivateObject();
    void testActivateDeactivateWidget();
};

GuiComponentTest::GuiComponentTest()
{
}

void GuiComponentTest::initTestCase()
{
    qDebug("Running tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void GuiComponentTest::cleanupTestCase()
{
    qDebug("Executed tests on GCF-%s built on %s",
           qPrintable(GCF::version()),
           qPrintable(GCF::buildTimestamp()));
}

void GuiComponentTest::cleanup()
{
    QFile::remove( GCF::Log::instance()->logFileName() );
}

void GuiComponentTest::testLoadUnloadWidget()
{
    QList<LoadEvent> loadEvents;
    QList<UnloadEvent> unloadEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestLoadUnloadWidget.xml";
    comp->LoadWidgetEvents = &loadEvents;
    comp->UnloadWidgetEvents = &unloadEvents;
    comp->load();

    QVERIFY(loadEvents.count() == 6);
    QVERIFY(unloadEvents.count() == 0);

    QStringList expectedNames = QStringList() << "Window" << "Table" << "GroupBox"
                                              << "StackedWidget" << "TabWidget" << "GraphicsView";
    QStringList expectedClasses = QStringList() << "QMainWindow" << "QTableWidget"
                                                << "QGroupBox" << "QStackedWidget"
                                                << "QTabWidget" << "QGraphicsView"; /* QGraphicsView is not recognized */
    QObjectList objects;

    for(int i=0; i<6; i++)
    {
        LoadEvent evt = loadEvents.at(i);

        QVERIFY(evt.Name == expectedNames.at(i));
        QVERIFY(evt.Info.value("type").toString() == "widget");
        QVERIFY(evt.Info.value("class").toString() == expectedClasses.at(i));
        QVERIFY(evt.Object != 0);

        QString className = QString::fromLatin1(evt.Object->metaObject()->className());
        QVERIFY(className == expectedClasses.at(i) || className == "QWidget");

        objects.append(evt.Object);
    }

    loadEvents.clear();
    unloadEvents.clear();
    comp->unload();

    QVERIFY(loadEvents.count() == 0);
    QVERIFY(unloadEvents.count() == 6);

    for(int i=0; i<6; i++)
    {
        LoadEvent evt = unloadEvents.at(i);

        QVERIFY(evt.Name == expectedNames.at(5-i));
        QVERIFY(evt.Info.value("type").toString() == "widget");
        QVERIFY(evt.Info.value("class").toString() == expectedClasses.at(5-i));
        QVERIFY(evt.Object == objects.at(5-i));
    }
}

void GuiComponentTest::testLoadUnloadAction()
{
    QList<LoadEvent> loadEvents;
    QList<UnloadEvent> unloadEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestLoadUnloadAction.xml";
    comp->LoadActionEvents = &loadEvents;
    comp->UnloadActionEvents = &unloadEvents;
    comp->load();

    QVERIFY(loadEvents.count() == 9);
    QVERIFY(unloadEvents.count() == 0);

    QObjectList objects;
    for(int i=0; i<9; i++)
    {
        LoadEvent evt = loadEvents.at(i);
        QVERIFY(evt.Name == QString("Action %1").arg(i+1));
        QVERIFY(evt.Info.value("type").toString() == "action");
        QVERIFY(QString::fromLatin1(evt.Object->metaObject()->className()) == "QAction");
        objects.append(evt.Object);
    }

    loadEvents.clear();
    unloadEvents.clear();
    comp->unload();

    QVERIFY(loadEvents.count() == 0);
    QVERIFY(unloadEvents.count() == 9);

    for(int i=0; i<9; i++)
    {
        UnloadEvent evt = unloadEvents.at(i);
        QVERIFY(evt.Name == QString("Action %1").arg(9-i));
        QVERIFY(evt.Info.value("type").toString() == "action");
        QVERIFY(evt.Object == objects.at(8-i));
    }
}

void GuiComponentTest::testLoadUnloadActionGroup()
{
    QList<LoadEvent> loadEvents;
    QList<UnloadEvent> unloadEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestLoadUnloadActionGroup.xml";
    comp->LoadActionGroupEvents = &loadEvents;
    comp->UnloadActionGroupEvents = &unloadEvents;
    comp->load();

    QVERIFY(loadEvents.count() == 5);
    QVERIFY(unloadEvents.count() == 0);

    QObjectList objects;
    for(int i=0; i<5; i++)
    {
        LoadEvent evt = loadEvents.at(i);
        QVERIFY(evt.Name == QString("ActionGroup %1").arg(i+1));
        QVERIFY(evt.Info.value("type").toString() == "actiongroup");
        QVERIFY(QString::fromLatin1(evt.Object->metaObject()->className()) == "QActionGroup");
        objects.append(evt.Object);
    }

    loadEvents.clear();
    unloadEvents.clear();
    comp->unload();

    QVERIFY(loadEvents.count() == 0);
    QVERIFY(unloadEvents.count() == 5);

    for(int i=0; i<5; i++)
    {
        UnloadEvent evt = unloadEvents.at(i);
        QVERIFY(evt.Name == QString("ActionGroup %1").arg(5-i));
        QVERIFY(evt.Info.value("type").toString() == "actiongroup");
        QVERIFY(evt.Object == objects.at(4-i));
    }
}

void GuiComponentTest::testLoadUnloadMenu()
{
    QList<LoadEvent> loadEvents;
    QList<UnloadEvent> unloadEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestLoadUnloadMenu.xml";
    comp->LoadMenuEvents = &loadEvents;
    comp->UnloadMenuEvents = &unloadEvents;
    comp->load();

    QVERIFY(loadEvents.count() == 4);
    QVERIFY(unloadEvents.count() == 0);

    QObjectList objects;
    for(int i=0; i<4; i++)
    {
        LoadEvent evt = loadEvents.at(i);
        QVERIFY(evt.Name == QString("Menu %1").arg(i+1));
        QVERIFY(evt.Info.value("type").toString() == "menu");
        QVERIFY(QString::fromLatin1(evt.Object->metaObject()->className()) == "QMenu");
        objects.append(evt.Object);
    }

    loadEvents.clear();
    unloadEvents.clear();
    comp->unload();

    QVERIFY(loadEvents.count() == 0);
    QVERIFY(unloadEvents.count() == 4);

    for(int i=0; i<4; i++)
    {
        UnloadEvent evt = unloadEvents.at(i);
        QVERIFY(evt.Name == QString("Menu %1").arg(4-i));
        QVERIFY(evt.Info.value("type").toString() == "menu");
        QVERIFY(evt.Object == objects.at(3-i));
    }
}

void GuiComponentTest::testLoadUnloadMenuBar()
{
    QList<LoadEvent> loadEvents;
    QList<UnloadEvent> unloadEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestLoadUnloadMenuBar.xml";
    comp->LoadMenuBarEvents = &loadEvents;
    comp->UnloadMenuBarEvents = &unloadEvents;
    comp->load();

    QVERIFY(loadEvents.count() == 3);
    QVERIFY(unloadEvents.count() == 0);

    QObjectList objects;
    for(int i=0; i<3; i++)
    {
        LoadEvent evt = loadEvents.at(i);
        QVERIFY(evt.Name == QString("MenuBar %1").arg(i+1));
        QVERIFY(evt.Info.value("type").toString() == "menubar");
        QVERIFY(QString::fromLatin1(evt.Object->metaObject()->className()) == "QMenuBar");
        objects.append(evt.Object);
    }

    loadEvents.clear();
    unloadEvents.clear();
    comp->unload();

    QVERIFY(loadEvents.count() == 0);
    QVERIFY(unloadEvents.count() == 3);

    for(int i=0; i<3; i++)
    {
        UnloadEvent evt = unloadEvents.at(i);
        QVERIFY(evt.Name == QString("MenuBar %1").arg(3-i));
        QVERIFY(evt.Info.value("type").toString() == "menubar");
        QVERIFY(evt.Object == objects.at(2-i));
    }
}

void GuiComponentTest::testLoadUnloadToolBar()
{
    QList<LoadEvent> loadEvents;
    QList<UnloadEvent> unloadEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestLoadUnloadToolBar.xml";
    comp->LoadToolBarEvents = &loadEvents;
    comp->UnloadToolBarEvents = &unloadEvents;
    comp->load();

    QVERIFY(loadEvents.count() == 3);
    QVERIFY(unloadEvents.count() == 0);

    QObjectList objects;
    for(int i=0; i<3; i++)
    {
        LoadEvent evt = loadEvents.at(i);
        QVERIFY(evt.Name == QString("ToolBar %1").arg(i+1));
        QVERIFY(evt.Info.value("type").toString() == "toolbar");
        QVERIFY(QString::fromLatin1(evt.Object->metaObject()->className()) == "QToolBar");
        objects.append(evt.Object);
    }

    loadEvents.clear();
    unloadEvents.clear();
    comp->unload();

    QVERIFY(loadEvents.count() == 0);
    QVERIFY(unloadEvents.count() == 3);

    for(int i=0; i<3; i++)
    {
        UnloadEvent evt = unloadEvents.at(i);
        QVERIFY(evt.Name == QString("ToolBar %1").arg(3-i));
        QVERIFY(evt.Info.value("type").toString() == "toolbar");
        QVERIFY(evt.Object == objects.at(2-i));
    }
}

void GuiComponentTest::testMergeUnmergeWithObject()
{
    QList<MergeEvent> mergeEvents;
    QList<UnmergeEvent> unmergeEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestMergeLayoutWithWidget.xml";
    comp->MergeEvents = &mergeEvents;
    comp->UnmergeEvents = &unmergeEvents;
    comp->load();

    QVERIFY(mergeEvents.count() == 4);
    QVERIFY(unmergeEvents.count() == 0);

    QVERIFY(gFindObject<QVBoxLayout>("Application.TestGuiComponent.Layout1"));
    QVERIFY(gFindObject<QHBoxLayout>("Application.TestGuiComponent.Layout2"));
    QVERIFY(gFindObject<QGridLayout>("Application.TestGuiComponent.Layout3"));
    QVERIFY(gFindObject<QFormLayout>("Application.TestGuiComponent.Layout4"));

    QList< QPair<QObject*,QObject*> > widgetLayoutPairs;

    for(int i=0; i<4; i++)
    {
        MergeEvent evt = mergeEvents.at(i);

        QString path = QString("Application.TestGuiComponent.Window%1").arg(i+1);
        QWidget *widget = gFindObject<QWidget>(path);

        QVERIFY(evt.Parent == widget);
        QVERIFY(evt.Child = widget->layout());

        switch(i)
        {
        case 0: QVERIFY( qobject_cast<QVBoxLayout*>(evt.Child) ); break;
        case 1: QVERIFY( qobject_cast<QHBoxLayout*>(evt.Child) ); break;
        case 2: QVERIFY( qobject_cast<QGridLayout*>(evt.Child) ); break;
        case 3: QVERIFY( qobject_cast<QFormLayout*>(evt.Child) ); break;
        default: break;
        }

        widgetLayoutPairs.append( qMakePair((QObject*)widget,(QObject*)(widget->layout())) );
    }

    QWidget *window5 = gFindObject<QWidget>("Application.TestGuiComponent.Window5");
    QVERIFY(window5 != 0);
    QVERIFY( qobject_cast<QVBoxLayout*>(window5->layout()) );

    QWidget *window6 = gFindObject<QWidget>("Application.TestGuiComponent.Window6");
    QVERIFY(window6 != 0);
    QVERIFY( qobject_cast<QHBoxLayout*>(window6->layout()) );

    QWidget *window7 = gFindObject<QWidget>("Application.TestGuiComponent.Window7");
    QVERIFY(window7 != 0);
    QVERIFY( qobject_cast<QGridLayout*>(window7->layout()) );

    QWidget *window8 = gFindObject<QWidget>("Application.TestGuiComponent.Window8");
    QVERIFY(window8 != 0);
    QVERIFY( qobject_cast<QFormLayout*>(window8->layout()) );

    mergeEvents.clear();
    unmergeEvents.clear();
    comp->unload();

    QVERIFY(unmergeEvents.count() == 4);
    for(int i=0; i<4; i++)
    {
        UnmergeEvent evt = unmergeEvents.at(i);
        QVERIFY(evt.Parent == widgetLayoutPairs.at(3-i).first);
        QVERIFY(evt.Child == widgetLayoutPairs.at(3-i).second);
    }
}

void GuiComponentTest::testMergeUnmergeWithWidget()
{
    QList<MergeEvent> mergeEvents;
    QList<UnmergeEvent> unmergeEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestMergeUnmergeWithWidget.xml";
    comp->MergeWidgetEvents = &mergeEvents;
    comp->UnmergeWidgetEvents = &unmergeEvents;
    comp->load();

    QVERIFY(mergeEvents.count() == 18);
    QVERIFY(unmergeEvents.count() == 0);

    QList< QPair<QObject*,QObject*> > widgetWidgetPairs;

    for(int i=0; i<5; i++)
    {
        MergeEvent evt = mergeEvents.at(i);

        QString path = QString("Application.TestGuiComponent.Window%1").arg(i+1);
        QString menuBarPath = QString("Application.TestGuiComponent.MenuBar%1").arg(i+1);
        QWidget *widget = gFindObject<QWidget>(path);
        QWidget *menuBar = gFindObject<QWidget>(menuBarPath);

        QVERIFY(evt.Parent == widget);
        QVERIFY(evt.Child == widget->layout()->menuBar());
        QVERIFY(menuBar == widget->layout()->menuBar());

        switch(i)
        {
        case 0: QVERIFY( qobject_cast<QVBoxLayout*>(widget->layout()) ); break;
        case 1: QVERIFY( qobject_cast<QHBoxLayout*>(widget->layout()) ); break;
        case 2: QVERIFY( qobject_cast<QGridLayout*>(widget->layout()) ); break;
        case 3: QVERIFY( qobject_cast<QFormLayout*>(widget->layout()) ); break;
        case 4: QVERIFY( qobject_cast<QVBoxLayout*>(widget->layout()) ); break;
        default: break;
        }

        widgetWidgetPairs.append( qMakePair((QObject*)widget,
                                            (QObject*)(menuBar)) );
    }

    QTabWidget *tabWidget1 = gFindObject<QTabWidget>("Application.TestGuiComponent.Window6");
    QVERIFY( tabWidget1 != 0 );
    QWidget *widget1 = gFindObject<QWidget>("Application.TestGuiComponent.ChildWidget1");
    QVERIFY( widget1 != 0 );
    QVERIFY( widget1 == tabWidget1->widget(0) );
    MergeEvent evt = mergeEvents.at(5);
    int index = tabWidget1->indexOf((QWidget*)evt.Child);
    QVERIFY( tabWidget1->tabText(index) == "TabWidget");
    widgetWidgetPairs.append( qMakePair((QObject*)tabWidget1,
                                        (QObject*)(widget1)) );

    QTabWidget *tabWidget2 = gFindObject<QTabWidget>("Application.TestGuiComponent.Window7");
    QVERIFY( tabWidget2 != 0 );
    QWidget *widget2 = gFindObject<QWidget>("Application.TestGuiComponent.ChildWidget2");
    QVERIFY( widget2 != 0 );
    QVERIFY( widget2 == tabWidget2->widget(0) );
    evt = mergeEvents.at(6);
    index = tabWidget2->indexOf((QWidget*)evt.Child);
    QVERIFY( tabWidget2->tabText(index) == "WindowTitle");
    widgetWidgetPairs.append( qMakePair((QObject*)tabWidget2,
                                        (QObject*)(widget2)) );

    QTabWidget *tabWidget3 = gFindObject<QTabWidget>("Application.TestGuiComponent.Window8");
    QVERIFY( tabWidget3 != 0 );
    QWidget *widget3 = gFindObject<QWidget>("Application.TestGuiComponent.ChildWidget3");
    QVERIFY( widget3 != 0 );
    QVERIFY( widget3 == tabWidget3->widget(0) );
    evt = mergeEvents.at(7);
    index = tabWidget3->indexOf((QWidget*)evt.Child);
    QVERIFY( tabWidget3->tabText(index) == "Tab 1");
    widgetWidgetPairs.append( qMakePair((QObject*)tabWidget3,
                                        (QObject*)(widget3)) );

    QStackedWidget *stackedWidget = gFindObject<QStackedWidget>("Application.TestGuiComponent.Window9");
    QVERIFY( stackedWidget != 0 );
    QWidget *page = gFindObject<QWidget>("Application.TestGuiComponent.Page");
    QVERIFY( page != 0 );
    QVERIFY( page == stackedWidget->widget(0) );
    evt = mergeEvents.at(8);
    index = tabWidget3->indexOf((QWidget*)evt.Child);
    widgetWidgetPairs.append( qMakePair((QObject*)stackedWidget,
                                        (QObject*)(page)) );

    QWidget *layoutParentWidget1 = gFindObject<QWidget>("Application.TestGuiComponent.Window10");
    QVERIFY( layoutParentWidget1 != 0 );
    QWidget *layoutChildWidget1 = gFindObject<QWidget>("Application.TestGuiComponent.ChildWidget10-1");
    QVERIFY( layoutChildWidget1 != 0 );
    QWidget *layoutChildWidget2 = gFindObject<QWidget>("Application.TestGuiComponent.ChildWidget10-2");
    QVERIFY( layoutChildWidget2 != 0 );
    QVERIFY( layoutParentWidget1->layout()->itemAt(0)->widget() == layoutChildWidget2);
    QVERIFY( layoutParentWidget1->layout()->itemAt(1)->widget() == layoutChildWidget1);
    widgetWidgetPairs.append( qMakePair((QObject*)layoutParentWidget1,
                                        (QObject*)(layoutChildWidget1)) );
    widgetWidgetPairs.append( qMakePair((QObject*)layoutParentWidget1,
                                        (QObject*)(layoutChildWidget2)) );

    QWidget *gridLayoutParentWidget1 = gFindObject<QWidget>("Application.TestGuiComponent.Window11");
    QVERIFY( gridLayoutParentWidget1 != 0 );
    QWidget *gridLayoutChildWidget1 = gFindObject<QWidget>("Application.TestGuiComponent.ChildWidget11-1");
    QVERIFY( gridLayoutChildWidget1 != 0 );
    QWidget *gridLayoutChildWidget2 = gFindObject<QWidget>("Application.TestGuiComponent.ChildWidget11-2");
    QVERIFY( gridLayoutChildWidget2 != 0 );
    QGridLayout *gridLayout = qobject_cast<QGridLayout*>(gridLayoutParentWidget1->layout());
    QVERIFY( gridLayout != 0);
    QVERIFY( gridLayout->itemAtPosition(0, 0)->widget() == gridLayoutChildWidget2);
    QVERIFY( gridLayout->itemAtPosition(1, 1)->widget() == gridLayoutChildWidget1);
    widgetWidgetPairs.append( qMakePair((QObject*)gridLayoutParentWidget1,
                                        (QObject*)(gridLayoutChildWidget2)) );
    widgetWidgetPairs.append( qMakePair((QObject*)gridLayoutParentWidget1,
                                        (QObject*)(gridLayoutChildWidget1)) );

    QWidget *formLayoutParentWidget1 = gFindObject<QWidget>("Application.TestGuiComponent.Window12");
    QVERIFY( formLayoutParentWidget1 != 0 );
    QWidget *formLayoutChildWidget1 = gFindObject<QWidget>("Application.TestGuiComponent.ChildWidget12-1");
    QVERIFY( formLayoutChildWidget1 != 0 );
    QFormLayout *formLayout1 = qobject_cast<QFormLayout*>(formLayoutParentWidget1->layout());
    QVERIFY( formLayout1 != 0);
    QVERIFY( formLayout1->itemAt(1)->widget() == formLayoutChildWidget1);
    QVERIFY( formLayout1->labelForField(formLayoutChildWidget1) != 0);
    QVERIFY( (qobject_cast<QLabel*>(formLayout1->labelForField(formLayoutChildWidget1)))->text() == "FormChild");
    widgetWidgetPairs.append( qMakePair((QObject*)formLayoutParentWidget1,
                                        (QObject*)(formLayoutChildWidget1)) );

    QWidget *formLayoutParentWidget2 = gFindObject<QWidget>("Application.TestGuiComponent.Window13");
    QVERIFY( formLayoutParentWidget2 != 0 );
    QWidget *formLayoutChildWidget2 = gFindObject<QWidget>("Application.TestGuiComponent.ChildWidget13-1");
    QVERIFY( formLayoutChildWidget2 != 0 );
    QFormLayout *formLayout2 = qobject_cast<QFormLayout*>(formLayoutParentWidget2->layout());
    QVERIFY( formLayout2 != 0);
    QVERIFY( formLayout2->itemAt(1)->widget() == formLayoutChildWidget2);
    QVERIFY( formLayout2->labelForField(formLayoutChildWidget2) != 0);
    QVERIFY( (qobject_cast<QLabel*>(formLayout2->labelForField(formLayoutChildWidget2)))->text() == "Form13-Child");
    widgetWidgetPairs.append( qMakePair((QObject*)formLayoutParentWidget2,
                                        (QObject*)(formLayoutChildWidget2)) );

    QWidget *formLayoutParentWidget3 = gFindObject<QWidget>("Application.TestGuiComponent.Window14");
    QVERIFY( formLayoutParentWidget3 != 0 );
    QWidget *formLayoutChildWidget3 = gFindObject<QWidget>("Application.TestGuiComponent.ChildWidget14-1");
    QVERIFY( formLayoutChildWidget3 != 0 );
    QFormLayout *formLayout3 = qobject_cast<QFormLayout*>(formLayoutParentWidget3->layout());
    QVERIFY( formLayout3 != 0);
    QVERIFY( formLayout3->itemAt(1)->widget() == formLayoutChildWidget3);
    QVERIFY( formLayout3->labelForField(formLayoutChildWidget3) != 0);
    QVERIFY( (qobject_cast<QLabel*>(formLayout3->labelForField(formLayoutChildWidget3)))->text() == "Form14-Child");
    widgetWidgetPairs.append( qMakePair((QObject*)formLayoutParentWidget3,
                                        (QObject*)(formLayoutChildWidget3)) );

    QWidget *formLayoutParentWidget4 = gFindObject<QWidget>("Application.TestGuiComponent.Window15");
    QVERIFY( formLayoutParentWidget4 != 0 );
    QWidget *formLayoutChildWidget4 = gFindObject<QWidget>("Application.TestGuiComponent.ChildWidget15-1");
    QVERIFY( formLayoutChildWidget4 != 0 );
    QFormLayout *formLayout4 = qobject_cast<QFormLayout*>(formLayoutParentWidget4->layout());
    QVERIFY( formLayout4 != 0);
    QVERIFY( formLayout4->itemAt(1)->widget() == formLayoutChildWidget4);
    QVERIFY( formLayout4->labelForField(formLayoutChildWidget4) != 0);
    QVERIFY( (qobject_cast<QLabel*>(formLayout4->labelForField(formLayoutChildWidget4)))->text() == "untitled");
    widgetWidgetPairs.append( qMakePair((QObject*)formLayoutParentWidget4,
                                        (QObject*)(formLayoutChildWidget4)) );

    QWidget *window16 = gFindObject<QWidget>("Application.TestGuiComponent.Window16");
    QWidget *widget16 = gFindObject<QWidget>("Application.TestGuiComponent.Widget16");
    QVERIFY(window16 != 0);
    QVERIFY(widget16 != 0);
    QVERIFY(widget16->parent() == window16);
    QVERIFY(window16->layout() != 0);
    QVERIFY(qobject_cast<QVBoxLayout*>(window16->layout()) != 0);
    QVERIFY(window16->layout()->itemAt(0)->widget() == widget16);
    QVERIFY(mergeEvents.last().Parent == window16);
    QVERIFY(mergeEvents.last().Child == widget16);

    widgetWidgetPairs.append( qMakePair((QObject*)window16,
                                        (QObject*)(widget16)) );

    mergeEvents.clear();
    unmergeEvents.clear();
    comp->unload();

    QVERIFY(mergeEvents.count() == 0);
    QVERIFY(unmergeEvents.count() == 18);
    for(int i=0; i<17; i++)
    {
        UnmergeEvent evt = unmergeEvents.at(i);
        QVERIFY(evt.Parent == widgetWidgetPairs.at(17-i).first);
        QVERIFY(evt.Child == widgetWidgetPairs.at(17-i).second);
    }
}

void GuiComponentTest::testMergeUnmergeWithActionGroup()
{
    QList<MergeEvent> mergeEvents;
    QList<UnmergeEvent> unmergeEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestMergeUnmergeWithActionGroup.xml";
    comp->MergeWithActionGroupEvents = &mergeEvents;
    comp->UnmergeFromActionGroupEvents = &unmergeEvents;
    comp->load();

    QVERIFY(mergeEvents.count() == 1);
    QVERIFY(unmergeEvents.count() == 0);

    QList< QPair<QObject*,QObject*> > actionGroupActionPairs;

    for(int i=0; i<mergeEvents.count(); i++)
    {
        MergeEvent evt = mergeEvents.at(i);

        QVERIFY(evt.Parent == gFindObject<QActionGroup>( QString("Application.TestGuiComponent.ActionGroup%1").arg(i+1)) );
        QVERIFY(evt.Child == gFindObject<QAction>( QString("Application.TestGuiComponent.Action%1").arg(i+1)) );

        QActionGroup *actionGroup = qobject_cast<QActionGroup*>(evt.Parent);
        QVERIFY(actionGroup != 0);

        QAction *action = qobject_cast<QAction*>(evt.Child);
        QVERIFY(action != 0);

        QVERIFY(actionGroup->actions().contains(action));
        actionGroupActionPairs.append(qMakePair(qobject_cast<QObject*>(actionGroup),
                                                qobject_cast<QObject*>(action)));
    }

    mergeEvents.clear();
    unmergeEvents.clear();
    comp->unload();

    QVERIFY(mergeEvents.count() == 0);
    QVERIFY(unmergeEvents.count() == 1);
    for(int i=0; i<unmergeEvents.count(); i++)
    {
        UnmergeEvent evt = unmergeEvents.at(i);
        QVERIFY(evt.Parent == actionGroupActionPairs.at(unmergeEvents.count()-1-i).first);
        QVERIFY(evt.Child == actionGroupActionPairs.at(unmergeEvents.count()-1-i).second);
    }
}

void GuiComponentTest::testMergeUnmergeWithMenu()
{
    QList<MergeEvent> mergeEvents;
    QList<UnmergeEvent> unmergeEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestMergeUnmergeWithMenu.xml";
    comp->MergeWithMenuEvents = &mergeEvents;
    comp->UnmergeFromMenuEvents = &unmergeEvents;
    comp->load();

    QVERIFY(mergeEvents.count() == 3);
    QVERIFY(unmergeEvents.count() == 0);

    QList< QPair<QObject*,QObject*> > menuObjectPairs;

    MergeEvent evt = mergeEvents.at(0);
    QVERIFY(evt.Parent == gFindObject<QMenu>("Application.TestGuiComponent.Menu1"));
    QVERIFY(evt.Child == gFindObject<QAction>("Application.TestGuiComponent.Action1"));
    QMenu *menu = qobject_cast<QMenu*>(evt.Parent);
    QVERIFY(menu != 0);
    QAction *action = qobject_cast<QAction*>(evt.Child);
    QVERIFY(action != 0);
    QVERIFY(menu->actions().first() == action);
    menuObjectPairs.append(qMakePair(qobject_cast<QObject*>(menu),
                                            qobject_cast<QObject*>(action)));

    evt = mergeEvents.at(1);
    QVERIFY(evt.Parent == gFindObject<QMenu>("Application.TestGuiComponent.Menu1"));
    QVERIFY(evt.Child == gFindObject<QMenu>("Application.TestGuiComponent.ChildMenu1"));
    menu = qobject_cast<QMenu*>(evt.Parent);
    QVERIFY(menu != 0);
    QMenu *childMenu = qobject_cast<QMenu*>(evt.Child);
    QVERIFY(childMenu != 0);
    QVERIFY(menu->actions().at(1) == childMenu->menuAction());
    menuObjectPairs.append(qMakePair(qobject_cast<QObject*>(menu),
                                            qobject_cast<QObject*>(childMenu)));

    evt = mergeEvents.at(2);
    QVERIFY(evt.Parent == gFindObject<QMenu>("Application.TestGuiComponent.Menu1"));
    QVERIFY(evt.Child == gFindObject<QWidget>("Application.TestGuiComponent.Widget1"));
    menu = qobject_cast<QMenu*>(evt.Parent);
    QVERIFY(menu != 0);
    QWidget *childWidget = qobject_cast<QWidget*>(evt.Child);
    QVERIFY(childWidget != 0);
    QWidgetAction *widgetAction = qobject_cast<QWidgetAction*>(menu->actions().at(2));
    QVERIFY(childWidget == widgetAction->defaultWidget());
    menuObjectPairs.append(qMakePair(qobject_cast<QObject*>(menu),
                                            qobject_cast<QObject*>(childWidget)));

    mergeEvents.clear();
    unmergeEvents.clear();
    comp->unload();

    QVERIFY(mergeEvents.count() == 0);
    QVERIFY(unmergeEvents.count() == 3);
    for(int i=0; i<unmergeEvents.count(); i++)
    {
        UnmergeEvent evt = unmergeEvents.at(i);
        QVERIFY(evt.Parent == menuObjectPairs.at(unmergeEvents.count()-1-i).first);
        QVERIFY(evt.Child == menuObjectPairs.at(unmergeEvents.count()-1-i).second);
    }
}

void GuiComponentTest::testMergeUnmergeWithToolBar()
{
    QList<MergeEvent> mergeEvents;
    QList<UnmergeEvent> unmergeEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestMergeUnmergeWithToolBar.xml";
    comp->MergeWithToolBarEvents = &mergeEvents;
    comp->UnmergeFromToolBarEvents = &unmergeEvents;
    comp->load();

    QVERIFY(mergeEvents.count() == 2);
    QVERIFY(unmergeEvents.count() == 0);

    QList< QPair<QObject*,QObject*> > toolBarObjectPairs;

    MergeEvent evt = mergeEvents.at(0);
    QVERIFY(evt.Parent == gFindObject<QToolBar>("Application.TestGuiComponent.ToolBar1"));
    QVERIFY(evt.Child == gFindObject<QAction>("Application.TestGuiComponent.Action1"));
    QToolBar *toolBar = qobject_cast<QToolBar*>(evt.Parent);
    QVERIFY(toolBar != 0);
    QAction *action = qobject_cast<QAction*>(evt.Child);
    QVERIFY(action != 0);
    QVERIFY(toolBar->actions().first() == action);
    toolBarObjectPairs.append(qMakePair(qobject_cast<QObject*>(toolBar),
                                            qobject_cast<QObject*>(action)));

    evt = mergeEvents.at(1);
    QVERIFY(evt.Parent == gFindObject<QToolBar>("Application.TestGuiComponent.ToolBar1"));
    QVERIFY(evt.Child == gFindObject<QMenu>("Application.TestGuiComponent.ChildMenu1"));
    toolBar = qobject_cast<QToolBar*>(evt.Parent);
    QVERIFY(toolBar != 0);
    QMenu *childMenu = qobject_cast<QMenu*>(evt.Child);
    QVERIFY(childMenu != 0);
    QVERIFY(toolBar->actions().at(1) == childMenu->menuAction());
    toolBarObjectPairs.append(qMakePair(qobject_cast<QObject*>(toolBar),
                                            qobject_cast<QObject*>(childMenu)));

    mergeEvents.clear();
    unmergeEvents.clear();
    comp->unload();

    QVERIFY(mergeEvents.count() == 0);
    QVERIFY(unmergeEvents.count() == 2);
    for(int i=0; i<unmergeEvents.count(); i++)
    {
        UnmergeEvent evt = unmergeEvents.at(i);
        QVERIFY(evt.Parent == toolBarObjectPairs.at(unmergeEvents.count()-1-i).first);
        QVERIFY(evt.Child == toolBarObjectPairs.at(unmergeEvents.count()-1-i).second);
    }
}

void GuiComponentTest::testMergeUnmergeWithMenuBar()
{
    QList<MergeEvent> mergeEvents;
    QList<UnmergeEvent> unmergeEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestMergeUnmergeWithMenuBar.xml";
    comp->MergeWithMenuBarEvents = &mergeEvents;
    comp->UnmergeFromMenuBarEvents = &unmergeEvents;
    comp->load();

    QVERIFY(mergeEvents.count() == 1);
    QVERIFY(unmergeEvents.count() == 0);

    QList< QPair<QObject*,QObject*> > menuBarObjectPairs;

    MergeEvent evt = mergeEvents.at(0);
    QVERIFY(evt.Parent == gFindObject<QMenuBar>("Application.TestGuiComponent.MenuBar1"));
    QVERIFY(evt.Child == gFindObject<QMenu>("Application.TestGuiComponent.ChildMenu1"));
    QMenuBar *menuBar = qobject_cast<QMenuBar*>(evt.Parent);
    QVERIFY(menuBar != 0);
    QMenu *childMenu = qobject_cast<QMenu*>(evt.Child);
    QVERIFY(childMenu != 0);
    QVERIFY(menuBar->actions().first() == childMenu->menuAction());
    menuBarObjectPairs.append(qMakePair(qobject_cast<QObject*>(menuBar),
                                            qobject_cast<QObject*>(childMenu)));

    mergeEvents.clear();
    unmergeEvents.clear();
    comp->unload();

    QVERIFY(mergeEvents.count() == 0);
    QVERIFY(unmergeEvents.count() == 1);
    for(int i=0; i<unmergeEvents.count(); i++)
    {
        UnmergeEvent evt = unmergeEvents.at(i);
        QVERIFY(evt.Parent == menuBarObjectPairs.at(unmergeEvents.count()-1-i).first);
        QVERIFY(evt.Child == menuBarObjectPairs.at(unmergeEvents.count()-1-i).second);
    }
}

void GuiComponentTest::testActivateDeactivateObject()
{
    QList<MergeEvent> activateEvents;
    QList<UnmergeEvent> deactivateEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestActivateDeactivateObject.xml";
    comp->ActivateObjectEvents = &activateEvents;
    comp->DeactivateObjectEvents = &deactivateEvents;
    comp->load();

    QVERIFY(activateEvents.count() == 2);
    QVERIFY(deactivateEvents.count() == 0);

    QList< QPair<QObject*,QObject*> > objectPairs;

    for(int i=0; i<activateEvents.count(); i++)
    {
        MergeEvent evt = activateEvents.at(i);
        QVERIFY(evt.Parent == gFindObject<QMenuBar>("Application.TestGuiComponent.MenuBar1"));
        QString actionName = QString("Application.TestGuiComponent.Action%1").arg(i+1);
        QAction *childAction = gFindObject<QAction>(actionName);
        QVERIFY(childAction != 0);
        QVERIFY(evt.Child == qobject_cast<QObject*>(childAction));

        QMenuBar *parentObject = qobject_cast<QMenuBar*>(evt.Parent);
        QVERIFY(parentObject != 0);
        QAction *action = qobject_cast<QAction*>(evt.Child);
        QVERIFY(action != 0);
        QVERIFY(action->isEnabled());
        objectPairs.append(qMakePair(qobject_cast<QObject*>(parentObject),
                                                qobject_cast<QObject*>(childAction)));
    }

    activateEvents.clear();
    deactivateEvents.clear();

    comp->deactivate();

    QVERIFY(activateEvents.count() == 0);
    QVERIFY(deactivateEvents.count() == 2);
    for(int i=0; i<deactivateEvents.count(); i++)
    {
        UnmergeEvent evt = deactivateEvents.at(i);
        QVERIFY(evt.Parent == objectPairs.at(i).first);

        QVERIFY(evt.Child == qobject_cast<QObject*>(
                    objectPairs.at(i).second));
        QAction *action = qobject_cast<QAction*>(evt.Child);
        QVERIFY(action != 0);
        QVERIFY(!action->isEnabled());
    }

    comp->unload();
}

void GuiComponentTest::testActivateDeactivateWidget()
{
    QList<MergeEvent> activateEvents;
    QList<UnmergeEvent> deactivateEvents;

    TestGuiComponent *comp = new TestGuiComponent;
    comp->ContentFile = ":/TestActivateDeactivateWidget.xml";
    comp->ActivateWidgetEvents = &activateEvents;
    comp->DeactivateWidgetEvents = &deactivateEvents;
    comp->load();

    QVERIFY(activateEvents.count() == 2);
    QVERIFY(deactivateEvents.count() == 0);

    QList< QPair<QObject*,QObject*> > widgetPairs;

    for(int i=0; i<activateEvents.count(); i++)
    {
        MergeEvent evt = activateEvents.at(i);
        QVERIFY(evt.Parent == gFindObject<QStackedWidget>("Application.TestGuiComponent.StackedWidget1"));
        QString widgetName = QString("Application.TestGuiComponent.Widget%1").arg(i+1);
        QWidget *childWidget = gFindObject<QWidget>(widgetName);
        QVERIFY(childWidget != 0);
        QVERIFY(evt.Child == qobject_cast<QObject*>(childWidget));

        QStackedWidget *parentWidget = qobject_cast<QStackedWidget*>(evt.Parent);
        QVERIFY(parentWidget != 0);
        QWidget *widget = qobject_cast<QWidget*>(evt.Child);
        QVERIFY(widget != 0);
        QVERIFY(widget->isEnabled());
        widgetPairs.append(qMakePair(qobject_cast<QObject*>(parentWidget),
                                                qobject_cast<QObject*>(childWidget)));
    }

    activateEvents.clear();
    deactivateEvents.clear();

    comp->deactivate();

    QVERIFY(activateEvents.count() == 0);
    QVERIFY(deactivateEvents.count() == 2);
    for(int i=0; i<deactivateEvents.count(); i++)
    {
        UnmergeEvent evt = deactivateEvents.at(i);
        QVERIFY(evt.Parent == widgetPairs.at(i).first);
        QVERIFY(evt.Child == widgetPairs.at(i).second);
        QWidget *widget = qobject_cast<QWidget*>(evt.Child);
        QVERIFY(widget != 0);
        QVERIFY(!widget->isEnabled());
    }
    comp->unload();
}

int main(int argc, char *argv[])
{
    GCF::GuiApplication app(argc, argv);
    GuiComponentTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_GuiComponentTest.moc"
