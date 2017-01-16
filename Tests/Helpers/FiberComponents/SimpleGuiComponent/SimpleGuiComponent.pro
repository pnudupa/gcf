TEMPLATE = lib

CONFIG -= app_bundle

DESTDIR = $$PWD/../../../../Binary/Tests/Helpers
include($$PWD/../../../../QMakePRF/GCFGui3.prf)

TARGET = SimpleGuiComponent

INCLUDEPATH += ../../IpcServerApp

HEADERS += \
    SimpleGuiComponent.h \
    ../../IpcServerApp/TestService.h

SOURCES += \
    SimpleGuiComponent.cpp

RESOURCES += \
    SimpleGuiComponent.qrc

OTHER_FILES += \
    SimpleGuiComponent.xml
