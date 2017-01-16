function test_comboBox() {
    var test = object("Application.tst_AgentScriptRunnerTest")

    var comboBox = object("Application.Window/QComboBox[0]")
    test.addString(comboBox.count)

    comboBox.currentIndex = 3
    test.addString(comboBox.text)

    test.addString( comboBox.indexOf("item 8") )
}
