<?php
// Improved station registration script.

// Retrieve command line arguments with defaults.
$stationName  = $argv[1]  ?? "CHGE99";
$displayName  = $argv[2]  ?? "Station Dev";
$shortName    = $argv[3]  ?? "Station Dev";
$MSName       = $argv[4]  ?? "NA";
$information  = $argv[5]  ?? "WindSpots Dev - Please contact Denis";
$spotType     = $argv[6]  ?? "4";
$online       = $argv[7]  ?? "Y";
$maintenance  = $argv[8]  ?? "N";
$reason       = $argv[9]  ?? "NA";
$altitude     = $argv[10] ?? "376";
$latitude     = $argv[11] ?? "46.20199";
$longitude    = $argv[12] ?? "6.15921";
$gmt          = $argv[13] ?? "1";
$url          = rtrim($argv[14] ?? "http://station.windspots.com", '/');
$windspotsLog = $argv[15] ?? "/var/log";

$registerURL = $url . "/register.php";
$wscheck = "ws100";

// Logging function.
function logIt($message) {
    global $windspotsLog;
    $logFile = $windspotsLog . "/windspots.log";
    $time = microtime(true);
    $micro = sprintf("%03d", ($time - floor($time)) * 1000);
    $logMessage = date("H:i:s") . ".$micro upload_station.php - $message" . PHP_EOL;
    file_put_contents($logFile, $logMessage, FILE_APPEND | LOCK_EX);
}

$current_date = date("YmdHis");

// Normalize online and maintenance flags.
$online = ($online === "N") ? 0 : 1;
$maintenance = ($maintenance === "N") ? 0 : 1;

// Build the data array for registration.
$data = [
    'station'     => $stationName,
    'date'        => $current_date,
    'stationName' => $stationName,
    'displayName' => $displayName,
    'shortName'   => $shortName,
    'MSName'      => $MSName,
    'information' => $information,
    'spotType'    => $spotType,
    'online'      => $online,
    'maintenance' => $maintenance,
    'reason'      => $reason,
    'altitude'    => $altitude,
    'latitude'    => $latitude,
    'longitude'   => $longitude,
    'gmt'         => $gmt,
    'check'       => $wscheck
];

// Send registration data using cURL.
$headers = [
    "Accept: application/json",
    "Content-Type: application/x-www-form-urlencoded",
    "User-Agent: Mozilla/5.0"
];
$ch = curl_init($registerURL);
curl_setopt($ch, CURLOPT_POST, true);
curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($data));
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
$result = curl_exec($ch);
if (curl_errno($ch)) {
    logIt("cURL error: " . curl_error($ch));
    logIt("Response: " . $result);
} else {
    logIt("Registration successful for: $registerURL");
}
curl_close($ch);
?>
