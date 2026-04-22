<?php
// Send weather station data to WindSpots endpoints.

const MAX_WIND_SPEED = 50.0;
const CURL_TIMEOUT = 20;
const DB_BUSY_TIMEOUT_MS = 1000;

// Retrieve command line arguments with default values.
$station         = $argv[1]  ?? "CHGE99";
$name            = $argv[2]  ?? "Station Dev";
$altitude        = $argv[3]  ?? "376";
$latitude        = $argv[4]  ?? "46.20199";
$longitude       = $argv[5]  ?? "6.15921";
$stationURL      = rtrim($argv[6] ?? "http://station.windspots.com", '/');
$statusURL       = rtrim($argv[7] ?? "http://status.windspots.org", '/');
$windspotsTmp    = $argv[8]  ?? "/var/tmp";
$windTagFile     = $argv[9]  ?? $windspotsTmp . '/windtag.txt';
$lastTransmission = $argv[10] ?? $windspotsTmp . '/lasttransmission';
$imageAge        = $argv[11] ?? "9999";
$windspotsLog    = $argv[12] ?? "/var/log";
$version         = $argv[13] ?? "20220101";

// Endpoint URLs.
$wscheck         = "ws100";
$stationURL      .= "/data.php";
$statusURL       .= "/station_report.php";
$legacyURL        = "http://station.windspots.com/data_upload.php";
$legacyStatusURL  = "http://old.windspots.com/station_report.php";

// Compass directions for wind.
$compass = [
    "N", "NNE", "NE", "ENE",
    "E", "ESE", "SE", "SSE",
    "S", "SSW", "SW", "WSW",
    "W", "WNW", "NW", "NNW"
];

// Logging function using file locking.
function logIt($message) {
    global $windspotsLog;
    $logFile = $windspotsLog . "/windspots.log";
    $time = microtime(true);
    $micro = sprintf("%03d", ($time - floor($time)) * 1000);
    $logMessage = date("H:i:s") . ".$micro upload_data.php - $message" . PHP_EOL;
    file_put_contents($logFile, $logMessage, FILE_APPEND | LOCK_EX);
}

// Function to send data via cURL.
function sendToEndpoint($url, $data) {
    global $lastTransmission;
    $headers = [
        "Accept: application/json",
        "Content-Type: application/x-www-form-urlencoded",
        "User-Agent: Mozilla/5.0"
    ];
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_POST, true);
    curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($data));
    curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 10);
    curl_setopt($ch, CURLOPT_TIMEOUT, CURL_TIMEOUT);
    $result = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    if (curl_errno($ch)) {
        logIt("cURL error for $url: " . curl_error($ch));
        logIt("Response: " . $result);
    } elseif ($httpCode !== 200) {
        logIt("HTTP response code for $url: $httpCode");
    } else {
        logIt("Success sending to $url");
        @touch($lastTransmission);
    }
    curl_close($ch);
    return $result;
}

// Determine time range for data collection.
$newTime    = strtotime('-1 minutes');
$lastUpload = date("Y-m-d H:i:s", $newTime);
$current_date = date("YmdHis");

// Status variables initialization.
$status_dir = $status_speed = $status_speedAverage = 0;
$status_temperature = $status_barometer = $status_battery = 0;

$weatherdata = [];
$nbrec = 0;
$previousRow = null;

