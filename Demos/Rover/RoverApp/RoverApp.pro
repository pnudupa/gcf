DESTDIR = $$PWD/../../../Binary/Demos
TARGET = Rover
CONFIG -= app_bundle
QT += gui widgets qml quick

INCLUDEPATH += $$PWD/../../../Include
LIBS += -L$$PWD/../../../Binary -lGCF3 -lGCFGui3

SOURCES += \
    Main.cpp \
    RoverApp.cpp \
    RoverAppWindow.cpp \
    ContentItemModel.cpp \
    ThumbnailImageProvider.cpp

HEADERS += \
    RoverApp.h \
    RoverAppWindow.h \
    FolderContentModel.h \
    ContentItemModel.h \
    ThumbnailImageProvider.h

OTHER_FILES += \
    RoverApp.xml \
    ContentView.qml \
    FolderListView.qml \
    FolderGridView.qml \
    Rover.dox

RESOURCES += \
    RoverApp.qrc
