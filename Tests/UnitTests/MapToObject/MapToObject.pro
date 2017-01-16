#-------------------------------------------------
#
# Project created by QtCreator 2013-06-26T11:03:46
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_MapToObjectTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCF3.prf)

SOURCES += tst_MapToObjectTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    MapToObjectListeners.h

INCLUDEPATH += $$PWD/../ObjectList/
SOURCES += $$PWD/../ObjectList/Object.cpp
HEADERS += $$PWD/../ObjectList/Object.h
