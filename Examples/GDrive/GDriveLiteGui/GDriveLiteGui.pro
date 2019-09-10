TEMPLATE = app
CONFIG -= app_bundle
TARGET = GDriveLiteGui
DESTDIR = $$PWD/../../../Binary/Examples
INCLUDEPATH += $$PWD/../../../Include
LIBS += -L$$PWD/../../../Binary -lGCF3 -lGCFGui3

QT += network

isEqual(QT_MAJOR_VERSION, 5) {
QT += webkitwidgets widgets qml quick
} else {
QT += webkit declarative
}

SOURCES += \
    Main.cpp \
    DriveExplorerWindow.cpp

FORMS += \
    DriveExplorerWindow.ui \
    AddSharePermissionsDialog.ui \
    UpdateSharePermissionsDialog.ui \
    RenameDialog.ui \
    DescribeDialog.ui \
    TargetFolderSelectionDialog.ui

HEADERS += \
    DriveExplorerWindow.h \
    DriveFolderModel.h

OTHER_FILES += \
    ShareWindowQt5.qml \
    ShareWindowQt4.qml \
    JobsWindowQt5.qml \
    JobsWindowQt4.qml \
    GDriveLiteGui.dox

RESOURCES += \
    GDriveLiteGui.qrc
