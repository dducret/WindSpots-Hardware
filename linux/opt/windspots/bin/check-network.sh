#!/bin/sh
. "$(dirname "$0")/common.sh"  # common windspots scripts

# Function to get a normalized operstate for a given interface.
# If the operstate is "unknown" and an IP is assigned, assume "up".
get_operstate() {
    local iface="$1"
    local state
    if [ -f "/sys/class/net/${iface}/operstate" ]; then
        state=$(cat "/sys/class/net/${iface}/operstate" 2>/dev/null)
        if [ "$state" = "unknown" ]; then
            # Fallback: if an IP address is assigned, consider the interface up.
            local ip
            ip=$(ip addr show "$iface" 2>/dev/null | awk '/inet / {print $2}' | cut -d/ -f1)
            if [ -n "$ip" ]; then
                state="up"
            else
                state="down"
            fi
        fi
    else
        state="down"
    fi
    echo "$state"
}

get_iface_ip() {
    local iface="$1"
    ip -4 addr show "$iface" 2>/dev/null | awk '/inet / {print $2}' | cut -d/ -f1 | head -n 1
}

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

# Check interface operstates
LANOPERATE=$(get_operstate eth0)
WLANOPERATE=$(get_operstate wlan0)
PPP_IFACE=$(get_ppp_iface)
USBOPERATE=$(get_operstate "$PPP_IFACE")

# Display state info for logging purposes
ws_log_console "RJ45 (eth0): $LANOPERATE"
ws_log_console "WIFI (wlan0): $WLANOPERATE"
ws_log_console "PPP interface (${PPP_IFACE}): $USBOPERATE"

# Manage RJ45 interface (eth0)
if [ "$RJ45" = "Y" ]; then
  if [ "$LANOPERATE" = "up" ]; then
    ETH0_IP=$(get_iface_ip eth0)
    ws_log_console "RJ45=Y => up : $ETH0_IP"
  else
    ws_log "RJ45=Y - Interface is down. Turning it up"
    ifconfig eth0 up && ifup eth0
  fi
else
  if [ "$LANOPERATE" = "up" ]; then
    ws_log "RJ45=N - Interface is up. Turning it down"
    ifconfig eth0 down && ifdown eth0
  else
    ws_log_console "RJ45=N => already down"
  fi
fi

# Manage WIFI interface (wlan0)
if [ "$WIFI" = "Y" ]; then
  if [ "$WLANOPERATE" = "up" ]; then
    WLAN0_IP=$(get_iface_ip wlan0)
    ws_log_console "WIFI=Y => up : $WLAN0_IP"
  else
    ws_log "WIFI=Y - Interface is $WLANOPERATE. Turning it up"
    ifconfig wlan0 up && ifup wlan0
  fi
else
  if [ "$WLANOPERATE" = "up" ]; then
    ws_log "WIFI=N - Interface is up. Turning it down"
    ifconfig wlan0 down && ifdown wlan0
  else
    ws_log_console "WIFI=N => already down"
  fi
fi

# Manage USB dongle as PPP interface
if [ "$PPP" = "Y" ]; then
  if [ "$USBOPERATE" = "up" ]; then
    USB_IP=$(get_iface_ip "$PPP_IFACE")
    ws_log_console "PPP=Y (${PPP_IFACE}) => up : $USB_IP"
  else
    ws_log "PPP=Y - ${PPP_IFACE} interface is down. Turning it up"
    ifconfig "$PPP_IFACE" up && ifup "$PPP_IFACE"
  fi
else
  if [ "$USBOPERATE" = "up" ]; then
    ws_log "PPP=N - ${PPP_IFACE} interface is up. Turning it down"
    ifconfig "$PPP_IFACE" down && ifdown "$PPP_IFACE"
  else
    ws_log_console "PPP=N => ${PPP_IFACE} interface already down"
  fi
fi

# Check if a default route exists
DEFAULT_GW=$(/sbin/ip route | awk '/default/ { print $3 }')
if [ -n "$DEFAULT_GW" ]; then
  ws_log_console "Default route: ${DEFAULT_GW}"
else
  ws_log "No default route"
  if [ "$(get_operstate "$PPP_IFACE")" = "up" ]; then
    DEFAULT_ROUTE=$(grep routers "/var/lib/dhcp/dhclient.${PPP_IFACE}.leases" 2>/dev/null | sort -u | cut -d ' ' -f 5 | sed -e 's/;//')
    if [ -n "$DEFAULT_ROUTE" ]; then
      route add -net 0.0.0.0/0 gw "$DEFAULT_ROUTE"
    fi
  fi
fi

# Conduct connectivity test
PACKETS=1
TARGET="www.windspots.com"
RET=$(ping -c $PACKETS "$TARGET" 2>/dev/null | awk '/received/ {print $4}')
if [ "$RET" = "1" ]; then
  ws_log_console "Connected to Internet"
  touch "${TMP}/lastconnection"
  exit 0
else
  ws_log "No ping response from hostname. Trying IP fallback..."
  TARGET2="1.1.1.1"
  RET2=$(ping -c $PACKETS "$TARGET2" 2>/dev/null | awk '/received/ {print $4}')
  if [ "$RET2" = "1" ]; then
    ws_log_console "Connected to Internet via IP fallback."
    touch "${TMP}/lastconnection"
    exit 0
  else
    ws_log "No ping response from IP address fallback."
  fi
fi

# Log error details if connectivity is lost
ws_syslog "Error: no ping response from IP address"
[ "$RJ45" = "Y" ] && ws_syslog "  RJ45 - $LANOPERATE"
[ "$WIFI" = "Y" ] && ws_syslog "  WIFI - $WLANOPERATE"
[ "$PPP" = "Y" ] && ws_syslog "  ${PPP_IFACE} (PPP) - $USBOPERATE"
