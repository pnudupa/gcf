#-------------------------------------------------
#
# Project created by QtCreator 2013-07-08T16:41:29
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_ResultTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCFGui3.prf)

SOURCES += tst_ResultTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
