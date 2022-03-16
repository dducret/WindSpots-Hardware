#!/bin/sh
. `dirname $0`/common.sh        # common windspots scripts
UPTIME="$(cat /proc/uptime | grep -o '^[0-9]\+')"
if [ "$UPTIME" -le "300" ]; then
  ws_log "Reboot postponed - uptime : $UPTIME"
  ws_syslog "Reboot postponed - uptime : $UPTIME"
  exit 1
fi
rm -v ${TMP}/lastconnection
ws_log "Reboot....\n\n\n"
UPMIN=$(( UPTIME/60%60 ))
UPHOUR=$(( UPTIME/60/60%24 ))
UPDAY=$(( UPTIME/60/60/24 ))
ws_syslog "Reboot - Uptime: $UPDAY day(s) $UPHOUR:$UPMIN...\n"
/sbin/reboot