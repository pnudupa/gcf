function test_objectTest() {
    var test = object("Application.tst_AgentScriptRunnerTest")

    // Case #1 tests
    test.mapObject("Application.MainWindow", object("Application.MainWindow"))
    test.mapObject("Application.ControlsWindow", object("Application.ControlsWindow"));
    test.mapObject("Application.tst_AgentScriptRunnerTest", test);
    test.mapObject("Application.MainWindow/treeView", object("Application.MainWindow/treeView"));
    test.mapObject("Application.MainWindow/textEdit1", object("Application.MainWindow/textEdit1"));

    // Case #3 tests
    test.mapObject("Application.ControlsWindow/QCheckBox[0]", object("Application.ControlsWindow/QCheckBox[0]"));
    test.mapObject("Application.MainWindow/QMenuBar[0]", object("Application.MainWindow/QMenuBar[0]"));

    // Case #4 tests (invalids)
    test.mapObject("Application.NonExistentObject", object("Application.NonExistentObject"));
}
