#!/bin/bash
. "$(dirname "$0")/common.sh"

# Reboot if requested via /tmp/reboot
if [ -f "/tmp/reboot" ]; then
  ws_log "*****************************"
  ws_log "*"
  ws_log "*  Rebooting ${STATION} by config.php - Uptime: ${UPDAY} day(s) ${UPHOUR}:${UPMIN}..."
  ws_log "*"
  ws_log "*****************************"
  ws_syslog "Reboot by config.php - Uptime: ${UPDAY} day(s) ${UPHOUR}:${UPMIN}..."
  sleep 4
  /sbin/reboot
fi

# Check if weather processes are running; restart if not
if [ "$W3RPI" = "Y" ]; then
  if pgrep -f w3rpi > /dev/null; then
    echo "w3rpi running"
  else
    ws_log "No w3rpi process found - restarting"
    "${WINDSPOTS_BIN}/process-weather.sh"
  fi
fi

# Test last data transmission timing
FILEDATE=$(stat -c %Y "${WINDSPOTS_LASTTRANSMISSION}")
NOW=$(date +"%s")
AGE=$((NOW - FILEDATE))
if [ "$AGE" -ge "$DATA_TRANSMISSION" ]; then
  ws_log "Transmission test: FILEDATE=${FILEDATE} AGE=${AGE} (threshold: ${DATA_TRANSMISSION})"
  killall upload-data.sh 2>/dev/null
else
  ws_log "Last Transmission: ${AGE} seconds ago"
fi
exit 0
