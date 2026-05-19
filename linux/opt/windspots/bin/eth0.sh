#!/bin/sh
. "$(dirname "$0")/common.sh"  # common windspots scripts

ws_log "Request RJ45 $1"
case "${1:-}" in
  up|down) ;;
  *)
    ws_log "Invalid RJ45 command: ${1:-}"
    exit 2
    ;;
esac

status=$(cat /sys/class/net/eth0/operstate)
param="$1"

if [ "${status%% *}" = "up" ]; then
  if [ "$param" = "down" ]; then
    ip link set eth0 down
    ws_log "RJ45 going down..."
  else
    ws_log "RJ45 is already up."
  fi
else
  if [ "$param" = "up" ]; then
    ip link set eth0 up
    ws_log "RJ45 going up..."
  else
    ws_log "RJ45 is already down."
  fi
fi
