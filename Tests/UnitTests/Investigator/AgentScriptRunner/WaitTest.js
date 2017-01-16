function test_wait() {
    var test = object("Application.tst_AgentScriptRunnerTest")

    test.waitStart()
    wait(500)
    var msecs = test.waitEnd();

    test.addString(msecs);
}
