<?php

function contact_server() {
    global $jsonObj;
    global $logger;
    
    if(array_key_exists('requestType', $jsonObj)) {
        $requestType = $jsonObj['requestType'];
        if($requestType === 'SESSION_MGMT') {
            return manage_session();
        } else if($requestType === 'SERVICE_REQUEST') {
            return invoke_service();
        }
    } 
    
    echo errorMessageToJSON("Invalid request message.");
}

function manage_session() {
    global $jsonObj;
    global $logger;
    global $clientIP;
    global $fiberServerSocket;
    global $response;
    
    $sessionReqMsg = json_encode($jsonObj, JSON_FORCE_OBJECT);
    $logger->log("Requesting : " . $sessionReqMsg);
    $response = request_to_local_server($fiberServerSocket, $sessionReqMsg);
    postProcessRequest();
    $responseObj = json_decode($response, true);
    if(array_key_exists('error', $responseObj)) {
        if( ($responseObj['error'] === "Couldn't communicate with the session handler") ||
            ($responseObj['error'] === "Couldn't connect to the socket of the session handler") ) {
            echo errorMessageToJSON("Couldn't communicate with Fiber. See server error log.");
            return;
        }
    }
        
    echo $response;
}

function invoke_service() {
    global $jsonObj;
    global $logger;
    global $response;
    
    $handlerName = get_handler();
    if($handlerName === '') {
        echo errorMessageToJSON("Couldn't communicate with Fiber. See server error log.");
        postProcessRequest();
        return;
    }
        
    $message = json_encode($jsonObj) . "<-FIBERFOOTER->";
    
    $logger->log("Contacting server"); // with : " . $message);
    $response = request_to_local_server($handlerName, $message);
    postProcessRequest();
    if($response === '')
        $response = errorMessageToJSON("Fiber hung up with out any response");
    $logger->log("Responding to client"); // with : " . $response);
    echo $response;
}

function get_handler() {
    global $fiberServerSocket;
    global $jsonObj;
    global $logger;

    $sessionName = get_session_name();
    $sessionType = get_session_type();
    $handlerReqObj = array('requestType' => "HANDLER",
                             'sessionName' => $sessionName,
                             'sessionType' => $sessionType);
    $handlerReqMsg = json_encode($handlerReqObj, JSON_FORCE_OBJECT);
    
    $result = request_to_local_server($fiberServerSocket, $handlerReqMsg);
    $resultObj = json_decode($result, true);
    $handlerName = "";
    if(array_key_exists('handlerName', $resultObj))
        $handlerName = $resultObj['handlerName'];
    if(array_key_exists('error', $resultObj))
        trigger_error("Querying for handler returned error {" . $resultObj['error'] . "}");
    
    return $handlerName;
}

function get_session_name() {
    global $jsonObj;
    $sessionName = 'NULL';
    if(array_key_exists('sessionName', $jsonObj)) {
        $sessionName = $jsonObj['sessionName'];
        if($sessionName == '')
            $sessionName = 'NULL';
    }
    
    return $sessionName;
}

function get_session_type() {
    global $jsonObj;
    $sessionType = "CORE";
    if(array_key_exists('sessionType', $jsonObj))
        $sessionType = $jsonObj['sessionType'];
    return $sessionType;
}

?>