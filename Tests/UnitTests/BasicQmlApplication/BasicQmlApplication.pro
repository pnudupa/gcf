#-------------------------------------------------
#
# Project created by QtCreator 2013-07-12T08:37:01
#
#-------------------------------------------------

QT       += testlib

TARGET = tst_BasicQmlApplicationTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCFQuick3.prf)

SOURCES += tst_BasicQmlApplicationTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
