function test_interfaceTest() {
    var test = object("Application.tst_AgentScriptRunnerTest")

    // These two will be valid pointers.
    test.mapObject("QMainWindow", interface("QMainWindow"));

    // Each of these should contain NULL pointers because the
    // objects are not included in the object-tree of the application.
    test.mapObject("QTreeView", interface("QTreeView"));
    test.mapObject("QCheckBox", interface("QCheckBox"));
    test.mapObject("GCF::IGDriveLite", interface("GCF::IGDriveLite"));
}
