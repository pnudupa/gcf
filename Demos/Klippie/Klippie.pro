INCLUDEPATH += $$PWD/../../Include
LIBS += -L$$PWD/../../Binary/ -lGCF3 -lGCFQuick3 -lGCFIpc3
DESTDIR = $$PWD/../../Binary/Demos
CONFIG -= app_bundle
QT += network gui qml quick

HEADERS += \
    Klippie.h \
    KlippieModel.h

SOURCES += \
    Klippie.cpp \
    KlippieModel.cpp \
    Main.cpp

RESOURCES += \
    Klippie.qrc

DISTFILES += \
    Klippie.dox

