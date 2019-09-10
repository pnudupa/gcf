#-------------------------------------------------
#
# Project created by QtCreator 2013-07-23T14:32:59
#
#-------------------------------------------------

QT       += testlib concurrent
QT       -= gui

TARGET = tst_IpcTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/ManualTests
include($$PWD/../../../QMakePRF/GCFIpc3.prf)

SOURCES += tst_IpcTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    Caller.h
