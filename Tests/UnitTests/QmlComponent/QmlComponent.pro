#-------------------------------------------------
#
# Project created by QtCreator 2013-07-12T08:56:34
#
#-------------------------------------------------

QT       += testlib
isEqual(QT_MAJOR_VERSION, 5) {
QT += widgets
}

TARGET = tst_QmlComponentTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DESTDIR = $$PWD/../../../Binary/Tests/UnitTests
include($$PWD/../../../QMakePRF/GCFQuick3.prf)

SOURCES += tst_QmlComponentTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    GenericQmlComponent.h

OTHER_FILES += \
    QmlComponent1.xml \
    ../../../Binary/Tests/UnitTests/TestQmlComponent2.qml \
    TestQmlComponent3.qml \
    TestQmlComponent5.qml \
    QmlComponent2.xml \
    TestQmlComponent1.qml

RESOURCES += \
    QmlComponent.qrc
