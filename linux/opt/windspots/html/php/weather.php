<?php
  header('Content-type: application/json');
  $values = [];
  // image
  $imageFiles = @scandir('/var/tmp/img');
  $image = is_array($imageFiles) ? implode('', array_diff($imageFiles, array('.', '..'))) : '';
  $values['image'] = $image;
  // weather data
  $newTime = strtotime('-1 minutes');
  $lastUpload = Date("Y-m-d H:i:s", $newTime);
  try {
    $db = new SQLite3('/var/tmp/windspots/ws.db');
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
  } catch(Exception $e) {
    error_log("Unable to open database connection: " . $e->getMessage());
  }
  // station date and time
  $date = Date("d-M-Y");
  $values['date'] = $date;
  $time = Date("H:i:s");
  $values['time'] = $time;

  //var_dump($values);
  echo json_encode($values);
?>
