QT       += testlib network script
QT       -= gui

TARGET = tst_FiberTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/ManualTests
include($$PWD/../../../QMakePRF/GCF3.prf)

DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += ../../../Fiber/Fiber

SOURCES += \
    tst_FiberTest.cpp \
    FiberAccess.cpp \
    HTTPCaller.cpp \
    ../../../Fiber/Fiber/Json.cpp

HEADERS += \
    FiberAccess.h \
    HTTPCaller.h \
    ../../../Fiber/Fiber/Json.h
