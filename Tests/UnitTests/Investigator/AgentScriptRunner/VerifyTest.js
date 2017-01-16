function test_verify() {
    log("verify #1")
    verify(10 === 10)
    log("verify #2")
    verify(10 === "10")
    log("verify #3")
    verify("10" === 10)
    log("verify #4")
    verify("a" === "b")
    log("done")
}
