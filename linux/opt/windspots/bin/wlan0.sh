#!/bin/bash
. "$(dirname "$0")/common.sh"  # Load common windspots scripts

ws_log "Request Wifi $1"
STATUS=$(cat /sys/class/net/wlan0/operstate)
PARAMETER="$1"

if [[ "$STATUS" == up* ]]; then
  if [[ "$PARAMETER" == "down" ]]; then
    sudo ip link set wlan0 down
    ws_log "Wifi going down..."
  else
    ws_log "Wifi is already up."
  fi
else
  if [[ "$PARAMETER" == "up" ]]; then
    sudo ip link set wlan0 up
    ws_log "Wifi going up..."
  else
    ws_log "Wifi is already down."
  fi
fi
