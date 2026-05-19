<?php
  header('Content-Type: application/json; charset=utf-8');

  $ssid = isset($_POST['ssid']) ? trim((string) $_POST['ssid']) : '';
  $wpa = isset($_POST['wpa']) ? (string) $_POST['wpa'] : '';

  if ($ssid === '') {
    http_response_code(400);
    echo json_encode(array('success' => false, 'message' => 'No SSID provided'));
    return;
  }

  if ($wpa === '') {
    http_response_code(400);
    echo json_encode(array('success' => false, 'message' => 'No WPA provided'));
    return;
  }

  if (strpos($ssid, ';') !== false || strpos($wpa, ';') !== false) {
    http_response_code(400);
    echo json_encode(array('success' => false, 'message' => 'SSID and WPA must not contain semicolons'));
    return;
  }

  if (preg_match('/[\r\n\0]/', $ssid) || preg_match('/[\r\n\0]/', $wpa)) {
    http_response_code(400);
    echo json_encode(array('success' => false, 'message' => 'SSID and WPA must be single-line values'));
    return;
  }

  $wpaSourceFile = '/opt/windspots/etc/wpa';
  $line = $ssid . ';' . $wpa . PHP_EOL;

  if (file_put_contents($wpaSourceFile, $line, FILE_APPEND | LOCK_EX) === false) {
    http_response_code(500);
    echo json_encode(array('success' => false, 'message' => 'Unable to update WPA source file'));
    return;
  }

  $output = array();
  $exitCode = 0;
  exec('/usr/bin/sudo /usr/bin/php /opt/windspots/bin/php/generate_wpa.php 2>&1', $output, $exitCode);

  if ($exitCode !== 0) {
    http_response_code(500);
    echo json_encode(array('success' => false, 'message' => 'Unable to generate WPA configuration', 'details' => $output));
    return;
  }

  echo json_encode(array('success' => true, 'message' => 'WPA configuration updated successfully'));
?>
