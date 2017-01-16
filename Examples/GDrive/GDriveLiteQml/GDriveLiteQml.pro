TEMPLATE = app
CONFIG -= app_bundle
TARGET = GDriveLiteQml
DESTDIR = $$PWD/../../../Binary/Examples
INCLUDEPATH += $$PWD/../../../Include
LIBS += -L$$PWD/../../../Binary -lGCF3 -lGCFQuick3

QT += network webkit qml quick

SOURCES += \
    Main.cpp \
    GDriveLiteContentListModel.cpp \
    ThumbnailImageProvider.cpp \
    GDriveLiteHelper.cpp

OTHER_FILES += \
    MainWindow.qml \
    AuthenticationWizard.qml \
    DriveExplorer.qml \
    ContentBrowser.qml \
    ContentItemProperties.qml \
    JobsWindow.qml \
    OptionsButton.qml \
    OptionButtonsToolBar.qml \
    MoveOptionWindow.qml \
    Button.qml \
    LineEdit.qml \
    SharePermissionEditorPage.qml \
    SharePermissionPage.qml \
    ShareWindow.qml \
    ItemPropertiesEditor.qml \
    QmlMessageBox.qml \
    QmlInputDialog.qml \
    GDriveLiteQml.dox \
    RadioButton.qml

RESOURCES += \
    GDriveLiteQml.qrc

HEADERS += \
    Authenticator.h \
    GDriveLiteContentListModel_p.h \
    GDriveLiteContentListModel.h \
    ThumbnailImageProvider.h \
    GDriveLiteHelper.h \
    QmlDialog.h
