TEMPLATE = lib
CONFIG += plugin shared
TARGET = GDriveLite
DESTDIR = $$PWD/../../Binary/GDrive
QT += gui core network
INCLUDEPATH += $$PWD/../../Include
LIBS += -L$$PWD/../../Binary -lGCF3

isEqual(QT_MAJOR_VERSION, 4) {
QT += script
}

HEADERS += \
    GDriveLite.h \
    GDriveLite_p.h \
    Json.h \
    IGDriveLiteAuthenticator.h \
    IGDriveLite.h \
    GDriveLiteContentModel.h \
    GDriveLiteAPI.h \
    IGDriveLiteContentModel.h \
    GDriveContent.h \
    GDriveLiteContentModel_p.h \
    IGDriveLiteFileDownloader.h \
    GDriveLiteFileDownloader.h \
    IGDriveLiteFileUploader.h \
    GDriveLiteFileUploader.h \
    IGDriveLiteFileDownloaderListModel.h \
    GDriveLiteFileDownloaderListModel.h \
    IGDriveLiteFileUploaderListModel.h \
    GDriveLiteFileUploaderListModel.h \
    IGDriveLiteContentSharingModel.h \
    GDriveLiteContentSharingModel.h

SOURCES += \
    GDriveLite.cpp \
    Json.cpp \
    GDriveLiteContentModel.cpp \
    GDriveLiteAPI.cpp \
    GDriveLiteContentModel_p.cpp \
    GDriveLiteFileDownloader.cpp \
    GDriveLiteFileUploader.cpp \
    GDriveLiteFileDownloaderListModel.cpp \
    GDriveLiteFileUploaderListModel.cpp \
    GDriveLiteContentSharingModel.cpp

RESOURCES += \
    Lite.qrc

OTHER_FILES += \
    GDriveLite.dox \
    IGDriveLite.dox \
    IGDriveLiteAuthenticator.dox \
    IGDriveLiteContentModel.dox \
    IGDriveLiteContentSharingModel.dox \
    IGDriveLiteFileDownloader.dox \
    IGDriveLiteFileUploader.dox \
    IGDriveLiteFileDownloaderListModel.dox \
    IGDriveLiteFileUploaderListModel.dox

