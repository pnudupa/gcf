#-------------------------------------------------
#
# Project created by QtCreator 2013-09-07T12:39:53
#
#-------------------------------------------------

QT       += testlib gui

TARGET = tst_JobTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCFGui3.prf)

SOURCES += tst_JobTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
