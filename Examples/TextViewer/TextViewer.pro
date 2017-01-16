include($$PWD/../../QMakePRF/GCFGui3.prf)
DESTDIR = $$PWD/../../Binary/Examples
CONFIG -= app_bundle

HEADERS += \
    TextViewerComponent.h \
    FileBrowserComponent.h

SOURCES += \
    Main.cpp \
    TextViewerComponent.cpp \
    FileBrowserComponent.cpp

RESOURCES += \
    TextViewer.qrc

OTHER_FILES += \
    TextViewerComponent.xml \
    FileBrowserComponent.xml
