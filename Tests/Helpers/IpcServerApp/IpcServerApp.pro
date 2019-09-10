TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
DESTDIR = $$PWD/../../../Binary/Tests/Helpers
include($$PWD/../../../QMakePRF/GCFIpc3.prf)

TARGET = IpcServerApp

HEADERS += \
    TestService.h

SOURCES += \
    Main.cpp

