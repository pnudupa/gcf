#-------------------------------------------------
#
# Project created by QtCreator 2013-07-05T12:25:06
#
#-------------------------------------------------

QT       += testlib

TARGET = tst_GuiComponentTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCFGui3.prf)

SOURCES += tst_GuiComponentTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    TestGuiComponent.h

OTHER_FILES += \
    TestLoadUnloadWidget.xml \
    TestLoadUnloadAction.xml \
    TestLoadUnloadActionGroup.xml \
    TestLoadUnloadMenu.xml \
    TestLoadUnloadMenuBar.xml \
    TestLoadUnloadToolBar.xml \
    TestMergeLayoutWithWidget.xml \
    TestMergeUnmergeWithWidget.xml \
    TestMergeUnmergeWithActionGroup.xml \
    TestMergeUnmergeWithMenu.xml \
    TestMergeUnmergeWithToolBar.xml \
    TestMergeUnmergeWithMenuBar.xml \
    TestActivateDeactivateObject.xml \
    TestActivateDeactivateWidget.xml

RESOURCES += \
    GuiComponent.qrc
