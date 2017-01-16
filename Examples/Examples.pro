TEMPLATE = subdirs

SUBDIRS += \
    SimpleGUI \
    TextViewer \
    GDrive

isEqual(QT_MAJOR_VERSION, 5) {
SUBDIRS += Transfigure
}

OTHER_FILES += Examples.dox
