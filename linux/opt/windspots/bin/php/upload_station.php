<?php
$stationName = (isset($argv[1])) ? $argv[1] : "CHGE99";
$displayName = (isset($argv[2])) ? $argv[2] : "Station Dev";
$shortName = (isset($argv[3])) ? $argv[3] : "Station Dev";
$MSName = (isset($argv[4])) ? $argv[4] : "NA";
$information = (isset($argv[5])) ? $argv[5] : "WindSpots Dev - Please contact Denis";
$spotType = (isset($argv[6])) ? $argv[6] : "4";
$online = (isset($argv[7])) ? $argv[7] : "Y";
$maintenance = (isset($argv[8])) ? $argv[8] : "N";
$reason = (isset($argv[9])) ? $argv[9] : "NA";
$altitude = (isset($argv[10])) ? $argv[10] : "376";
$latitude = (isset($argv[11])) ? $argv[11] : "46.20199";
$longitude = (isset($argv[12])) ? $argv[12] : "6.15921";
$gmt = (isset($argv[13])) ? $argv[13] : "1";
$url = (isset($argv[14])) ? $argv[14] : "http://station.windspots.com";
$windspotsLog =  (isset($argv[15])) ? $argv[15] : "/var/log";
//
$registerURL = $url . "/register.php";
$wscheck="ws100";
//
function microtime_float() {  
  list($usec, $sec) = explode(" ", microtime());
  return ((float)$usec + (float)$sec);
}
//
function logIt($message) {
  global $windspotsLog;
  $dirname = dirname(__FILE__);
  $wlHandle = fopen($windspotsLog."/windspots.log", "a");
  $t = microtime(true);
  $micro = sprintf("%06d",($t - floor($t)) * 1000000);
  $micro = substr($micro,0,3);
  fwrite($wlHandle, Date("H:i:s").".$micro"." upload_station.php - ".$message."\n");
  fclose($wlHandle);
}
//
$current_date = Date("YmdHis");
if($online == "N")
	$online = 0;
else
	$online = 1;
if($maintenance == "N")
	$maintenance = 0;
else
	$maintenance = 1;
// station 
$data = array('station' => $stationName, 'date' => $current_date, 'stationName' => $stationName, 'displayName' => $displayName, 
                  'shortName' => $shortName, 'MSName' => $MSName, 'information' => $information,
                  'spotType' => $spotType, 'online' => $online, 'maintenance' => $maintenance, 'reason' => $reason, 
                  'altitude' => $altitude, 'latitude' => $latitude, 'longitude' => $longitude, 'gmt' => $gmt,
                  'check' => $wscheck);
// upload data
//logIt("Station: ".$stationName);
//logIt("Date:    ".$current_date);
//logIt("Data:    ".json_encode($data));
//logIt("Data:    ".json_encode($stationData));
$headers = array();
$headers[] = "Accept: application/json";
$headers[] = "Content-Type: application/x-www-form-urlencoded";
$headers[] = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36";
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $registerURL);
curl_setopt($ch, CURLOPT_POST, true);
curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($data));
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
$result = curl_exec($ch);
if (curl_errno($ch)) {
  logIt("Upload Data Error: ".curl_error($ch));
  logIt("                   ".$result);
} else {
  logIt("Transmit done for: ".$registerURL);
}
curl_close ($ch);
?>