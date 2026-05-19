#!/bin/sh
. "$(dirname "$0")/common.sh"  # common windspots scripts

ws_log "Request 3G ${1:-}"
case "${1:-}" in
  up|down) ;;
  *)
    ws_log "Invalid PPP command: ${1:-}"
    exit 2
    ;;
esac

PPP_IFACE=$(ws_get_ppp_iface)

status=$(cat "/sys/class/net/${PPP_IFACE}/operstate")
param="$1"

if [ "${status%% *}" = "up" ]; then
  if [ "$param" = "down" ]; then
    ip link set "$PPP_IFACE" down
    ws_log "PPP interface ${PPP_IFACE} going down..."
  else
    ws_log "PPP interface ${PPP_IFACE} is already up."
  fi
else
  if [ "$param" = "up" ]; then
    ip link set "$PPP_IFACE" up
    ws_log "PPP interface ${PPP_IFACE} going up..."
  else
    ws_log "PPP interface ${PPP_IFACE} is already down."
  fi
fi
