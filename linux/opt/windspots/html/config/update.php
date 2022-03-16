<?php
  var_dump($_POST);
  $file = "/opt/windspots/etc/main";
  $windspots_ini = parse_ini_file($file);
  //
  $station = $windspots_ini['STATION'];
  $station_name = $windspots_ini['STATION_NAME'];
  $altitude = $windspots_ini['ALTITUDE'];
  $dir_adj = $windspots_ini['DIRADJ'];
  $cam_rotate = intval($windspots_ini['CAMROTATE'],10);
  $cam_adj = $windspots_ini['CAMADJUST'];
  $r433mhz = $windspots_ini['WS433'];
  $anemo = $windspots_ini['WSANEMO'];
  $temp = $windspots_ini['WSTEMP'];
  $solar = $windspots_ini['WSSOLAR'];
  $rj45 = $windspots_ini['RJ45'];
  $wifi = $windspots_ini['WIFI'];
  $ppp = $windspots_ini['PPP'];
  $pppProvider = $windspots_ini['PROVIDER'];
  //
  $ssid = "SDIC";
  //
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
  //
  $fhandle = fopen($file,"r");
  $content = fread($fhandle,filesize($file));
  fclose($fhandle);
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
  if(isset($_POST['r433mhz']))
    $content = str_replace("WS433=".$r433mhz, "WS433=".$_POST['r433mhz'], $content);
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
  if(isset($_POST['pppProvider']))
    $content = str_replace("PROVIDER=".$pppProvider, "PROVIDER=".$_POST['pppProvider'], $content);
  $fhandle = fopen($file,"w");
  fwrite($fhandle,$content);
  fclose($fhandle);  
  //
  $file = "/opt/windspots/etc/fswebcam.conf";
  $fhandle = fopen($file,"r");
  $content = fread($fhandle,filesize($file));
  fclose($fhandle);
  if(isset($_POST['cam_adj']))
    $content = str_replace("crop 2304x648,0x".$cam_adj, "crop 2304x648,0x".$_POST['cam_adj'], $content);
  $fhandle = fopen($file,"w");
  fwrite($fhandle,$content);
  fclose($fhandle);  
  //
  if(isset($_POST['ssid']))
    $ssid = $_POST['ssid'];
  //
  if(strncmp($_POST['rj45'],"Y",1) == 0) { shell_exec("/opt/windspots/bin/eth0.sh up"); } else { shell_exec("/opt/windspots/bin/eth0.sh down"); }
  if(strncmp($_POST['wifi'],"Y",1) == 0) { shell_exec("/opt/windspots/bin/wlan0.sh up"); } else { shell_exec("/opt/windspots/bin/wlan0.sh down"); }
  if(strncmp($_POST['ppp'],"Y",1) == 0) { shell_exec("/opt/windspots/bin/ppp0.sh up"); } else { shell_exec("/opt/windspots/bin/ppp0.sh down"); }
  // shell_exec("cp /opt/windspots/etc/main /opt/windspots/etc/main.bak");
  //
  shell_exec("/opt/windspots/bin/ws-configure.sh");
?>