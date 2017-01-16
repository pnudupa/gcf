function test_dialClick() {
    var dialComp = object("Application.DialComponent");
    var dial = object("Application.DialComponent.Dial")
    dial.mousePressAtPos(dial.width/2, 10, Qt.LeftButton, Qt.NoModifier)

    var signalObj = waitForSignal(dialComp, "dialValueChanged(int)");

    verify(dial.value === 49, "Dial should have been 49, but it is " + dial.value)
    verify(dial.value === signalObj.value)
}
