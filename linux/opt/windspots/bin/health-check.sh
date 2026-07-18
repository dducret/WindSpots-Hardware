#!/bin/bash
. "$(dirname "$0")/common.sh"

marker_is_active() {
  local marker="$1"
  local marker_pid=""

  if [ ! -r "$marker" ]; then
    return 1
  fi
  read -r marker_pid < "$marker"
  if [ -n "$marker_pid" ] && kill -0 "$marker_pid" 2>/dev/null; then
    return 0
  fi
  rm -f "$marker"
  return 1
}

if marker_is_active /run/windspots-installing || marker_is_active /run/windspots-booting; then
  ws_log "Installation or boot in progress - health check paused"
  exit 0
fi

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
    ws_log_console "w3rpi running"
  else
    ws_log "No w3rpi process found - restarting"
    "${WINDSPOTS_BIN}/process-weather.sh"
  fi
fi

# Test last data transmission timing
FILEDATE=$(ws_file_mtime "${WINDSPOTS_LASTTRANSMISSION}")
AGE=$(ws_file_age_seconds "${WINDSPOTS_LASTTRANSMISSION}" "${DATA_TRANSMISSION}")
if [ "$AGE" -ge "$DATA_TRANSMISSION" ]; then
  ws_log "Transmission test: FILEDATE=${FILEDATE} AGE=${AGE} (threshold: ${DATA_TRANSMISSION})"
  killall upload-data.sh 2>/dev/null
else
  ws_log "Last Transmission: ${AGE} seconds ago"
fi

# Check if the default controller is discoverable
# 'bluetoothctl show' returns the full status of the default adapter
# rfkill unblock bluetooth >/dev/null 2>&1
STATUS=$(bluetoothctl show | grep "Discoverable: yes")
if [ -z "$STATUS" ]; then
  ws_log_console "Discoverable is currently NO. Turning it ON..."
  bluetoothctl >/dev/null 2>&1 <<EOF
power on
discoverable on
pairable on
default-agent
EOF
fi

exit 0
