function test_objectWrap() {
    var calendar = object("Application.ControlsWindow/calendarWidget");
    log(calendar.objectName)

    var parent = calendar.parent
    log(parent.objectName)

    var parent2 = parent.parent
    log(parent2.objectName)

    var tabWidget = object("Application.ControlsWindow/tabWidget")
    log(tabWidget.objectName)

    var calendar2 = tabWidget.object("tab2/calendarWidget")
    log(calendar2.objectName)

    compare(calendar2, calendar, "Member function object() doesnt work")
}
