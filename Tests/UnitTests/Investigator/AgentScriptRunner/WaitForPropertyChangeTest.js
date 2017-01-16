function test_waitForPropertyChange() {
    var test = object("Application.tst_AgentScriptRunnerTest")
    var obj = test.signalEmitter();

    var inString = "india272";
    obj.string = inString
    var outString = waitForPropertyChange(obj, "string")
    if(inString !== outString)
        fail("Property value did not change as expected")
    if(inString !== obj.string)
        fail("Property value did not change as expected")

    // must report an error, becasue this property doesnt have
    // notification signals
    waitForPropertyChange(obj, "integer")

    // must report an error, becasue this property doesnt exist
    waitForPropertyChange(obj, "unknownProperty")
}
