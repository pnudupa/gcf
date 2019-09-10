#-------------------------------------------------
#
# Project created by QtCreator 2013-08-26T16:04:44
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_IpcServerDiscoveryTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/ManualTests
include($$PWD/../../../QMakePRF/GCFIpc3.prf)

SOURCES += tst_IpcServerDiscoveryTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
