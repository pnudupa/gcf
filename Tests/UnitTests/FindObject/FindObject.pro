#-------------------------------------------------
#
# Project created by QtCreator 2013-07-02T01:59:09
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_FindObjectTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCF3.prf)

SOURCES += tst_FindObjectTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += $$PWD/../ObjectList/
SOURCES += $$PWD/../ObjectList/Object.cpp
HEADERS += $$PWD/../ObjectList/Object.h
