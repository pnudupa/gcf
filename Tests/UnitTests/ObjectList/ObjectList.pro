#-------------------------------------------------
#
# Project created by QtCreator 2013-06-24T17:13:15
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_ObjectListTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCF3.prf)

SOURCES += tst_ObjectListTest.cpp \
    Object.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    Object.h \
    ObjectListListeners.h
