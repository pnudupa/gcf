TEMPLATE = subdirs

SUBDIRS += \
    Version \
    Log \
    ObjectList \
    ObjectMap \
    MapToObject \
    ObjectTree \
    ComponentEvents \
    FindObject \
    FindFile \
    LoadComponentFromLibrary \
    BasicGuiApplication \
    GuiComponent \
    ObjectDetails \
    Result \
    Ipc \
    SignalSpy \
    IpcRemoteObject \
    Investigator \
    Fiber \
    IpcServerDiscovery \
    Job \
    JobList \
    GDriveTests

isEqual(QT_MAJOR_VERSION, 5) {
SUBDIRS += BasicQmlApplication \
           QmlComponent
} else {
message("GCF-QtQuick support can only be used with Qt 5. So we wont be compiling those test cases.")
}
