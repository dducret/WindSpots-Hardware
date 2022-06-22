<?php
  header('Content-type: application/json');
  $file="/var/tmp/infos";
  $fp = fopen($file, "r") or die("Can't open $file");
  $lines = "";
  while(!feof($fp))  {
    $line = fgets($fp);
    $lines=$lines.$line;
  }
  fclose($fp);
  shell_exec("rm /var/tmp/infos");
  shell_exec("/usr/bin/php /opt/windspots/html/php/infosd.php >/var/tmp/infos &");
  echo $lines;
?>