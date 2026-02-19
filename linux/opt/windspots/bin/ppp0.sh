#!/bin/bash
. "$(dirname "$0")/common.sh"  # common windspots scripts

ws_log "Request 3G $1"
status=$(cat /sys/class/net/eth1/operstate)
param="$1"

if [ "${status%% *}" = "up" ]; then
  if [ "$param" = "down" ]; then
    sudo ip link set eth1 down
    ws_log "3G going down..."
  else
    ws_log "3G is already up."
  fi
else
  if [ "$param" = "up" ]; then
    sudo ip link set eth1 up
    ws_log "3G going up..."
  else
    ws_log "3G is already down."
  fi
fi
