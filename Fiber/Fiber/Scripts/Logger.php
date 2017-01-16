<?php

class Logger {

    public $filename = "";
    public $logfile = false;

    function __construct($fName = "PHPLog.txt") {
        $this->fileName = $fName;
        $this->logfile = fopen($fName, "a+");
    }
    
    function __destruct() {
        if($this->logfile)
            fclose($this->logfile);
    }

    function log($message) {
        $formattedMessage = "[" . date('d-m-Y H:i:s') . "] " . $message . "\n";
        fwrite($this->logfile, $formattedMessage);
    }
    
}

?>