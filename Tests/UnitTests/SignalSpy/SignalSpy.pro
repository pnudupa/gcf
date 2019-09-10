#-------------------------------------------------
#
# Project created by QtCreator 2013-08-01T17:52:35
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_SignalSpyTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCF3.prf)

SOURCES += tst_SignalSpyTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
