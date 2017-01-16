TEMPLATE = subdirs

SUBDIRS += Communique

isEqual(QT_MAJOR_VERSION, 5) {
SUBDIRS += TicTacToe

greaterThan(QT_MINOR_VERSION, 0) {
SUBDIRS += Rover
}

}
