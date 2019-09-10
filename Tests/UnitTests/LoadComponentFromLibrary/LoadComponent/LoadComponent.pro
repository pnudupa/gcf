#-------------------------------------------------
#
# Project created by QtCreator 2013-07-02T21:51:45
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_LoadComponentTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../../Binary/Tests/UnitTests
include($$PWD/../../../../QMakePRF/GCF3.prf)

SOURCES += tst_LoadComponentTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
