#!/bin/sh
. "$(dirname "$0")/common.sh"  # common windspots scripts

IMAGE_AGE=$(ws_file_age_seconds "$WINDSPOTS_LASTIMAGE" 999999)

timeout --signal=TERM --kill-after=5s 55s \
  php "$WINDSPOTS_BIN/php/upload_data.php" "$STATION" "$STATION_NAME" "$ALTITUDE" "$LATITUDE" "$LONGITUDE" "$STATION_URL" "$STATUS_URL" "$TMP" "$WINDSPOTS_WTAG" "$WINDSPOTS_LASTTRANSMISSION" "$IMAGE_AGE" "$LOG" "$VERSION"
status=$?
if [ "$status" -eq 124 ]; then
  ws_log "upload-data.sh: transmission timed out after 55 seconds"
elif [ "$status" -ne 0 ]; then
  ws_log "upload-data.sh: php exited with status ${status}"
fi
