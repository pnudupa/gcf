TEMPLATE = lib
CONFIG += shared plugin # the plugin is to avoid symbolic links to the .so or .dylib
TARGET = SpiralFlowView
DESTDIR = $$PWD/../../../Binary/Demos/RoverComponents

QT += gui widgets qml quick
INCLUDEPATH += $$PWD/../../../Include
LIBS += -L$$PWD/../../../Binary -lGCF3 -lGCFGui3

RESOURCES += \
    SpiralFlow.qrc

OTHER_FILES += \
    SpiralFlow.xml \
    FolderSpiralFlowView.qml

HEADERS += \
    SpiralFlowComponent.h

SOURCES += \
    SpiralFlowComponent.cpp
