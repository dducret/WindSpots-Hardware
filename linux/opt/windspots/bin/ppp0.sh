#!/bin/sh
. "$(dirname "$0")/common.sh"  # common windspots scripts

ws_log "Request 3G $1"
if [ -f /sys/class/net/usb0/operstate ]; then
  PPP_IFACE="usb0"
else
  PPP_IFACE="eth1"
fi

status=$(cat "/sys/class/net/${PPP_IFACE}/operstate")
param="$1"

if [ "${status%% *}" = "up" ]; then
  if [ "$param" = "down" ]; then
    sudo ip link set "$PPP_IFACE" down
    ws_log "PPP interface ${PPP_IFACE} going down..."
  else
    ws_log "PPP interface ${PPP_IFACE} is already up."
  fi
else
  if [ "$param" = "up" ]; then
    sudo ip link set "$PPP_IFACE" up
    ws_log "PPP interface ${PPP_IFACE} going up..."
  else
    ws_log "PPP interface ${PPP_IFACE} is already down."
  fi
fi
