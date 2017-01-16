<?php 

function request_to_local_server($localPipeName, $message) {
    $os = php_uname('s');
    if(strpos($os, "Windows") !== false)
        return request_to_local_pipe($localPipeName, $message);
    else if(strpos($os, "Linux") !== false)
        return request_to_local_socket($localPipeName, $message);
    else
        return json_encode(array('Result' => "", 'Error' => "Unrecognized Server OS"), JSON_FORCE_OBJECT);
}
    
function request_to_local_pipe($localPipeName, $message) {
    
    global $logger;
    
    if($localPipeName == false) {
        $response = array();
        $response["error"] = "Invalid pipe name specified for connection";
        return json_encode($response, JSON_FORCE_OBJECT);
    }

    $logger->log("About to send request to session " . $localPipeName);
        
    // $pipe = fopen('\\\.\\pipe\\' . $localPipeName, 'r+');
    $pipe = fopen($localPipeName, 'r+');
    if(!$pipe)
    {
        $response = array();
        $response["error"] = "Couldn't communicate with the session handler";
        return json_encode($response, JSON_FORCE_OBJECT);
    }
    
    stream_set_blocking($pipe, 1);
    fwrite($pipe, $message);

    $logger->log("Finished sending request to session " . $localPipeName);
    
    $response = "";
    while(1) {
        $input = fread($pipe, 4096);
        if( strlen($input) == 0 )
            break;
        $response = $response . $input;
    }
    fclose($pipe);
    
    $logger->log("Finished fetching response from session " . $localPipeName);
    
    return $response;
}

function request_to_local_socket($localSocketName, $message) {

    $response = "";
    
    if($localSocketName == false) {
        $response = array();
        $response["error"] = "Invalid socket name specified for connection";
        return json_encode($response, JSON_FORCE_OBJECT);
    }

    global $logger;
    $socket = socket_create(AF_UNIX, SOCK_STREAM, 0);

    if($socket == false) {
        $response = array();
        $response["error"] = "Couldn't create socket for the session handler";
        return json_encode($response, JSON_FORCE_OBJECT);
    } else {
        $result = socket_connect($socket, $localSocketName, 0);
        if($result == false) {
            $response = array();
            $response["error"] = "Couldn't connect to the socket of the session handler";
            return json_encode($response, JSON_FORCE_OBJECT);
        } else {
        
            $logger->log("About to send request to " . $localSocketName);
            socket_write($socket, $message, strlen($message));
            $logger->log("Finished sending request to " . $localSocketName);
            
            while(1) {
                $out = socket_read($socket, 4096);
                if( strlen($out) == 0 || socket_last_error($socket) )
                    break;

                $response .= $out;
            }
            $logger->log("Finished fetching response from " . $localSocketName);
        }
    }
    
    return $response;
}

function errorMessageToJSON($message) {
    $response = array('success' => false,
                      'result' => "",
                      'error' => $message);
    return json_encode($response, JSON_FORCE_OBJECT);
}

?>
