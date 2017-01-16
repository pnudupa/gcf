<?php

function initialize() {
    error_reporting(E_ALL);
    ini_set('display_errors','Off');
    ini_set('log_errors','On');
    ini_set('html_errors','Off');
    
    global $logger;
    global $os;
    
    $logFilePath = "";
    $errorLogFilePath = "";
    $os = php_uname('s');
    
    if(strpos($os, "Windows") !== false) {
        set_time_limit(0);
        $logFilePath = sys_get_temp_dir() . "\\" . "FiberPHPLog.txt";
        $errorLogFilePath = sys_get_temp_dir() . "\\" . "FiberPHPErrorLog.txt";
    }
    else {
        $logFilePath = sys_get_temp_dir() . "/" . "FiberPHPLog.txt";
        $errorLogFilePath = sys_get_temp_dir() . "/" . "FiberPHPErrorLog.txt";
    }
    
    ini_set('error_log', $errorLogFilePath);
    $logger = new Logger($logFilePath);
}

function postReceiveRequest() {
    global $fiberServerSocket;
    global $os;
    
    if(strpos($os, "Windows") !== false)
        $fiberServerSocket = "\\\.\pipe\Fiber";
    else if(strpos($os, "Linux") !== false)
        $fiberServerSocket = "/tmp/Fiber";
    else if(strpos($os, "Darwin") !== false)
        $fiberServerSocket = "/tmp/Fiber";
    else
        $fiberServerSocket = "/tmp/Fiber";
}

function postProcessRequest() {
}

function postSendResponse() {
}

function finish() {
}

?>