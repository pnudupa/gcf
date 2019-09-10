#-------------------------------------------------
#
# Project created by QtCreator 2013-06-21T11:39:53
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_LogTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCF3.prf)

SOURCES += tst_LogTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    LogMessageHandler.h \
    Logger.h
