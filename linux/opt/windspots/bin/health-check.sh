#!/bin/bash
# BASH_ARGV0=`dirname $0`/health
. `dirname $0`/common.sh        # common windspots scripts
# check if reboot requested
if [ -f "/tmp/reboot" ];then
  ws_log "*****************************"
  ws_log "*"
  ws_log "*  Rebooting ${STATION} by config.php - Uptime: $UPDAY day(s) $UPHOUR:$UPMIN..."
  ws_log "*"
  ws_log "*****************************"
  ws_syslog "Reboot by config.php - Uptime: $UPDAY day(s) $UPHOUR:$UPMIN..."
  sleep 4
  /sbin/reboot
fi
if [ "$W3RPI" = "Y" ]; then
  # check if w3rpi is running
  if [ `pgrep -f w3rpi` ];then
    echo "w3rpi running"
  else
    ws_log "No w3rpi process found - restarting"
    $WINDSPOTS_BIN/process-weather.sh
  fi
fi
if [ "$W3BLE" = "Y" ]; then
  # check if w3ble is running
  if [ `pgrep -f w3ble` ];then
    echo "w3ble running"
  else
    ws_log "No w3ble process found - restarting"
    $WINDSPOTS_BIN/process-weather.sh
  fi
fi
# check if bluetooth is running
if [ `pgrep -f edl_main` ];then
  echo "Bluetooth running"
else
  ws_log "Bluetooth, no edl_main process found - restarting"
  ${WINDSPOTS_BIN}/bluetooth/edl_stop
  ${WINDSPOTS_BIN}/bluetooth/edl_main --use_existing_bridge br0 > /var/log/bluetooth.log 2>&1  &
  /usr/sbin/service nginx restart
fi
# test last transmission
FILEDATE=`stat -c %Y "${WINDSPOTS_LASTTRANSMISSION}"`
NOW=$(date +"%s")
AGE=`expr $NOW - $FILEDATE`
if [ "$AGE" -ge "$DATA_TRANSMISSION" ]; then
  ws_log "test transmission $FILEDATE - AGE $AGE - TEST $DATA_TRANSMISSION"
  /usr/bin/killall upload-data.sh
else
  ws_log "Last Transmission: $AGE"
fi
exit 0