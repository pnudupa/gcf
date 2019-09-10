#-------------------------------------------------
#
# Project created by QtCreator 2013-09-02T14:39:41
#
#-------------------------------------------------

QT       += gui testlib network
isEqual(QT_MAJOR_VERSION, 5) {
QT += webkitwidgets widgets
} else {
QT += webkit script
}

TARGET = tst_GDriveLiteTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../../Binary/Tests/UnitTests
include($$PWD/../../../../QMakePRF/GCFGui3.prf)

SOURCES += tst_GDriveLiteTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += $$PWD/../../../../GDrive/Lite/
HEADERS += $$PWD/../../../../GDrive/Lite/Json.h
SOURCES += $$PWD/../../../../GDrive/Lite/Json.cpp

HEADERS += \
    TestSettings.h \
    GDriveRequestAPI.h

