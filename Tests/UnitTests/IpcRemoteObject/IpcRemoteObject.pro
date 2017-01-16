#-------------------------------------------------
#
# Project created by QtCreator 2013-08-06T14:36:48
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_IpcRemoteObjectTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/ManualTests
include($$PWD/../../../QMakePRF/GCFIpc3.prf)

SOURCES += tst_IpcRemoteObjectTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
