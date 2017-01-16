function test_qmlContextObject() {
    var test = object("Application.tst_AgentScriptRunnerTest")

    test.mapObject("Application.QmlEngine", object("Application.QmlEngine"));
    test.mapObject("Application.QmlEngine/Object1", object("Application.QmlEngine/Object1"));
    test.mapObject("Application.QmlEngine/Object2", object("Application.QmlEngine/Object2"));
    test.mapObject("Application.QmlEngine/Object3", object("Application.QmlEngine/Object3"));
    test.mapObject("Application.QmlEngine/Object4", object("Application.QmlEngine/Object4"));
    test.mapObject("Application.QmlEngine/Object5", object("Application.QmlEngine/Object5"));
}
