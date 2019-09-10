#-------------------------------------------------
#
# Project created by QtCreator 2013-06-20T11:31:02
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_VersionTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCF3.prf)

SOURCES += tst_VersionTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
