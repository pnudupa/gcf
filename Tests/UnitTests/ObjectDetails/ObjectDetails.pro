#-------------------------------------------------
#
# Project created by QtCreator 2013-07-08T10:27:54
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_ObjectDetailsTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCF3.prf)

SOURCES += tst_ObjectDetailsTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

OTHER_FILES += \
    Properties.xml \
    Connections.xml

INCLUDEPATH += $$PWD/../ObjectList/
SOURCES += $$PWD/../ObjectList/Object.cpp
HEADERS += $$PWD/../ObjectList/Object.h \
    Sender.h \
    Receiver.h \
    ConnectionTestComponent.h

INCLUDEPATH += $$PWD/../ComponentEvents/
HEADERS += $$PWD/../ComponentEvents/GenericComponent.h

RESOURCES += \
    ObjectDetails.qrc
