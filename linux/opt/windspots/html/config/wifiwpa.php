<?php
  // var_dump($_POST);
  $ssid ="TEST";
  $wpa = "TEST1234";
  if(isset($_POST['ssid'])) $ssid = $_POST['ssid'];
  else {
        echo "No SSID provided";
        return;
  }
  if(isset($_POST['wpa'])) $wpa = $_POST['wpa'];
  else {
        echo " No WPA provided";
        return;
  }
  $wpa_line = "echo -n '".$ssid.";".$wpa."' >> /opt/windspots/etc/wpa";
  $wpa_result = shell_exec($wpa_line);
  shell_exec("php /opt/windspots/bin/php/generate_wpa.php");
?>
