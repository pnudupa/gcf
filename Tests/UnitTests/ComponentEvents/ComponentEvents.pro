#-------------------------------------------------
#
# Project created by QtCreator 2013-07-01T15:54:30
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_ComponentEventsTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCF3.prf)

SOURCES += tst_ComponentEventsTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    AppEventListener.h \
    SimpleComponent.h \
    SimpleContentComponent.h \
    GenericComponent.h

OTHER_FILES += \
    ComponentContent.xml \
    PlatformComponent.xml \
    TreeComponent.xml \
    EditorComponent.xml \
    DuplicateObjects.xml \
    LoadUnloadObjects.xml \
    MergeUnmergeObjects.xml \
    ActivateDeactivateObjects.xml \

INCLUDEPATH += $$PWD/../ObjectList/
SOURCES += $$PWD/../ObjectList/Object.cpp
HEADERS += $$PWD/../ObjectList/Object.h

RESOURCES += \
    ComponentEvents.qrc
