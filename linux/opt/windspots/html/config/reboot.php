<?php
  header('Content-Type: application/json; charset=utf-8');

  if (!isset($_POST['reboot'])) {
    http_response_code(400);
    echo json_encode(array('success' => false, 'message' => 'No date provided'));
    return;
  }

  if (!touch('/tmp/reboot')) {
    http_response_code(500);
    echo json_encode(array('success' => false, 'message' => 'Unable to request reboot'));
    return;
  }

  echo json_encode(array('success' => true, 'message' => 'Reboot requested'));
?>
