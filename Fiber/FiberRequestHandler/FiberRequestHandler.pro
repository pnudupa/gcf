TEMPLATE = app

CONFIG -= app_bundle
CONFIG += console

TARGET = FiberRequestHandler

DESTDIR = $$PWD/../../Binary

include($$PWD/../../QMakePRF/GCF3.prf)
QT += script network concurrent

INCLUDEPATH += ../Fiber

HEADERS += \
    ../Fiber/FiberConfigurationSettings.h \
    ../Fiber/Json.h \
    FiberRequestHandlerServer.h \
    RequestManager.h

SOURCES += \
    ../Fiber/FiberConfigurationSettings.cpp \
    ../Fiber/Json.cpp \
    FiberRequestHandlerMain.cpp \
    FiberRequestHandlerServer.cpp \
    RequestManager.cpp
