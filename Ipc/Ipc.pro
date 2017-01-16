TEMPLATE = lib
CONFIG += shared
DEFINES += GCF_IPC_DLL_BUILD
win32 {
TARGET = GCFIpc
} else {
TARGET = GCFIpc3
}
DESTDIR = $$PWD/../Binary
QT += core network
INCLUDEPATH += $$PWD $$PWD/../Core
LIBS += -L$$PWD/../Binary -lGCF3

# Everytime we bump the version, the following lines need to be changed
# Also, GCF.h must be to have GCF_VERSION_xxx macros reflect this.
VER_MAJ = 3
VER_MIN = 0
VER_PAT = 0
VERSION = 3.0.0

HEADERS += \
    IpcCommon.h \
    IpcServer.h \
    IpcServer_p.h \
    IpcCall.h \
    IpcCommon_p.h \
    IpcRemoteObject.h \
    IpcServerDiscovery.h

SOURCES += \
    IpcServer.cpp \
    IpcCall.cpp \
    IpcCommon_p.cpp \
    IpcRemoteObject.cpp \
    IpcServerDiscovery.cpp

OTHER_FILES += \
    Ipc.dox

