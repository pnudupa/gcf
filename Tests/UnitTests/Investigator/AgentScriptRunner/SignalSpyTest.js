function test_signalSpy() {
    var test = object("Application.tst_AgentScriptRunnerTest")
    var sigEmitter = test.signalEmitter()

    var spy = createSignalSpy(sigEmitter, "signal(QString)")
    verify(spy !== false)

    var args;

    sigEmitter.emitSignal(0, "take #1")
    verify(spy.count === 1)
    args = spy.at(0)
    verify(args.length === 1)
    verify(args[0] === "take #1")
    spy.clear()

    sigEmitter.emitSignal(500, "take #2")
    verify(spy.count === 0)
    spy.wait()
    verify(spy.count === 1)
    args = spy.at(0)
    verify(args.length === 1)
    verify(args[0] === "take #2")
    spy.clear()

    sigEmitter.emitSignal(1000, "take #3")
    verify(spy.count === 0)
    spy.wait(500)
    verify(spy.count === 0)
    spy.clear()

    spy = createSignalSpy(sigEmitter, "invalidSignal(QVariant)")
    verify(spy === false)
}
