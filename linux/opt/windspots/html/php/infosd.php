<?php
header('Content-type: application/json');

function run_command($cmd) {
    return trim((string) shell_exec($cmd));
}

function parse_ini_output($output) {
    $parsed = parse_ini_string((string) $output);
    return is_array($parsed) ? $parsed : array();
}

$values = array();

// alim
$alim = parse_ini_output(shell_exec('/opt/windspots/bin/cpp/WS200/getalim/getalim'));
$values = array_merge($values, $alim);

// i2c probes
$i2cdetect = (string) shell_exec('i2cdetect -y 1');
foreach (array('40', '41', '43', '48', '77') as $addr) {
    $values['I2C' . $addr] = (strpos($i2cdetect, $addr . ' ') !== false) ? '1' : '0';
}

// sensors
$windspots_ini = parse_ini_file('/opt/windspots/etc/main');
$altitude = (is_array($windspots_ini) && isset($windspots_ini['ALTITUDE'])) ? $windspots_ini['ALTITUDE'] : '0';
$bmp280 = parse_ini_output(shell_exec('/opt/windspots/bin/cpp/WS200/getbaro/getbaro -a ' . $altitude));
$ads1015 = parse_ini_output(shell_exec('/opt/windspots/bin/cpp/WS200/gettemp/gettemp'));

$anemo = array();
exec('sudo /opt/windspots/bin/cpp/WS200/getanemo/getanemo', $anemo);
$anemo1 = isset($anemo[0]) ? parse_ini_output($anemo[0]) : array();
$anemo0 = isset($anemo[1]) ? parse_ini_output($anemo[1]) : array();

// hilink
$hilink = parse_ini_output(shell_exec('/opt/windspots/bin/hi-status.sh'));
$values['SIGNALICON'] = isset($hilink['SIGNALICON']) ? $hilink['SIGNALICON'] : '';
$values['MAXSIGNAL'] = isset($hilink['MAXSIGNAL']) ? $hilink['MAXSIGNAL'] : '';
$values['IPADDRESS'] = isset($hilink['IPADDRESS']) ? $hilink['IPADDRESS'] : '';
$values['WORKMODE'] = isset($hilink['WORKMODE']) ? $hilink['WORKMODE'] : '';
$values['FULLNAME'] = isset($hilink['FULLNAME']) ? $hilink['FULLNAME'] : '';

// sensor values
$values['PRESSURE'] = isset($bmp280['PRESSURE']) ? $bmp280['PRESSURE'] : '';
$values['SEALEVEL'] = isset($bmp280['SEALEVEL']) ? $bmp280['SEALEVEL'] : '';
$values['INBOXTEMP'] = isset($bmp280['TEMPERATURE']) ? $bmp280['TEMPERATURE'] : '';
$values['TEMPERATURE'] = isset($ads1015['TEMPERATURE']) ? $ads1015['TEMPERATURE'] : '';
$values['DIRECTION'] = isset($anemo0['DIRECTION']) ? $anemo0['DIRECTION'] : '';
$values['SPEED'] = isset($anemo1['SPEED']) ? $anemo1['SPEED'] : ''; // speed not working due to WiringPI ISR

// display log
$linecount = 9;
$file = '/opt/windspots/log/windspots.log';
$lines = array();
if (is_readable($file)) {
    $logLines = @file($file, FILE_IGNORE_NEW_LINES);
    if (is_array($logLines)) {
        $lines = array_slice($logLines, -$linecount);
    }
}
$values['LOG'] = implode('<br>', $lines);

// network
$lan = run_command('cat /sys/class/net/eth0/operstate 2>/dev/null');
$wlan = run_command('cat /sys/class/net/wlan0/operstate 2>/dev/null');
$ppp = run_command('cat /sys/class/net/eth1/operstate 2>/dev/null');
$lanIP = run_command("/sbin/ifconfig eth0 2>/dev/null | grep 'inet' | cut -d: -f2 | awk '{ print $2}'");
$wlanIP = run_command("/sbin/ifconfig wlan0 2>/dev/null | grep 'inet' | cut -d: -f2 | awk '{ print $2}'");

if (strncmp($lan, 'up', 2) === 0) {
    $values['lan'] = '1';
    $values['lanIP'] = $lanIP !== '' ? $lanIP : '0.0.0.0';
} else {
		$values['lan'] = '0';
		$values['lanIP'] = '0.0.0.0';
}

if (strncmp($wlan, 'up', 2) === 0) {
    $values['wlan'] = '1';
    $values['wlanIP'] = $wlanIP !== '' ? $wlanIP : '0.0.0.0';
} else {
    $values['wlan'] = '0';
    $values['wlanIP'] = '0.0.0.0';
}

if (strncmp($ppp, 'up', 2) === 0) {
    $values['ppp'] = '1';
    $values['pppIP'] = isset($hilink['IPADDRESS']) ? $hilink['IPADDRESS'] : '0.0.0.0';
} else {
    $values['ppp'] = '0';
    $values['pppIP'] = '0.0.0.0';
}

// ssid
$values['ssid'] = run_command('iwgetid -r 2>/dev/null');

// image
$values['image'] = run_command('ls /var/tmp/img/ 2>/dev/null');

echo json_encode($values);
?>
