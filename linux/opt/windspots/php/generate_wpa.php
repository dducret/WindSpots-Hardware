<?php
//
$wpaFile = '/etc/wpa_supplicant/wpa_supplicant.conf';
// $wpaFile = '/tmp/wpa_supplicant.conf';
function add_line($line) {
	global $wpaFile;
  $wpa_line = "echo -n '".$line."' >> '".$wpaFile."'";
  $wpa_result = shell_exec($wpa_line);
  echo $wpa_result;
}
//$move = "mv  ".$wpaFile." ".$wpaFile.".old";
//shell_exec($move);
// $reset = "touch ".$wpaFile;
$fh = fopen($wpaFile,'w');
fclose($fh);
shell_exec($reset);
add_line("country=CH\r\n");
add_line("ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev\r\n");
add_line("update_config=1\r\n");
$file_array = file('/opt/windspots/etc/wpa');
foreach ($file_array as $line_number =>&$line) {
    $row=explode(';',$line);
    $wpa_sup = shell_exec("wpa_passphrase ".$row[0]." ".$row[1]." ");
    add_line($wpa_sup);
}
?>