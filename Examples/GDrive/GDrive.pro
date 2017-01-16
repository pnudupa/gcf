TEMPLATE = subdirs

SUBDIRS += GDriveLiteGui

isEqual(QT_MAJOR_VERSION, 5) {
SUBDIRS += GDriveLiteQml
}
