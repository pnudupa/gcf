TEMPLATE = app

CONFIG -= app_bundle
CONFIG += console

TARGET = Fiber

DESTDIR = $$PWD/../../Binary

include($$PWD/../../QMakePRF/GCF3.prf)
QT += script network

SOURCES += \
    JSON.cpp \
    FiberServer.cpp \
    FiberMain.cpp \
    FiberConfigurationSettings.cpp \
    FiberControlCentre.cpp

HEADERS += \
    Json.h \
    FiberServer.h \
    FiberCommon.h \
    FiberConfigurationSettings.h \
    FiberControlCentre.h
