<?php
$station = (isset($argv[1])) ? $argv[1] : "CHGE99";
$name = (isset($argv[2])) ? $argv[2] : "Station Dev";
$altitude = (isset($argv[3])) ? $argv[3] : "376";
$latitude = (isset($argv[4])) ? $argv[4] : "46.20199";
$longitude = (isset($argv[5])) ? $argv[5] : "6.15921";
$stationURL = (isset($argv[6])) ? $argv[6] : "http://station.windspots.com";
$statusURL =  (isset($argv[7])) ? $argv[7] : "http://status.windspots.org";
$windspotsTmp =  (isset($argv[8])) ? $argv[8] : "/var/tmp";
$windTagFile =  (isset($argv[9])) ? $argv[9] : $windspotsTmp.'/windtag.txt';
$lastTransmission =  (isset($argv[10])) ? $argv[10] : $windspotsTmp.'/lasttransmission';
$imageAge =  (isset($argv[11])) ? $argv[11] : "9999";
$windspotsLog =  (isset($argv[12])) ? $argv[12] : "/var/log";
$version =  (isset($argv[13])) ? $argv[13] : "20220101";
//
$wscheck = "ws100";
$stationURL = $stationURL . "/data.php";
$statusURL = $statusURL . "/station_report.php";
$compass = array(
  "N", "NNE", "NE", "ENE",
  "E", "ESE", "SE", "SSE",
  "S", "SSW", "SW", "WSW",
  "W", "WNW", "NW", "NNW"
);
//
function microtime_float() {  
  list($usec, $sec) = explode(" ", microtime());
  return ((float)$usec + (float)$sec);
}
function logIt($message) {
  global $windspotsLog;
  $dirname = dirname(__FILE__);
  $wlHandle = fopen($windspotsLog."/windspots.log", "a");
  $t = microtime(true);
  $micro = sprintf("%06d",($t - floor($t)) * 1000000);
  $micro = substr($micro,0,3);
  fwrite($wlHandle, Date("H:i:s").".$micro"." upload_data.php - ".$message."\n");
  fclose($wlHandle);
}
$newTime = strtotime('-1 minutes');
$lastUpload = Date("Y-m-d H:i:s", $newTime);
$current_date = Date("YmdHis");
// logIt("Send Weather Data ".$station);
$weatherdata = array();
$status_dir = 0;
$status_speed = 0;
$status_speedAverage = 0;
$status_temperature = 0;
$status_barometer = 0;
$status_battery = 0;
try {
  $db = new SQLite3($windspotsTmp.'/ws.db');
  $SQLnow = Date("Y-m-d H:i:s");
  // logIt("Now: ".$SQLnow." last: ".$lastUpload."");
  // get data
  $stmt = $db->prepare("SELECT * FROM data WHERE last_update >= :last_update AND last_update <= :SQLnow ORDER BY sensor_id DESC");
  $stmt->bindValue(':last_update', $lastUpload);
  $stmt->bindValue(':SQLnow', $SQLnow);
  $result = $stmt->execute();
  $nbrec = 0;
  while($row = $result->fetchArray(SQLITE3_ASSOC)) {
    // $withComma = implode(",", $row);
    // logIt("Weather Data row: ".$withComma);
    if(++$nbrec >= 90) // to limit post
      break;
    if(($row['wind_speed'] > 99) || ($row['wind_speed_average'] > 99)) { // wrong > 356 km/h
      $row['wind_direction'] = 0;
      $row['wind_speed'] = 0;
      $row['wind_speed_average'] = 0;
    }
    $weatherdata[] = $row;
    $dir = $row['wind_direction'];
    $speed = $row['wind_speed'];
    $speedAverage = $row['wind_speed_average'];
    if(!empty($dir)) {
      // write file for wind image tag
      if($dir != 0 || $speed !=0 || $speedAverage != 0) {
       // status
        $status_dir = $dir;
        $status_speed = $speed;
        $status_speedAverage = $speedAverage;
       // write file for wind image tag
       // logIt("Direction for Wind Tag: ".$dir);
        $dirAlpha = $compass[round($dir / 22.5) % 16];
        $itHandle = null;
        $itHandle = fopen($windTagFile, 'w+');
        fwrite($itHandle, " - ".$dirAlpha." ".round((($speed*3.6)/1.852),0)." knots (".round(($speed*3.6),0)." km/h) ");
       // logIt("Wind Tag Created: ".$dirAlpha);
        fclose($itHandle);
      }
    }
    // take last value for status
    if($row['temperature'] != 0) {
      $status_temperature = $row['temperature'];
    }
    if($row['barometer'] != 0) {
      $status_barometer = $row['barometer'];
    }
    if(strcmp($row['name'],"WS200") == 0) {
      if(isset($row['battery'])) {
        $status_battery = $row['battery'];
      }
    }
    // logIt("    row(".$nbrec.") : ".print_r($row));
  }
  logIt("Starting for: ".$nbrec." records");
  // logIt("        weatherdata:      ".json_encode($weatherdata));
  $db = null;
} catch(PDOException $e) {
  logIt("Unable to open database connection: ".print_r($e,true));
}
if($nbrec < 1) {
  logIt("RESTART PROCESS-WEATHER REQUIRED DUE TO NO RECORD TO UPLOAD.");
  $output = shell_exec('/opt/windspots/bin/process-weather.sh');
  // upload empty data for heart-beat
  $row = array();
  $row['barometer'] = 0;
  $row['temperature'] = 0;
  $row['wind_direction'] = 0;
  $row['wind_speed'] = 0;
  $row['wind_speed_average'] = 0;
  $row['battery'] = 0;
  $weatherdata[] = $row;
}
// upload data
$jsondata = json_encode($weatherdata);
$data = array('station' => $station, 'date' => $current_date, 'data' => $jsondata, 'check' => $wscheck);
// logIt("Station: ".$station);
// logIt("Date:    ".$current_date);
// logIt("Data:    ".json_encode($data));
$headers = array();
$headers[] = "Accept: application/json";
$headers[] = "Content-Type: application/x-www-form-urlencoded";
$headers[] = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36";
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $stationURL);
curl_setopt($ch, CURLOPT_POST, true);
curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($data));
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
$result = curl_exec($ch);
if (curl_errno($ch)) {
  logIt("Error Station: ".curl_error($ch));
  logIt("               ".$result);
} else {
  $output = shell_exec('touch '.$lastTransmission);
}
curl_close ($ch);
logIt("Ok for: ".$stationURL);
// legacy
$legacyURL="http://station.windspots.com/data_upload.php";
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $legacyURL);
curl_setopt($ch, CURLOPT_POST, true);
curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($data));
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
$result = curl_exec($ch);
if (curl_errno($ch)) {
  logIt("Error Legacy: ".curl_error($ch));
  logIt("              ".$result);
} 
curl_close ($ch);
logIt("Ok for: ".$legacyURL);
// status
$secondes = intval((Date("i")*166))+intval(Date("s"));
$station_date = Date("H").".".$secondes;
$statusdata = $station_date."\t".$status_dir."\t".$status_speed."\t".$status_speedAverage."\t".$status_temperature."\t".$status_barometer."\t".$status_battery."\t".$name."\t".$imageAge."\t".$version;
$location = $altitude."\t".$latitude."\t".$longitude;
$orgdata = array('station' => $station, 'date' => $current_date, 'data' => $statusdata, 'location' => $location);
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $statusURL);
curl_setopt($ch, CURLOPT_POST, true);
curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($orgdata));
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
$result = curl_exec($ch);
if (curl_errno($ch)) {
  logIt("Error Status: ".curl_error($ch));
  logIt("              ".$result);
}
curl_close ($ch);
logIt("Ok for: ".$statusURL);
// legacy status
$legacyStatusURL = 'http://old.windspots.com/station_report.php';
$legacyStatusWeatherData = $station_date."\t".$status_temperature."\t"."0"."\t"."0"."\t"."0"."\t"."0"."\t"."0"."\t".$status_barometer."\t".$status_barometer."\t".$status_dir."\t".$status_speed."\t".$status_dir."\t".$status_speedAverage."\t"."0"."\t"."0"."\t"."0"."\t".$status_battery;
$legacyStatusData = array('station' => $station.'1', 'date' => $current_date, 'data' => $legacyStatusWeatherData, 'forecast' => '', 'trend' => '');
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $legacyStatusURL);
curl_setopt($ch, CURLOPT_POST, true);
curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($legacyStatusData));
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
$result = curl_exec($ch);
if (curl_errno($ch)) {
  logIt("Error Legacy Status: ".curl_error($ch));
  logIt("                     ".$result);
} 
curl_close ($ch);
logIt("Ok for: ".$legacyStatusURL);
?>