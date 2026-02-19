#!/bin/sh
. "$(dirname "$0")/common.sh"  # common windspots scripts

# Get uptime in seconds (integer)
UPTIME=$(cut -d. -f1 /proc/uptime)
if [ "$UPTIME" -le 300 ]; then
  ws_log "Reboot postponed - uptime: ${UPTIME}"
  ws_syslog "Reboot postponed - uptime: ${UPTIME}"
  exit 1
fi

rm -v "${TMP}/lastconnection"
ws_log "Reboot...."

UPMIN=$(( (UPTIME / 60) % 60 ))
UPHOUR=$(( (UPTIME / 3600) % 24 ))
UPDAY=$(( UPTIME / 86400 ))
ws_syslog "Reboot - Uptime: ${UPDAY} day(s) ${UPHOUR}:${UPMIN}..."
/sbin/reboot
