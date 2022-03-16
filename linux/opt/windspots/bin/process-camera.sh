#!/bin/sh
# This will process camera : convert, upload
. `dirname $0`/common.sh        # common windspots scripts
UPTIME="$(cat /proc/uptime | grep -o '^[0-9]\+')"
if [ "$UPTIME" -le "180" ]; then
  ws_log "Process Camera postponed - uptime : $UPTIME"
  exit 1
fi
# ws_log "Start"
if [ ! -c ${CAM_DEVICE} ]; then
  ws_log "Webcam not present, so no picture"
#  $WINDSPOTS_BIN/reboot.sh
  exit 1
fi
# Check is Night in common.sh
ws_night
NIGHT=$?
if [ ! $NIGHT = "0" ];then
  ws_log "It is the night, so no picture"
  exit 1
fi
rm "${IMAGE_PATH}"
touch $TMP/webcam.log
fswebcam -c $WINDSPOTS_ETC/fswebcam.conf > $TMP/webcam.log
# 5MP
# raspistill -o /var/tmp/input.jpg -q 10 -e jpg -n -t 2000 -w 2592 -h 929 -vf -v >$TMP/webcam.log
# 8MP
# raspistill -o /var/tmp/input.jpg -ISO 100 -awb horizon -ex sport -drc high -q 10 -e jpg -n -t 2000 -w 3280 -h 922 -vf -vs -v >$TMP/webcam.log
# raspistill -o /var/tmp/input.raw  -awb horizon -w 3280 -h 922 -ex sports >$TMP/webcam.log
# gm convert /var/tmp/input.raw /var/tmp/input.jpg
CORRUPTJPEG=`grep -F "Corrupt JPEG" $TMP/webcam.log`
if [ ! -z "$CORRUPTJPEG" ]; then
  ws_log "Corrupt JPEG, exit"
  exit 2
fi
if [ -f "${IMAGE_PATH}" ]; then
  touch "${WINDSPOTS_LASTIMAGE}"
  $WINDSPOTS_BIN/upload-image.sh "${IMAGE_PATH}" $STATION_ID
  rm $TMP/img/*.jpg
  cp $TMP/input.jpg $TMP/img/$(date -d "today" +"%Y%m%d%H%M").jpg
else
  ws_log "File ${IMAGE_PATH} does not exist - probably camera is unconnected"
  FILEDATE=`stat -c %Y "${WINDSPOTS_LASTIMAGE}"`
  NOW=$(date +"%s")
  AGE=`expr $NOW - $FILEDATE`
  ws_log "test image FILEDATE $FILEDATE - AGE $AGE - TEST $IMAGE_TRANSMISSION"
  if [ "$AGE" -ge "$IMAGE_TRANSMISSION" ]; then
    ws_log "reboot due to no image from ${IMAGE_TRANSMISSION} seconds"
    ws_syslog "reboot due to no image from ${IMAGE_TRANSMISSION} seconds"
    ${WINDSPOTS_BIN}/reboot.sh
    exit 3
  fi
fi
# ws_log "End"
exit 0