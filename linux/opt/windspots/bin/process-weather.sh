#!/bin/sh
. "$(dirname "$0")/common.sh"  # common windspots scripts

# Restart weather processes
killall w3rpi 2>/dev/null
sleep 1

if [ "$W3RPI" = "Y" ]; then
  "${WINDSPOTS_BIN}/w3rpi" -s "$STATION" -d "$DIRADJ" -a "$ALTITUDE" -r "$WS433" -n "$WSANEMO" -p "$WSTEMP" -o "$WSSOLAR" -t "$TMP" -l "$LOG" --debug "$DEBUG" 2>&1 &
  status=$?
  if [ $status -ne 0 ]; then
    ws_log "process-weather w3rpi exited with status ${status}"
    exit 1
  fi
fi

exit 0
