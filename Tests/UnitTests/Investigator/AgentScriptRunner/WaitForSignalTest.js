function test_waitForSignal() {
    var test = object("Application.tst_AgentScriptRunnerTest")
    var sigEmitter = test.signalEmitter()
    var sig = false;

    // Test for waitForSignal() that does wait sucessfully
    sigEmitter.emitSignal(500, "take #1")
    sig = waitForSignal(sigEmitter, "signal(QString)")
    if(sig === false)
        fail("waitForSignal-take #1 failed")
    if(sig.value !== "take #1")
        fail("waitForSignal-take #1 failed")

    // Test for waitForSignal() that does not wait successfully
    sigEmitter.emitSignal(1000, "take #2")
    sig = waitForSignal(sigEmitter, "signal(QString)", 500)
    if(sig !== false)
        fail("waitForSignal-take #2 failed")

    // Test for waiting on invalid (non-existing) signal
    sig = waitForSignal(sigEmitter, "invalidSignal(QVariant)")
    if(sig !== false)
        fail("waitForSignal-take #3 failed")
}
