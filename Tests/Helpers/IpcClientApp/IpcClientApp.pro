TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
DESTDIR = $$PWD/../../../Binary/Tests/Helpers
include($$PWD/../../../QMakePRF/GCFIpc3.prf)

TARGET = IpcClientApp

HEADERS += \
    TestClient.h

SOURCES += \
    Main.cpp \
    TestClient.cpp

