#-------------------------------------------------
#
# Project created by QtCreator 2013-06-25T16:11:08
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_ObjectMapTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCF3.prf)

SOURCES += tst_ObjectMapTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += $$PWD/../ObjectList/
SOURCES += $$PWD/../ObjectList/Object.cpp
HEADERS += $$PWD/../ObjectList/Object.h \
    ObjectMapListeners.h
