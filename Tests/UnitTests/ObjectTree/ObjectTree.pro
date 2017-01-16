#-------------------------------------------------
#
# Project created by QtCreator 2013-06-28T09:48:58
#
#-------------------------------------------------

QT       += testlib xml
QT       -= gui

TARGET = tst_ObjectTreeTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCF3.prf)

SOURCES += tst_ObjectTreeTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += $$PWD/../ObjectList/
SOURCES += $$PWD/../ObjectList/Object.cpp
HEADERS += $$PWD/../ObjectList/Object.h

OTHER_FILES += \
    Tree1.xml

RESOURCES += \
    ObjectTrees.qrc
