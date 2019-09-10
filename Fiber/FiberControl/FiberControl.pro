TEMPLATE = app

CONFIG -= app_bundle
CONFIG += console

TARGET = FiberCtl

DESTDIR = $$PWD/../../Binary

QT += network script

INCLUDEPATH += ../Fiber

SOURCES += \
    FiberControlMain.cpp \
    ../Fiber/Json.cpp

HEADERS += \
    ../Fiber/Json.h
