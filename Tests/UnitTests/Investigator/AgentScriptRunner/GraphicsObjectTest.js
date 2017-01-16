function test_graphicsObjectTest() {
    var test = object("Application.tst_AgentScriptRunnerTest")

    test.mapObject("Application.GraphicsView", object("Application.GraphicsView"));
    test.mapObject("Application.GraphicsScene", object("Application.GraphicsScene"));
    test.mapObject("Application.GraphicsView/Calendar", object("Application.GraphicsView/Calendar"));
    test.mapObject("Application.GraphicsView/Dial", object("Application.GraphicsView/Dial"));
    test.mapObject("Application.GraphicsScene/Calendar", object("Application.GraphicsScene/Calendar"));
    test.mapObject("Application.GraphicsScene/Dial", object("Application.GraphicsScene/Dial"));
}
