#-------------------------------------------------
#
# Project created by QtCreator 2013-07-02T13:28:34
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_FindFileTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCF3.prf)

SOURCES += tst_FindFileTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
