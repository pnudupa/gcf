function test_waitForWindowShown() {
    var test = object("Application.tst_AgentScriptRunnerTest")

    var window = test.newControlsWindow()
    window.addString("before window active")
    window.show()
    waitForWindowShown(window)
    window.addString("after window active")
}
