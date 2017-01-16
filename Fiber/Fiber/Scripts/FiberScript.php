<?php

include "Logger.php";
include "FiberScriptUtils.php";
include "FiberAccessMethods.php";
include "FiberAccessHooks.php";

$fiberServerSocket = "";
$logger = "";
$os = "";

initialize();

$clientIP = $_SERVER['REMOTE_ADDR'];
$clientAgent = $_SERVER['HTTP_USER_AGENT'];

$logger->log( "Request initiated from : " . $clientIP . " [" . $clientAgent . "]");

if(isset($HTTP_RAW_POST_DATA)) {
    $jsonObj = json_decode($HTTP_RAW_POST_DATA, true);
    $response = "";

    if($jsonObj) {
        $jsonObj['clientIP'] = $clientIP;
        postReceiveRequest();
        contact_server();
        postSendResponse();
    } else {
        $logger->log("Invalid data from " . $clientIP . " : " . $HTTP_RAW_POST_DATA);
        echo errorMessageToJSON("Invalid request message format. Expected JSON.");
    }
} else {
    $logger->log("Illegal attempt to contact Fiber by " . $clientIP);
    echo errorMessageToJSON("Illegal attempt to contact Fiber. This will be reported!");
}

$logger->log( "Response sent to : " . $clientIP);

finish();
    
?>
