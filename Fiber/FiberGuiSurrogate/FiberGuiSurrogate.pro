TEMPLATE = app

CONFIG -= app_bundle

TARGET = FiberGuiSurrogate

DESTDIR = $$PWD/../../Binary

include($$PWD/../../QMakePRF/GCFGui3.prf)
QT += script network

INCLUDEPATH += ../Fiber ../FiberCoreSurrogate

HEADERS += ../Fiber/Json.h \
    ../Fiber/FiberConfigurationSettings.h \
    ../FiberCoreSurrogate/FiberSurrogateServer.h

SOURCES += ../Fiber/Json.cpp \
    ../Fiber/FiberConfigurationSettings.cpp \
    ../FiberCoreSurrogate/FiberSurrogateServer.cpp \
    FiberGuiSurrogateMain.cpp
