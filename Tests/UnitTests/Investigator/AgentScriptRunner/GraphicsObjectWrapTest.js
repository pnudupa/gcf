function test_graphicsObjectTest() {
    var gView = object("Application.GraphicsView")
    var gViewport = object("Application.GraphicsView/viewport")
    var textObj = object("Application.GraphicsView/Text")
    var rect1Obj = object("Application.GraphicsView/Rect1")
    var rect2Obj = object("Application.GraphicsView/Rect2")

    textObj.mouseClick(Qt.LeftButton, Qt.NoModifier)
    gViewport.keyClick(Qt.Key_A, Qt.ControlModifier)
    gViewport.keyClicks("this is a new string", Qt.NoModifier)
    rect1Obj.mouseClick(Qt.LeftButton, Qt.NoModifier)
    verify(gView.text === "this is a new string")

    rect1Obj.mousePressAtPos(320, 50, Qt.LeftButton, Qt.NoModifier)
    rect1Obj.mouseMove(320, 60)
    rect1Obj.mouseReleaseAtPos(320, 60, Qt.LeftButton, Qt.NoModifier)

    rect2Obj.mousePressAtPos(320, 50, Qt.LeftButton, Qt.NoModifier)
    rect2Obj.mouseMove(320, 60)
    rect2Obj.mouseReleaseAtPos(320, 60, Qt.LeftButton, Qt.NoModifier)

    verify(gView.compareRect1Pos(0, 110));
    verify(gView.compareRect2Pos(0, 260));
}
