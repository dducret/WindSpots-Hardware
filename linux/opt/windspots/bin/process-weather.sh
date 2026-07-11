#!/bin/sh
. "$(dirname "$0")/common.sh"  # common windspots scripts

# Restart weather processes
killall w3rpi 2>/dev/null
sleep 1

if [ "$W3RPI" = "Y" ]; then
  WEATHER_DB="${TMP}/ws.db"
  if ! "${WINDSPOTS_BIN}/initwsdb" -s "${STATION}" -l "${LOG}" -t "${TMP}"; then
    ws_log_console "process-weather: weather database initialization failed"
    exit 1
  fi

  TABLE_COUNT=$(sqlite3 "${WEATHER_DB}" \
    "SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name IN ('data', 'log');")
  if [ "${TABLE_COUNT}" != "2" ]; then
    ws_log_console "process-weather: weather database schema is incomplete"
    exit 1
  fi

  chown www-data:windspots "${WEATHER_DB}"
  chmod 0664 "${WEATHER_DB}"

  "${WINDSPOTS_BIN}/w3rpi" -s "$STATION" -d "$DIRADJ" -a "$ALTITUDE" -n "$WSANEMO" -p "$WSTEMP" -o "$WSSOLAR" -t "$TMP" -l "$LOG" --debug "$DEBUG" 2>&1 &
  w3rpi_pid=$!
  sleep 1
  if ! kill -0 "${w3rpi_pid}" 2>/dev/null; then
    wait "${w3rpi_pid}"
    status=$?
    ws_log "process-weather w3rpi exited with status ${status}"
    exit 1
  fi
fi

exit 0
