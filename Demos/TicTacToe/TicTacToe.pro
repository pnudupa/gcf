INCLUDEPATH += $$PWD/../../Include
LIBS += -L$$PWD/../../Binary/ -lGCF3 -lGCFQuick3 -lGCFIpc3
DESTDIR = $$PWD/../../Binary/Demos
CONFIG -= app_bundle
QT += network gui qml quick

OTHER_FILES += \
    TicTacToeBoard.qml \
    TicTacToe.dox

SOURCES += \
    Main.cpp \
    TicTacToe.cpp

RESOURCES += \
    TicTacToe.qrc

HEADERS += \
    TicTacToe.h
