<?php
  header('Content-Type: application/json; charset=utf-8');

  function runCommand($cmd, &$errors, &$messages, $okMessage, $errorMessage) {
    $output = array();
    $exitCode = 0;
    exec($cmd . ' 2>&1', $output, $exitCode);
    if ($exitCode !== 0) {
      $details = count($output) > 0 ? (': ' . implode(" | ", $output)) : '';
      $errors[] = $errorMessage . $details;
      return false;
    }
    if ($okMessage !== '') {
      $messages[] = $okMessage;
    }
    return true;
  }

  $main_file = "/opt/windspots/etc/main";
  $fswebcam_file = "/opt/windspots/etc/fswebcam.conf";

  $errors = array();
  $messages = array();

  if (!file_exists($main_file)) {
    $errors[] = "Missing configuration file: " . $main_file;
  }

  if (!file_exists($fswebcam_file)) {
    $errors[] = "Missing camera configuration file: " . $fswebcam_file;
  }

  $windspots_ini = parse_ini_file($main_file);
  if ($windspots_ini === false) {
    $errors[] = "Unable to parse configuration file: " . $main_file;
  }

  if (count($errors) > 0) {
    http_response_code(500);
    echo json_encode(array('success' => false, 'messages' => $errors));
    exit;
  }

  $station = isset($windspots_ini['STATION']) ? $windspots_ini['STATION'] : '';
  $station_name = isset($windspots_ini['STATION_NAME']) ? $windspots_ini['STATION_NAME'] : '';
  $altitude = isset($windspots_ini['ALTITUDE']) ? $windspots_ini['ALTITUDE'] : '';
  $dir_adj = isset($windspots_ini['DIRADJ']) ? $windspots_ini['DIRADJ'] : '';
  $cam_rotate = isset($windspots_ini['CAMROTATE']) ? intval($windspots_ini['CAMROTATE'],10) : 0;
  $cam_adj = isset($windspots_ini['CAMADJUST']) ? $windspots_ini['CAMADJUST'] : '';
  $anemo = isset($windspots_ini['WSANEMO']) ? $windspots_ini['WSANEMO'] : '';
  $temp = isset($windspots_ini['WSTEMP']) ? $windspots_ini['WSTEMP'] : '';
  $solar = isset($windspots_ini['WSSOLAR']) ? $windspots_ini['WSSOLAR'] : '';
  $rj45 = isset($windspots_ini['RJ45']) ? $windspots_ini['RJ45'] : '';
  $wifi = isset($windspots_ini['WIFI']) ? $windspots_ini['WIFI'] : '';
  $ppp = isset($windspots_ini['PPP']) ? $windspots_ini['PPP'] : '';

  $new_cam_rotate = 0;
  if(isset($_POST['camrotate']))
    $new_cam_rotate = intval($_POST['camrotate'],10);
  if($new_cam_rotate > 0) {
    if($cam_rotate > 0) {
      $new_cam_rotate = $cam_rotate + $new_cam_rotate;
    } else {
      $new_cam_rotate = $new_cam_rotate - abs($cam_rotate);
    }
  } else {
    if($new_cam_rotate < 0) {
      if($cam_rotate > 0) {
        $new_cam_rotate = $cam_rotate - abs($new_cam_rotate);
      } else {
        $new_cam_rotate = (abs($cam_rotate) + abs($new_cam_rotate)) * -1;
      }
    } else {
      $new_cam_rotate = $cam_rotate;
    }
  }

  $content = @file_get_contents($main_file);
  if ($content === false) {
    $errors[] = "Unable to read configuration file: " . $main_file;
  } else {
    if(isset($_POST['station']))
      $content = str_replace("STATION=".$station, "STATION=".$_POST['station'], $content);
    if(isset($_POST['station_name']))
      $content = str_replace("STATION_NAME=\"".$station_name."\"", "STATION_NAME=\"".$_POST['station_name']."\"", $content);
    if(isset($_POST['altitude']))
      $content = str_replace("ALTITUDE=".$altitude, "ALTITUDE=".$_POST['altitude'], $content);
    if(isset($_POST['dir_adj']))
      $content = str_replace("DIRADJ=".$dir_adj, "DIRADJ=".$_POST['dir_adj'], $content);
    if(isset($_POST['camrotate']))
      $content = str_replace("CAMROTATE=".$cam_rotate, "CAMROTATE=".$new_cam_rotate, $content);
    if(isset($_POST['cam_adj']))
      $content = str_replace("CAMADJUST=".$cam_adj, "CAMADJUST=".$_POST['cam_adj'], $content);
    if(isset($_POST['anemo']))
      $content = str_replace("WSANEMO=".$anemo, "WSANEMO=".$_POST['anemo'], $content);
    if(isset($_POST['solar']))
      $content = str_replace("WSSOLAR=".$solar, "WSSOLAR=".$_POST['solar'], $content);
    if(isset($_POST['temp']))
      $content = str_replace("WSTEMP=".$temp, "WSTEMP=".$_POST['temp'], $content);
    if(isset($_POST['rj45']))
      $content = str_replace("RJ45=".$rj45, "RJ45=".$_POST['rj45'], $content);
    if(isset($_POST['wifi']))
      $content = str_replace("WIFI=".$wifi, "WIFI=".$_POST['wifi'], $content);
    if(isset($_POST['ppp']))
      $content = str_replace("PPP=".$ppp, "PPP=".$_POST['ppp'], $content);

    if (@file_put_contents($main_file, $content) === false) {
      $errors[] = "Unable to write configuration file: " . $main_file;
    } else {
      $messages[] = "Main configuration updated.";
    }
  }

  $content = @file_get_contents($fswebcam_file);
  if ($content === false) {
    $errors[] = "Unable to read camera configuration file: " . $fswebcam_file;
  } else {
    if(isset($_POST['cam_adj']))
      $content = str_replace("crop 2304x648,0x".$cam_adj, "crop 2304x648,0x".$_POST['cam_adj'], $content);

    if (@file_put_contents($fswebcam_file, $content) === false) {
      $errors[] = "Unable to write camera configuration file: " . $fswebcam_file;
    } else {
      $messages[] = "Camera configuration updated.";
    }
  }

  $newRj45 = isset($_POST['rj45']) ? $_POST['rj45'] : $rj45;
  $newWifi = isset($_POST['wifi']) ? $_POST['wifi'] : $wifi;
  $newPpp = isset($_POST['ppp']) ? $_POST['ppp'] : $ppp;

  $ethCmd = strncmp($newRj45,"Y",1) == 0 ? "/opt/windspots/bin/eth0.sh up" : "/opt/windspots/bin/eth0.sh down";
  runCommand($ethCmd, $errors, $messages, "RJ45 network updated.", "Failed to execute network command: " . $ethCmd);

  $wifiCmd = strncmp($newWifi,"Y",1) == 0 ? "/opt/windspots/bin/wlan0.sh up" : "/opt/windspots/bin/wlan0.sh down";
  runCommand($wifiCmd, $errors, $messages, "WiFi network updated.", "Failed to execute network command: " . $wifiCmd);

  $pppCmd = strncmp($newPpp,"Y",1) == 0 ? "/opt/windspots/bin/ppp0.sh up" : "/opt/windspots/bin/ppp0.sh down";
  runCommand($pppCmd, $errors, $messages, "PPP network updated.", "Failed to execute network command: " . $pppCmd);

  $backupCmd = "cp /opt/windspots/etc/main /opt/windspots/etc/main_".date('Ymd-His').".bak";
  runCommand($backupCmd, $errors, $messages, "Backup created.", "Failed to create backup.");

  $configureCmd = "/opt/windspots/bin/ws-configure.sh";
  runCommand($configureCmd, $errors, $messages, "Configuration script executed.", "Failed to run configure script.");

  if (count($errors) > 0) {
    http_response_code(500);
    echo json_encode(array('success' => false, 'messages' => array_merge($messages, $errors)));
  } else {
    echo json_encode(array('success' => true, 'messages' => array_merge($messages, array('Configuration updated successfully.'))));
  }
