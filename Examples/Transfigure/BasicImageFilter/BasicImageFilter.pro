TEMPLATE = lib
CONFIG += shared plugin # the plugin is to avoid symbolic links to the .so or .dylib
TARGET = BasicFilters

include($$PWD/../../../QMakePRF/GCFQuick3.prf)
DESTDIR = $$PWD/../../../Binary/Examples/TransfigureComponents
CONFIG -= app_bundle
QT += gui

INCLUDEPATH += . $$PWD/../Application

HEADERS += \
    BasicImageFilter.h

SOURCES += \
    BasicImageFilter.cpp
