function test_compare() {
    log("compare #1")
    compare(10, 10)
    log("compare #2")
    compare(10, "10")
    log("compare #3")
    compare("10", 10)
    log("compare #4")
    compare("a", "b")
    log("done")
}
