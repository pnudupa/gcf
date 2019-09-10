include($$PWD/../../QMakePRF/GCFGui3.prf)
DESTDIR = $$PWD/../../Binary/Examples
CONFIG -= app_bundle

SOURCES += \
    Main.cpp

HEADERS += \
    AppWindowComponent.h \
    DialComponent.h \
    CalendarComponent.h

OTHER_FILES += \
    DialComponent.xml \
    AppWindowComponent.xml \
    CalendarComponent.xml

RESOURCES += \
    SimpleGUI.qrc
