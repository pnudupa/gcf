#-------------------------------------------------
#
# Project created by QtCreator 2013-07-04T00:55:42
#
#-------------------------------------------------

QT       += testlib

TARGET = tst_BasicGuiApplicationTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCFGui3.prf)

SOURCES += tst_BasicGuiApplicationTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
