#!/bin/sh
. "$(dirname "$0")/common.sh"  # common windspots scripts

get_ppp_iface() {
  local iface
  local interfaces_file

  for interfaces_file in /etc/network/interfaces "$(dirname "$0")/../../../../interfaces"; do
    if [ -f "$interfaces_file" ]; then
      iface=$(awk '
        $1 == "iface" && ($2 == "eth1" || $2 == "usb0") {
          print $2
          exit
        }
      ' "$interfaces_file")
      if [ -n "$iface" ]; then
        echo "$iface"
        return
      fi
    fi
  done

  for iface in eth1 usb0; do
    if [ -d "/sys/class/net/${iface}" ]; then
      echo "$iface"
      return
    fi
  done

  echo "eth1"
}

ws_log "Request 3G $1"
PPP_IFACE=$(get_ppp_iface)

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
