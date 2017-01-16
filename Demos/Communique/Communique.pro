INCLUDEPATH += $$PWD/../../Include
LIBS += -L$$PWD/../../Binary/ -lGCF3 -lGCFGui3 -lGCFIpc3
DESTDIR = $$PWD/../../Binary/Demos
CONFIG -= app_bundle

QT += network
isEqual(QT_MAJOR_VERSION, 5) {
QT += gui widgets
}

SOURCES += \
    Main.cpp \
    CommuniqueWindow.cpp

HEADERS += \
    CommuniqueWindow.h

FORMS += \
    CommuniqueWindow.ui

OTHER_FILES += \
    Communique.dox
