#!/bin/sh
# Process camera: capture, convert, and upload image.
. "$(dirname "$0")/common.sh"  # common windspots scripts

# Ensure sufficient uptime before processing
UPTIME=$(cut -d. -f1 /proc/uptime)
if [ "$UPTIME" -le 180 ]; then
  ws_log "Process Camera postponed - uptime: ${UPTIME}"
  exit 1
fi

# Check if webcam device exists
if [ ! -c "$CAM_DEVICE" ]; then
  ws_log "Webcam not present; skipping image capture."
  exit 1
fi

# Skip processing during night hours
ws_night
if [ $? -ne 0 ]; then
  ws_log "It is night time; no picture taken."
  exit 1
fi

rm -f "${IMAGE_PATH}"
touch "$TMP/webcam.log"
fswebcam -c "$WINDSPOTS_ETC/fswebcam.conf" > "$TMP/webcam.log"

# Check for corrupt JPEG in log
if grep -q "Corrupt JPEG" "$TMP/webcam.log"; then
  ws_log "Corrupt JPEG detected; aborting capture."
  exit 2
fi

if [ -f "$IMAGE_PATH" ]; then
	touch "${WINDSPOTS_LASTIMAGE}"
  $WINDSPOTS_BIN/upload-image.sh "${IMAGE_PATH}" $STATION_ID
  rm $TMP/img/*.jpg
  timestamp=$(date -d "today" +"%Y%m%d%H%M")
  cp $TMP/input.jpg $TMP/img/${timestamp}.jpg
else
  ws_log "File ${IMAGE_PATH} not found; camera may be unconnected."
  FILEDATE=$(stat -c %Y "${WINDSPOTS_LASTIMAGE}")
  NOW=$(date +"%s")
  AGE=$((NOW - FILEDATE))
  ws_log "Image age: ${AGE} seconds (threshold: ${IMAGE_TRANSMISSION})"
  if [ "$AGE" -ge "$IMAGE_TRANSMISSION" ]; then
    ws_log "Reboot due to no image received in ${IMAGE_TRANSMISSION} seconds."
    ws_syslog "Reboot due to no image in ${IMAGE_TRANSMISSION} seconds."
    "${WINDSPOTS_BIN}/reboot.sh"
    exit 3
  fi
fi
exit 0
