TEMPLATE = lib
CONFIG += shared
DEFINES += GCF_DLL_BUILD
win32 {
TARGET = GCF
} else {
TARGET = GCF3
}
DESTDIR = $$PWD/../Binary
QT += core xml

isEqual(QT_MAJOR_VERSION, 5) {
QT -= gui widgets
} else {
QT -= gui
}

# Everytime we bump the version, the following lines need to be changed
# Also, GCFGlobal.h must be to have GCF_VERSION_xxx macros reflect this.
VER_MAJ = 3
VER_MIN = 0
VER_PAT = 0
VERSION = 3.0.0

HEADERS += \
    Version.h \
    Log.h \
    ObjectList.h \
    ObjectList_p.h \
    ObjectMap.h \
    MapToObject.h \
    ObjectTree.h \
    Application.h \
    Component.h \
    GCFGlobal.h \
    Application_p.h \
    SignalSpy.h \
    AbstractJob.h \
    JobListModel.h

SOURCES += \
    Log.cpp \
    ObjectList.cpp \
    ObjectTree.cpp \
    Application.cpp \
    Component.cpp \
    GCFGlobal.cpp \
    Application_p.cpp \
    Job.cpp

OTHER_FILES += \
    Version.dox \
    SignalSpy.dox \
    Result.dox \
    ObjectMap.dox \
    MapToObject.dox \
    Core.dox \
    ComponentModel.dox
