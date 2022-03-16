#!/bin/sh
. `dirname $0`/common.sh        # common windspots scripts
# check lastconnection
if [ ! -f ${TMP}/lastconnection ]; then
  ws_log "lastconnection not exists."
  touch ${TMP}/lastconnection
  exit 0
fi
FILEDATE=`stat -c %Y ${TMP}/lastconnection`
NOW=$(date +"%s")
AGE=`expr $NOW - $FILEDATE`
TESTI=130
if [ "$AGE" -ge "$TESTI" ]; then
  ws_log "No connection from $AGE seconds - TEST $TEST"
fi
TEST=660
if [ "$AGE" -ge "$TEST" ]; then
  rm ${TMP}/lastconnection
  ws_syslog "reboot due to no Internet connection from $TEST second"
  sleep 4
  ${WINDSPOTS_BIN}/reboot.sh
  exit 0
fi
# check which interface should be up and down
LANOPERATE="$(cat /sys/class/net/eth0/operstate)"
if [ "$RJ45" = "Y" ]; then
  if [ "$LANOPERATE" = "up" ]; then
    ETH0=$(ifconfig eth0 | grep  -Po 'inet \K[\d.]+')
    echo "RJ45=Y => up : $ETH0"
  else
    ws_log "RJ45=Y - Interface is down. Turning it up"
    ifconfig eth0 up
    ifup eth0
  fi
else
  if [ "$LANOPERATE" = "up" ]; then
    ws_log "RJ45=N - Interface is up. Turning it down"
    ifconfig eth0 down
    ifdown eth0
  else
    echo "RJ45=N - down"
  fi
fi
# WIFi can be disable, to check  rfkill list and   rfkill unblock 0
WLANOPERATE="$(cat /sys/class/net/wlan0/operstate)"
if [ "$WIFI" = "Y" ]; then
  if [ "$WLANOPERATE" = "up" ]; then
    WLAN0=$(ifconfig wlan0 | grep  -Po 'inet \K[\d.]+')
    echo "WIFI=Y => up : $WLAN0"
  else
    ws_log "WIFI=Y - Interface is $WLANOPERATE. Turning it up"
    ifconfig wlan0 up
    ifup wlan0
  fi
else
  if [ "$WLANOPERATE" = "up" ]; then
    ws_log "WIFI=N - Interface is up. Turning it down"
    ifconfig wlan0 down
    ifdown wlan0
  else
    echo "WIFI=N => down"
  fi
fi
# PPP is now eth1 with Huawei HI Link
PPPOPERATE="down"
if [ -f /sys/class/net/eth1/operstate ]; then
  PPPOPERATE="$(cat /sys/class/net/eth1/operstate)"
else
  PPP="Not connected"
fi
if [ "$PPP" = "Y" ]; then
  if [ "$PPPOPERATE" = "up" ]; then
    ETH1=$(ifconfig eth1 | grep  -Po 'inet \K[\d.]+')
    echo "PPP=Y => up : $ETH1"
  else
    ws_log "PPP=Y - Interface is down. Turning it up"
    ifconfig eth1 up
    ifup eth1
  fi
else
  if [ "$PPPOPERATE" = "up" ]; then
    ws_log "PPP=N - Interface is up. Turning it down"
    ifconfig eth1 down
    ifdown eth1
  else
    echo "PPP=N - down"
  fi
fi
# check if default route exist
IP=$(/sbin/ip route | awk '/default/ { print $3 }')
if [ $IP ]; then
  echo "Default route: ${IP}"
else
  ws_log "No default route"
  if [ "$RJ45" = "Y" ]; then
    if [ "$LANOPERATE" = "up" ]; then
      DEFAULTROUTE="$(grep routers /var/lib/dhcp/dhclient.eth0.leases |sort -u |cut -d ' ' -f 5 |sed -e 's/;//')"
      route add -net 0.0.0.0/0 gw $DEFAULTROUTE
    fi
  fi
  if [ "$PPP" = "Y" ]; then
    if [ "$PPPOPERATE" = "up" ]; then
      route add -net 0.0.0.0/0 gw $MODEM_IP
    fi
  fi
  if [ "$WIFI" = "Y" ]; then
    if [ "$WLANOPERATE" = "up" ]; then
      DEFAULTROUTE="$(grep routers /var/lib/dhcp/dhclient.wlan0.leases |sort -u |cut -d ' ' -f 5 |sed -e 's/;//')"
      route add -net 0.0.0.0/0 gw $DEFAULTROUTE
    fi
  fi
fi
# check br0
PACKETS=1
TARGET10="10.10.10.10"
RET=`ping -c $PACKETS $TARGET10 2> /dev/null | awk '/received/ {print $4}'`
if [ $RET ]; then
  echo "br0 up: $TARGET10"
else
  ws_log "br0 down - restarting"
  ifconfig br0 up
  ifup br0
fi
# check connection
PACKETS=1
TARGET="www.windspots.com"
RET=`ping -c $PACKETS $TARGET 2> /dev/null | awk '/received/ {print $4}'`
if [ $RET ]; then
  echo "Connected to Internet"
  touch ${TMP}/lastconnection
  exit 0
else
  ws_log "no ping with name"
  TARGET2="1.1.1.1"
  RET2=`ping -c $PACKETS $TARGET2 2> /dev/null | awk '/received/ {print $4}'`
  if [ $RET2 ]; then
    echo "Connected to Internet"
    touch ${TMP}/lastconnection
    exit 0
  else
    ws_log "no ping with IP address"
  fi
fi
# end of check network
ws_syslog "Error:"
ws_syslog "no ping with IP address"
if [ "$RJ45" = "Y" ]; then
  if [ "$LANOPERATE" = "up" ]; then
    ws_syslog "  RJ45 - up"
  else
    ws_syslog "  RJ45 - down"
  fi
fi
if [ "$PPP" = "Y" ]; then
  if [ "$PPPOPERATE" = "up" ]; then
    ws_syslog "  PPP - up"
  else
    ws_syslog "  PPP - down"
  fi
fi
if [ "$WIFI" = "Y" ]; then
  if [ "$WLANOPERATE" = "up" ]; then
    ws_syslog "  WIFI - up"
  else
    ws_syslog "  WIFI - down"
  fi
fi