<?php
  // var_dump($_POST);
  $reboot ="";
  if(isset($_POST['reboot'])) $ssid = $_POST['reboot'];
  else {
        echo "No Date provided";
        return;
  }
  shell_exec("touch /tmp/reboot");
?>