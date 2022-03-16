<?php
  header('Content-type: application/json');
  function logIt($message) {
          $t = microtime(true);
          $micro = sprintf("%06d",($t - floor($t)) * 1000000);
          $micro = substr($micro,0,3);
//        echo(Date("H:i:s").".$micro"." upload_data.php - ".$message."\n");
        }
  // alim
  // logIt("Start");
  $alim_values = shell_exec("/opt/windspots/bin/getalim");
  $values = parse_ini_string($alim_values);
  // i2c
  $i2cdetect = shell_exec("i2cdetect -y 1");
  // var_dump($i2cdetect);
  // bmp280
  // logIt("bmp280");
  $windspots_ini = parse_ini_file("/opt/windspots/etc/main");
  $bmp280_values = shell_exec("/opt/windspots/bin/getbaro -a ".$windspots_ini['ALTITUDE']);
  $bmp280 = parse_ini_string($bmp280_values);
  // ads1015
  // logIt("ads1015");
  $ads1015_values = shell_exec("/opt/windspots/bin/gettemp");
  $ads1015 = parse_ini_string($ads1015_values);
  // anemo
  // logIt("anemo");
  // $anemo_values = shell_exec("/opt/windspots/bin/getanemo");
  // $anemo = parse_ini_string($anemo_values);
  exec("sudo /opt/windspots/bin/getanemo", $anemo);
  $anemo1=parse_ini_string($anemo[0]);
  $anemo0=parse_ini_string($anemo[1]);
  //
  // logIt("display");
  $pos = strpos($i2cdetect, '40 ');
  if ($pos != false) { $values['I2C40'] = '1'; } else { $values['I2C40'] = '0'; }
  $pos = strpos($i2cdetect, '41 ');
  if ($pos != false) { $values['I2C41'] = '1'; } else { $values['I2C41'] = '0'; }
  $pos = strpos($i2cdetect, '43 ');
  if ($pos != false) { $values['I2C43'] = '1'; } else { $values['I2C43'] = '0'; }
  $pos = strpos($i2cdetect, '48 ');
  if ($pos != false) { $values['I2C48'] = '1'; } else { $values['I2C48'] = '0'; }
  $pos = strpos($i2cdetect, '77 ');
  if ($pos != false) { $values['I2C77'] = '1'; } else { $values['I2C77'] = '0'; }
  // hilink
  $hilink_values = shell_exec("/opt/windspots/bin/hi-status.sh");
  $hilink = parse_ini_string($hilink_values);
  $values['SIGNALICON'] = $hilink['SIGNALICON'];
  $values['MAXSIGNAL'] = $hilink['MAXSIGNAL'];
  $values['IPADDRESS'] = $hilink['IPADDRESS'];
  $values['WORKMODE'] = $hilink['WORKMODE'];
  $values['FULLNAME'] = $hilink['FULLNAME'];
  // bmp280
  $values['PRESSURE'] = $bmp280['PRESSURE'];
  $values['SEALEVEL'] = $bmp280['SEALEVEL'];
  $values['INBOXTEMP'] = $bmp280['TEMPERATURE'];
  //ads1015
  $values['TEMPERATURE'] = $ads1015['TEMPERATURE'];
  // anemo
  $values['DIRECTION'] = $anemo0['DIRECTION'];
  $values['SPEED'] = $anemo1['SPEED']; // speed not working due to WiringPI ISR
  // log
  // logIt("log");
  // display log
  $linecount=9;
  $length=40;
  $file="/var/log/windspots.log";
  $offset_factor=1;
  $bytes=filesize($file);
  $fp = fopen($file, "r") or die("Can't open $file");
  $complete=false;
  while (!$complete) {
      //seek to a position close to end of file
      $offset = $linecount * $length * $offset_factor;
      fseek($fp, -$offset, SEEK_END);
      //we might seek mid-line, so read partial line
      //if our offset means we're reading the whole file,
      //we don't skip...
      if ($offset<$bytes) fgets($fp);
      //read all following lines, store last x
      $lines=array();
      while(!feof($fp))  {
          $line = fgets($fp);
          array_push($lines, $line);
          if (count($lines)>$linecount) {
              array_shift($lines);
              $complete=true;
          }
      }
      //if we read the whole file, we're done, even if we
      //don't have enough lines
      if ($offset>=$bytes)
          $complete=true;
      else
          $offset_factor*=2; //otherwise let's seek even further back
  }
  fclose($fp);
  $log = "";
  foreach($lines as $line) {   $log = $log.$line."<br>";  }
  $log = str_replace(array("\n", "\r"), '', $log);
  $values['LOG'] = $log;
  // network
  $lan  = shell_exec("cat /sys/class/net/eth0/operstate");
  $wlan = shell_exec("cat /sys/class/net/wlan0/operstate");
  $ppp  = shell_exec("cat /sys/class/net/eth1/operstate");
  $lanIP = shell_exec("/sbin/ifconfig eth0 | grep 'inet' | cut -d: -f2 | awk '{ print $2}'");
  $lanIP = str_replace(array("\n", "\r"), '', $lanIP);
  $wlanIP = shell_exec("/sbin/ifconfig wlan0 | grep 'inet' | cut -d: -f2 | awk '{ print $2}'");
  $wlanIP = str_replace(array("\n", "\r"), '', $wlanIP);
  $pppIP = shell_exec("/sbin/ifconfig eth1 | grep 'inet' | cut -d: -f2 | awk '{ print $2}'");
  $pppIP = str_replace(array("\n", "\r"), '', $pppIP);
 
  if(strncmp($lan,"up",2) == 0) {
    $values['lan'] = '1';
    $values['lanIP'] = $lanIP;
  } else {
    $values['lan'] = '0';
    $values['lanIP'] = "0.0.0.0";
  }
  if(strncmp($wlan,"up",2)==0) {
    $values['wlan'] = '1';
    $values['wlanIP'] = $wlanIP;
  } else {
    $values['wlan'] = '0';
    $values['wlanIP'] = "0.0.0.0";
  }
  if(strncmp($ppp,"up",2)==0) {
    $values['ppp'] = '1';
    $values['pppIP'] = $hilink['IPADDRESS'];
  } else {
    $values['ppp'] = '0';
    $values['pppIP'] = "0.0.0.0";
  }
  // ssid
  $ssid = shell_exec("iwgetid -r");
  $ssid = str_replace(array("\n", "\r"), '', $ssid);
  $values['ssid'] = $ssid;
  // image
  $image = shell_exec("ls /var/tmp/img/");
  $image = str_replace(array("\n", "\r"), '', $image);
  $values['image'] = $image;
  // logIt("End");
  //var_dump($values);
  echo json_encode($values);
?>