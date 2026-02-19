#!/bin/bash
. "$(dirname "$0")/common.sh"  # common windspots scripts

# Expects two parameters: IMAGE_PATH and IMAGE_NAME
IMAGE_PATH=$1
IMAGE_NAME=$2
IMAGE_SIZE=$(stat -c '%s' "$IMAGE_PATH")
IMAGE_TEXT_FORMAT="\"$STATION_NAME $(date +'%d/%m/%Y %H:%M')\$WINDTAG - (c) WindSpots.com\""
PACKETS=1
TARGET="8.8.8.8"

if [ -z "$STATION_URL" ]; then
  ws_log "FAIL: Upload server not set: $STATION_URL"
  exit 1
fi

# Check Internet connectivity
if [ -n "$(ping -c $PACKETS "$TARGET" 2>/dev/null | awk '/received/ {print $4}')" ]; then
  STATION_IMAGE_URL="${STATION_URL}/image.php"
  WINDTAG=" - No Wind"
  if [ -f "$WINDSPOTS_WTAG" ]; then
    IMAGE_WINDTAG=$(cat "$WINDSPOTS_WTAG")
    WINDTAG="$IMAGE_WINDTAG"
    FILEDATE=$(stat -c %Y "$WINDSPOTS_WTAG")
    NOW=$(date +"%s")
    AGE=$((NOW - FILEDATE))
    TESTI=130
    [ "$AGE" -ge "$TESTI" ] && rm -f "$WINDSPOTS_WTAG"
  fi

  # Evaluate IMAGE_TEXT with proper variable expansion
  eval IMAGE_TEXT="$IMAGE_TEXT_FORMAT"
  curl -X POST -s \
    -F "tag=${IMAGE_TEXT}" \
    -F "camrotate=${CAMROTATE}" \
    -F "file=@${IMAGE_PATH};filename=${IMAGE_NAME}" \
    -o "$CURL_LOG_PATH" \
    -m "$CURL_TIMEOUT" \
    "$STATION_IMAGE_URL"
  curl_status=$?
  if [ $curl_status -ne 0 ]; then
    ws_log "Error: curl exited with status ${curl_status} when uploading to ${STATION_IMAGE_URL}"
  else
    ws_log "${IMAGE_TEXT}"
    ws_log "Upload successful: ${IMAGE_NAME} (${IMAGE_SIZE} bytes)"
  fi
else
  ws_log "No Internet connection; image upload aborted."
fi
exit 0
