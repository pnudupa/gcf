#-------------------------------------------------
#
# Project created by QtCreator 2013-09-08T22:21:41
#
#-------------------------------------------------

QT       += testlib gui

TARGET = tst_JobListTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCFGui3.prf)

SOURCES += tst_JobListTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

isEqual(QT_MAJOR_VERSION, 5) {
QT += quick qml
} else {
QT += declarative
}

OTHER_FILES += \
    JobsListViewQt5.qml \
    JobsListViewQt4.qml

RESOURCES += \
    JobsList.qrc
