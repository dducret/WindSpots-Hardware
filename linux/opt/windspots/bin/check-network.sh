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

# Check interface operstates
LANOPERATE=$(get_operstate eth0)
WLANOPERATE=$(get_operstate wlan0)
# No longer using eth1 for PPP; USB dongle is now on usb0
if [ -f /sys/class/net/usb0/operstate ]; then
    USBOPERATE=$(get_operstate usb0)
else
    USBOPERATE="Not connected"
fi

# Display state info for logging purposes
echo "RJ45 (eth0): $LANOPERATE"
echo "WIFI (wlan0): $WLANOPERATE"
echo "USB dongle (usb0): $USBOPERATE"

# Manage RJ45 interface (eth0)
if [ "$RJ45" = "Y" ]; then
  if [ "$LANOPERATE" = "up" ]; then
    ETH0_IP=$(ifconfig eth0 | grep -Po 'inet \K[\d.]+')
    echo "RJ45=Y => up : $ETH0_IP"
  else
    ws_log "RJ45=Y - Interface is down. Turning it up"
    ifconfig eth0 up && ifup eth0
  fi
else
  if [ "$LANOPERATE" = "up" ]; then
    ws_log "RJ45=N - Interface is up. Turning it down"
    ifconfig eth0 down && ifdown eth0
  else
    echo "RJ45=N => already down"
  fi
fi

# Manage WIFI interface (wlan0)
if [ "$WIFI" = "Y" ]; then
  if [ "$WLANOPERATE" = "up" ]; then
    WLAN0_IP=$(ifconfig wlan0 | grep -Po 'inet \K[\d.]+')
    echo "WIFI=Y => up : $WLAN0_IP"
  else
    ws_log "WIFI=Y - Interface is $WLANOPERATE. Turning it up"
    ifconfig wlan0 up && ifup wlan0
  fi
else
  if [ "$WLANOPERATE" = "up" ]; then
    ws_log "WIFI=N - Interface is up. Turning it down"
    ifconfig wlan0 down && ifdown wlan0
  else
    echo "WIFI=N => already down"
  fi
fi

# Manage USB dongle as PPP interface
if [ "$PPP" = "Y" ]; then
  if [ "$USBOPERATE" = "up" ]; then
    USB_IP=$(ifconfig usb0 | grep -Po 'inet \K[\d.]+')
    echo "PPP=Y (USB) => up : $USB_IP"
  else
    ws_log "PPP=Y - USB interface is down. Turning it up"
    ifconfig usb0 up && ifup usb0
  fi
else
  if [ "$USBOPERATE" = "up" ]; then
    ws_log "PPP=N - USB interface is up. Turning it down"
    ifconfig usb0 down && ifdown usb0
  else
    echo "PPP=N => USB interface already down"
  fi
fi

# Check if a default route exists
DEFAULT_GW=$(/sbin/ip route | awk '/default/ { print $3 }')
if [ -n "$DEFAULT_GW" ]; then
  echo "Default route: ${DEFAULT_GW}"
else
  ws_log "No default route"
  # Example: try to add default route using USB dongle DHCP lease if interface is up.
  if [ "$(get_operstate usb0)" = "up" ]; then
    DEFAULT_ROUTE=$(grep routers /var/lib/dhcp/dhclient.usb0.leases 2>/dev/null | sort -u | cut -d ' ' -f 5 | sed -e 's/;//')
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
  echo "Connected to Internet"
  touch "${TMP}/lastconnection"
  exit 0
else
  ws_log "No ping response from hostname. Trying IP fallback..."
  TARGET2="1.1.1.1"
  RET2=$(ping -c $PACKETS "$TARGET2" 2>/dev/null | awk '/received/ {print $4}')
  if [ "$RET2" = "1" ]; then
    echo "Connected to Internet via IP fallback."
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
[ "$PPP" = "Y" ] && ws_syslog "  USB (PPP) - $USBOPERATE"
