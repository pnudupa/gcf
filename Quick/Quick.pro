TEMPLATE = lib
CONFIG += shared
DEFINES += GCF_QUICK_DLL_BUILD
win32 {
TARGET = GCFQuick
} else {
TARGET = GCFQuick3
}
DESTDIR = $$PWD/../Binary
QT += core xml gui qml quick
INCLUDEPATH += $$PWD $$PWD/../Core
LIBS += -L$$PWD/../Binary -lGCF3

# Everytime we bump the version, the following lines need to be changed
# Also, GCFCommon.h must be to have GCF_VERSION_xxx macros reflect this.
VER_MAJ = 3
VER_MIN = 0
VER_PAT = 0
VERSION = 3.0.0

HEADERS += \
    QmlApplication.h \
    QuickCommon.h \
    QmlComponent.h

SOURCES += \
    QmlApplication.cpp \
    QmlComponent.cpp

OTHER_FILES += \
    Quick.dox
