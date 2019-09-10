LIBS += -L$$PWD/../../../Binary -lGCF3 -lGCFGui3 -lGCFQuick3

include($$PWD/../../../QMakePRF/GCFGui3.prf)
DESTDIR = $$PWD/../../../Binary/Examples
CONFIG -= app_bundle
TARGET = Transfigure

QT += gui qml quick

SOURCES += \
    Main.cpp

HEADERS += \
    FilteredImageProvider.h \
    IImageFilter.h \
    FiltersModel.h

RESOURCES += \
    Application.qrc

OTHER_FILES += \
    Main.qml \
    Button.qml \
    ZoomButton.qml \
    Transfigure.dox
