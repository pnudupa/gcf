TEMPLATE = lib
CONFIG += shared plugin # the plugin is to avoid symbolic links to the .so or .dylib
TARGET = SimpleComponent
DESTDIR = $$PWD/../../../../Binary/Tests/UnitTests/Components
include($$PWD/../../../../QMakePRF/GCF3.prf)
QT -= gui

HEADERS += \
    SimpleComponent.h

SOURCES += \
    SimpleComponent.cpp



