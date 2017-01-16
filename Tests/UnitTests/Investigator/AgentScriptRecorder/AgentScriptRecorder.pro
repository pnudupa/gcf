#-------------------------------------------------
#
# Project created by QtCreator 2013-08-18T18:34:44
#
#-------------------------------------------------


QT       += testlib core gui script
isEqual(QT_MAJOR_VERSION, 5) {
QT += qml quick widgets
} else {
QT += declarative
}

TARGET = tst_AgentScriptRecorderTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../../Binary/Tests/UnitTests
include($$PWD/../../../../QMakePRF/GCFGui3.prf)

SOURCES += tst_AgentScriptRecorderTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

FORMS += \
    $$PWD/../AgentScriptRunner/MainWindow.ui \
    $$PWD/../AgentScriptRunner/ControlsWindow.ui
