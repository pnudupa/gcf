TEMPLATE = app

CONFIG -= app_bundle
CONFIG += console

TARGET = FiberCoreSurrogate

DESTDIR = $$PWD/../../Binary

include($$PWD/../../QMakePRF/GCF3.prf)
QT += script network

INCLUDEPATH += ../Fiber

HEADERS += ../Fiber/Json.h \
    ../Fiber/FiberConfigurationSettings.h \
    FiberSurrogateServer.h

SOURCES += ../Fiber/Json.cpp \
    ../Fiber/FiberConfigurationSettings.cpp \
    FiberSurrogateServer.cpp \
    FiberCoreSurrogateMain.cpp
