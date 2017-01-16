TEMPLATE = lib
CONFIG += shared plugin
DESTDIR = $$PWD/../../Binary/Investigator
QT += core gui script testlib
isEqual(QT_MAJOR_VERSION, 5) {
QT += widgets
}
INCLUDEPATH += $$PWD $$PWD/../../Include
LIBS += -L$$PWD/../../Binary -lGCF3 -lGCFGui3

isEqual(QT_MAJOR_VERSION, 5) {
QT += qml quick
} else {
QT += declarative
}

HEADERS += \
    AgentComponent.h \
    Agent.h \
    AgentScriptRunner.h \
    AgentSignalSpy.h \
    AgentScriptRecorder.h

SOURCES += \
    AgentComponent.cpp \
    AgentScriptRunner.cpp \
    AgentScriptRunnerFindObject.cpp \
    AgentScriptRunnerWrap.cpp \
    AgentScriptRecorder.cpp

OTHER_FILES += \
    ../Samples/SimpleGUITestCase1.js
