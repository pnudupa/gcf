function test_widgetWrap() {
    var window = object('Application.Window')

    window.resize(320, 240)
    wait(100)

    window.resize(640, 480)
    wait(100)

    window.mousePress(Qt.LeftButton, Qt.ShiftModifier)
    window.mouseRelease(Qt.LeftButton, Qt.ShiftModifier)

    window.mousePressAtPos(10, 10, Qt.LeftButton, Qt.NoModifier)
    window.mouseMove(40, 20)
    window.mouseReleaseAtPos(40, 20, Qt.LeftButton, Qt.NoModifier)

    window.mouseClick(Qt.LeftButton, Qt.NoModifier)

    window.mouseClick(100, 50, Qt.LeftButton, Qt.ControlModifier)

    window.mouseDClick(Qt.LeftButton, Qt.NoModifier)

    window.mouseDClickAtPos(100, 50, Qt.LeftButton, Qt.AltModifier)

    window.keyPress('a', Qt.NoModifier)
    window.keyRelease('a', Qt.NoModifier)
    window.keyClick('b', Qt.NoModifier)

    window.keyPress(Qt.Key_C, Qt.NoModifier)
    window.keyRelease(Qt.Key_C, Qt.NoModifier)
    window.keyClick(Qt.Key_D, Qt.NoModifier)

    window.keyClicks('abcdef', Qt.ShiftModifier)

    window.wheel(-1, Qt.NoModifier)
    window.wheelAtPos(50, 100, 1, Qt.ShiftModifier)
}
