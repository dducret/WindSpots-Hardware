<?php 
  header('Content-type: application/json');
  // image
  $image = shell_exec("ls /var/tmp/img/");
  $image = str_replace(array("\n", "\r"), '', $image);
  $values['image'] = $image;
  // weather data
  $newTime = strtotime('-1 minutes');
  $lastUpload = Date("Y-m-d H:i:s", $newTime);
  try {
    $db = new SQLite3('/var/tmp/ws.db');
    $SQLnow = Date("Y-m-d H:i:s");
    // get data
    $result = $db->query('SELECT * FROM data ORDER BY id DESC LIMIT 1');
    $nbrec = 0;
    while($row = $result->fetchArray()) {
      // var_dump($row);
      $weatherdata[] = $row;
      $values['sensor'] = $row['name'];
      $values['dir'] = $row['wind_direction'];
      $values['speed'] = $row['wind_speed'];
      $values['speed_average'] =  $row['wind_speed_average'];
      $values['temperature'] = $row['temperature'];
      $values['humidity'] = $row['relative_humidity'];
      $values['barometer'] = $row['barometer'];
    }
    $db->close();
    unset($db);
    $db = null;
  } catch(PDOException $e) {
    logIt("Unable to open database connection: ".print_r($e,true));
  }
  // station date and time
  $date = Date("d-M-Y");
  $values['date'] = $date;
  $time = Date("H:i:s");
  $values['time'] = $time;
  
  //var_dump($values);
  echo json_encode($values);  
?>