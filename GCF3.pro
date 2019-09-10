TEMPLATE = subdirs
CONFIG += ordered

isEqual(QT_MAJOR_VERSION, 5) {
cache()
}

SUBDIRS += \
    Core \
    Gui \
    Ipc \
    Investigator \
    # Fiber \
    # GDrive

isEqual(QT_MAJOR_VERSION, 5) {
SUBDIRS += Quick
} else {
message("GCF-QtQuick support can only be used with Qt 5")
}

SUBDIRS += Examples
SUBDIRS += Demos

# SUBDIRS += Tests

macx {
QMAKE_CXXFLAGS += -Wnoweak-tables
QMAKE_CFLAGS += -Wnoweak-tables
}


