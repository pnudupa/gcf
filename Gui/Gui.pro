TEMPLATE = lib
CONFIG += shared
DEFINES += GCF_GUI_DLL_BUILD
win32 {
TARGET = GCFGui
} else {
TARGET = GCFGui3
}
DESTDIR = $$PWD/../Binary
QT += core xml gui
isEqual(QT_MAJOR_VERSION, 5) {
QT += widgets
}
INCLUDEPATH += $$PWD $$PWD/../Core
LIBS += -L$$PWD/../Binary -lGCF3

# Everytime we bump the version, the following lines need to be changed
# Also, GCFCommon.h must be to have GCF_VERSION_xxx macros reflect this.
VER_MAJ = 3
VER_MIN = 0
VER_PAT = 0
VERSION = 3.0.0

HEADERS += \
    GuiCommon.h \
    GuiApplication.h \
    GuiComponent.h

SOURCES += \
    GuiApplication.cpp \
    GuiComponent.cpp

OTHER_FILES += \
    Gui.dox
