#!/bin/sh
. "$(dirname "$0")/common.sh"  # common windspots scripts

IMAGE_AGE=$(ws_file_age_seconds "$WINDSPOTS_LASTIMAGE" 999999)

php "$WINDSPOTS_BIN/php/upload_data.php" "$STATION" "$STATION_NAME" "$ALTITUDE" "$LATITUDE" "$LONGITUDE" "$STATION_URL" "$STATUS_URL" "$TMP" "$WINDSPOTS_WTAG" "$WINDSPOTS_LASTTRANSMISSION" "$IMAGE_AGE" "$LOG" "$VERSION"
status=$?
[ "$status" -ne 0 ] && ws_log "upload-data.sh: php exited with status ${status}"
