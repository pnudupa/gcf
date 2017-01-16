TEMPLATE = lib

CONFIG -= app_bundle

DESTDIR = $$PWD/../../../../Binary/Tests/Helpers
include($$PWD/../../../../QMakePRF/GCF3.prf)

TARGET = SimpleCoreComponent

INCLUDEPATH += ../../IpcServerApp

HEADERS += \
    SimpleCoreComponent.h \
    ../../IpcServerApp/TestService.h

OTHER_FILES += \
    SimpleCoreComponent.xml

RESOURCES += \
    SimpleCoreComponent.qrc

SOURCES += \
    SimpleCoreComponent.cpp
