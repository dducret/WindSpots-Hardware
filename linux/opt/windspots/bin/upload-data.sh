#!/bin/sh
killall php 2>/dev/null
rm -f /tmp/upload-data.sh.pid
. "$(dirname "$0")/common.sh"  # common windspots scripts

FILEDATE=$(stat -c %Y "$WINDSPOTS_LASTIMAGE")
NOW=$(date +"%s")
IMAGE_AGE=$((NOW - FILEDATE))

php "$WINDSPOTS_BIN/php/upload_data.php" "$STATION" "$STATION_NAME" "$ALTITUDE" "$LATITUDE" "$LONGITUDE" "$STATION_URL" "$STATUS_URL" "$TMP" "$WINDSPOTS_WTAG" "$WINDSPOTS_LASTTRANSMISSION" "$IMAGE_AGE" "$LOG" "$VERSION"
status=$?
[ $status -ne 0 ] && ws_log "upload-data.sh: php exited with status ${status}"
