<?php
header('Content-type: application/json');

function parse_ini_output($output) {
    $parsed = parse_ini_string((string) $output);
    return is_array($parsed) ? $parsed : array();
}

function read_operstate($iface) {
    return (string) shell_exec('cat /sys/class/net/' . escapeshellarg($iface) . '/operstate 2>/dev/null');
}

function read_ipv4($iface) {
    return trim((string) shell_exec('/sbin/ip -4 addr show ' . escapeshellarg($iface) . " 2>/dev/null | awk '/inet / {print \$2}' | cut -d/ -f1 | head -n 1"));
}

function detect_ppp_iface() {
    $candidates = array();

    foreach (array('/etc/network/interfaces', __DIR__ . '/../../../../interfaces') as $interfacesFile) {
        if (!is_readable($interfacesFile)) {
            continue;
        }

        $lines = @file($interfacesFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        if (!is_array($lines)) {
            continue;
        }

        foreach ($lines as $line) {
            if (preg_match('/^\s*iface\s+(eth1|usb0)\s+/', $line, $matches)) {
                return $matches[1];
            }
        }
    }

    foreach (array('eth1', 'usb0') as $iface) {
        if (file_exists('/sys/class/net/' . $iface . '/operstate')) {
            return $iface;
        }
    }

    return 'eth1';
}

$values = array();

// power values
$alim = parse_ini_output(shell_exec('/opt/windspots/bin/cpp/WS200/getalim/getalim 2>/dev/null'));
$values = array_merge($values, $alim);

// i2c probes
$i2cdetect = shell_exec('/usr/sbin/i2cdetect -y 1 2>/dev/null');
foreach (array('40', '41', '43', '48', '77') as $addr) {
    $pattern = '/\s' . $addr . '(\s|$)/'; 
    $values['I2C' . $addr] = preg_match($pattern, $i2cdetect) ? '1' : '0';
}

// sensors
$windspots_ini = @parse_ini_file('/opt/windspots/etc/main');
$altitude = (is_array($windspots_ini) && isset($windspots_ini['ALTITUDE'])) ? $windspots_ini['ALTITUDE'] : '0';
$bmp280 = parse_ini_output(shell_exec('/opt/windspots/bin/cpp/WS200/getbaro/getbaro -a ' . escapeshellarg((string) $altitude) . ' 2>/dev/null'));
$ads1015 = parse_ini_output(shell_exec('/opt/windspots/bin/cpp/WS200/gettemp/gettemp 2>/dev/null'));

$anemoOutput = (string) shell_exec('/opt/windspots/bin/cpp/WS200/getanemo/getanemo 2>/dev/null');
$anemoLines = preg_split('/\r?\n/', trim($anemoOutput));
$anemo1 = isset($anemoLines[0]) ? parse_ini_output($anemoLines[0]) : array();
$anemo0 = isset($anemoLines[1]) ? parse_ini_output($anemoLines[1]) : array();

// cellular status
$hilink = parse_ini_output(shell_exec('/opt/windspots/bin/hi-status.sh 2>/dev/null'));
$values['SIGNALICON'] = isset($hilink['SIGNALICON']) ? $hilink['SIGNALICON'] : '';
$values['MAXSIGNAL'] = isset($hilink['MAXSIGNAL']) ? $hilink['MAXSIGNAL'] : '';
$values['IPADDRESS'] = isset($hilink['IPADDRESS']) ? $hilink['IPADDRESS'] : '';
$values['WORKMODE'] = isset($hilink['WORKMODE']) ? $hilink['WORKMODE'] : '';
$values['FULLNAME'] = isset($hilink['FULLNAME']) ? $hilink['FULLNAME'] : '';

// sensor fields used by UI
$values['PRESSURE'] = isset($bmp280['PRESSURE']) ? $bmp280['PRESSURE'] : '';
$values['SEALEVEL'] = isset($bmp280['SEALEVEL']) ? $bmp280['SEALEVEL'] : '';
$values['INBOXTEMP'] = isset($bmp280['TEMPERATURE']) ? $bmp280['TEMPERATURE'] : '';
$values['TEMPERATURE'] = isset($ads1015['TEMPERATURE']) ? $ads1015['TEMPERATURE'] : '';
$values['DIRECTION'] = isset($anemo0['DIRECTION']) ? $anemo0['DIRECTION'] : '';
$values['SPEED'] = isset($anemo1['SPEED']) ? $anemo1['SPEED'] : '';

// display log
$logFile = '/opt/windspots/log/windspots.log';
$output = shell_exec("tail -n 9 " . escapeshellarg($logFile) . " 2>&1");
$values['LOG'] = htmlspecialchars($output);

// network
$lan = read_operstate('eth0');
$wlan = read_operstate('wlan0');
$pppIface = detect_ppp_iface();
$ppp = read_operstate($pppIface);
$lanIP = read_ipv4('eth0');
$wlanIP = read_ipv4('wlan0');

$values['lan'] = (strncmp($lan, 'up', 2) === 0) ? '1' : '0';
$values['lanIP'] = (strncmp($lan, 'up', 2) === 0 && $lanIP !== '') ? $lanIP : '0.0.0.0';
$values['wlan'] = (strncmp($wlan, 'up', 2) === 0) ? '1' : '0';
$values['wlanIP'] = (strncmp($wlan, 'up', 2) === 0 && $wlanIP !== '') ? $wlanIP : '0.0.0.0';
$values['ppp'] = (strncmp($ppp, 'up', 2) === 0) ? '1' : '0';
$values['pppIP'] = (strncmp($ppp, 'up', 2) === 0 && isset($hilink['IPADDRESS']) && $hilink['IPADDRESS'] !== '') ? $hilink['IPADDRESS'] : '0.0.0.0';

// ssid + image
$values['ssid'] = shell_exec('/usr/sbin/iwgetid -r 2>/dev/null');
$values['image'] = shell_exec('ls /var/tmp/img/ 2>/dev/null');

$json = json_encode($values);
if ($json === false) {
    echo json_encode(array('error' => 'Unable to encode infos payload'));
    exit;
}

echo $json;
?>
