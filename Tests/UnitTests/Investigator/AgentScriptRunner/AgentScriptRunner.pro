#-------------------------------------------------
#
# Project created by QtCreator 2013-08-14T18:33:16
#
#-------------------------------------------------

QT       += testlib core gui script
isEqual(QT_MAJOR_VERSION, 5) {
QT += qml quick widgets
} else {
QT += declarative
}

TARGET = tst_AgentScriptRunnerTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../../Binary/Tests/UnitTests
include($$PWD/../../../../QMakePRF/GCFGui3.prf)

SOURCES += tst_AgentScriptRunnerTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

OTHER_FILES += \
    FunctionCallOrderTest.js \
    EmptyTestScript.js \
    NoTestFunctions.js \
    SyntaxErrorScript.js \
    LogTest.js \
    FailTest.js \
    ObjectTest.js \
    WaitTest.js \
    WaitForWindowShownTest.js \
    WaitForWindowActiveTest.js \
    WaitForSignalTest.js \
    WaitForPropertyChangeTest.js \
    VerifyTest.js \
    CompareTest.js \
    ObjectWrapTest.js \
    SignalSpyTest.js \
    WidgetWrapTest.js \
    ComboBoxTest.js \
    QmlContextPropertyTest.js \
    GraphicsObjectTest.js \
    WindowWrapTest.js \
    GraphicsObjectWrapTest.js \
    ScrollAreaAndItemViewPartsTest.js \
    InterfaceTest.js

FORMS += \
    MainWindow.ui \
    ControlsWindow.ui

HEADERS += \
    GraphicsView.h \
    ItemViewsForm.h