$dbPath = $windspotsTmp . '/ws.db';
if (!file_exists($dbPath)) {
    logIt("Database file not found: $dbPath");
} else {
    try {
        $db = new SQLite3($dbPath);
        $db->busyTimeout(DB_BUSY_TIMEOUT_MS);
        $SQLnow = date("Y-m-d H:i:s");
        $stmt = $db->prepare(
            "SELECT last_update, name, battery, temperature, barometer, wind_direction, wind_speed, wind_speed_average
             FROM data
             WHERE last_update BETWEEN :last_update AND :SQLnow
             ORDER BY sensor_id DESC
             LIMIT 90"
        );
        $stmt->bindValue(':last_update', $lastUpload);
        $stmt->bindValue(':SQLnow', $SQLnow);
        $result = $stmt->execute();
        while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
            $nbrec++;
            if ($row['wind_speed'] > MAX_WIND_SPEED || $row['wind_speed_average'] > MAX_WIND_SPEED) {
                if ($previousRow !== null) {
                    $nextRow = $result->fetchArray(SQLITE3_ASSOC);
                    if ($nextRow && $nextRow['wind_speed'] <= MAX_WIND_SPEED && $nextRow['wind_speed_average'] <= MAX_WIND_SPEED) {
                        $row['wind_speed'] = ($previousRow['wind_speed'] + $nextRow['wind_speed']) / 2;
                        $row['wind_speed_average'] = ($previousRow['wind_speed_average'] + $nextRow['wind_speed_average']) / 2;
                    } else {
                        $row['wind_speed'] = $previousRow['wind_speed'];
                        $row['wind_speed_average'] = $previousRow['wind_speed_average'];
                    }
                }
            } else {
                $previousRow = $row;
            }
            $weatherdata[] = $row;

            // Update wind tag file if directional data is present.
            $dir = $row['wind_direction'] ?? 0;
            $speed = $row['wind_speed'] ?? 0;
            $speedAverage = $row['wind_speed_average'] ?? 0;
            if (!empty($dir) && ($dir != 0 || $speed != 0 || $speedAverage != 0)) {
                $status_dir = $dir;
                $status_speed = $speed;
                $status_speedAverage = $speedAverage;
                $dirIndex = round($dir / 22.5) % 16;
                $dirAlpha = $compass[$dirIndex];
                $windTagMessage = " - " . $dirAlpha . " " . round((($speed * 3.6) / 1.852), 0) . " knots (" . round(($speed * 3.6), 0) . " km/h) ";
                file_put_contents($windTagFile, $windTagMessage);
            }
            if (!empty($row['temperature'])) {
                $status_temperature = $row['temperature'];
            }
            if (!empty($row['barometer'])) {
                $status_barometer = $row['barometer'];
            }
            if (isset($row['name']) && $row['name'] === "WS200" && isset($row['battery'])) {
                $status_battery = $row['battery'];
            }
        }
        logIt("Processed $nbrec records from database.");
        $db->close();
    } catch (Exception $e) {
        logIt("Database error: " . $e->getMessage());
    }
}

if ($nbrec < 1) {
	try {
    $db = new SQLite3($windspotsTmp.'/ws.db');
    $db->busyTimeout(DB_BUSY_TIMEOUT_MS);
    $lastRowResult = $db->query("SELECT last_update FROM data ORDER BY last_update DESC LIMIT 1");
    $lastRow = $lastRowResult ? $lastRowResult->fetchArray(SQLITE3_ASSOC) : null;
    $lastSeenUpdate = ($lastRow && isset($lastRow['last_update'])) ? $lastRow['last_update'] : 'NONE';
    logIt("NO RECORDS IN WINDOW [".$lastUpload." -> ".$SQLnow."] - latest db row=".$lastSeenUpdate);
    $db->close();
  } catch (Exception $e) {
    logIt("FAILED TO READ LAST DB ROW BEFORE RESTART: ".print_r($e, true));
  }
  logIt("RESTART PROCESS-WEATHER REQUIRED DUE TO NO RECORD TO UPLOAD.");
  $output = shell_exec('/opt/windspots/bin/process-weather.sh 2>&1');
  if(!empty($output)) {
    logIt("process-weather output: ".trim($output));
  }
    // Add a heartbeat record.
    $weatherdata[] = [
        'barometer'         => 0,
        'temperature'       => 0,
        'wind_direction'    => 0,
        'wind_speed'        => 0,
        'wind_speed_average'=> 0,
        'battery'           => 0
    ];
}

$jsondata = json_encode($weatherdata);
$data = [
    'station' => $station,
    'date'    => $current_date,
    'data'    => $jsondata,
    'check'   => $wscheck
];

// Send weather data to primary and legacy endpoints.
sendToEndpoint($stationURL, $data);
sendToEndpoint($legacyURL, $data);

// Prepare and send status data.
$secondes = intval(date("i")) * 166 + intval(date("s"));
$station_date = date("H") . "." . $secondes;
$statusdata = implode("\t", [
    $station_date,
    $status_dir,
    $status_speed,
    $status_speedAverage,
    $status_temperature,
    $status_barometer,
    $status_battery,
    $name,
    $imageAge,
    $version
]);
$location = implode("\t", [$altitude, $latitude, $longitude]);
$orgdata = [
    'station'  => $station,
    'date'     => $current_date,
    'data'     => $statusdata,
    'location' => $location
];
sendToEndpoint($statusURL, $orgdata);

// Prepare and send legacy status data.
$legacyStatusWeatherData = implode("\t", [
    $station_date,
    $status_temperature,
    "0", "0", "0", "0", "0",
    $status_barometer,
    $status_barometer,
    $status_dir,
    $status_speed,
    $status_dir,
    $status_speedAverage,
    "0", "0", "0",
    $status_battery
]);
$legacyStatusData = [
    'station'  => $station . '1',
    'date'     => $current_date,
    'data'     => $legacyStatusWeatherData,
    'forecast' => '',
    'trend'    => ''
];
sendToEndpoint($legacyStatusURL, $legacyStatusData);
?>
