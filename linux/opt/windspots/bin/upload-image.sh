#!/bin/bash
# legacy image sent by web server station.windspots.org/data.php
# BASH_ARGV0=`dirname $0`/upimg
. `dirname $0`/common.sh        # common windspots scripts
# expects 2 parameters
IMAGE_PATH=$1
IMAGE_NAME=$2
IMAGE_SIZE=`stat -c%s "$IMAGE_PATH"`
IMAGE_TEXT_FORMAT='"$STATION_NAME `date +%d/%m/%Y\ %H:%M`$WINDTAG - (c) WindSpots.com"'
PACKETS=1
# TARGET="www.google.com"
TARGET="8.8.8.8"
if [ ! $STATION_URL ]; then
  ws_log "FAIL: Upload server not set: $STATION_URL"
  exit 1
fi
# check if internet up
RET=`ping -c $PACKETS $TARGET 2> /dev/null | awk '/received/ {print $4}'`
if [ $RET ]; then
  STATION_IMAGE_URL="$STATION_URL/image.php"
  # Image Tag
  WINDTAG=" - No Wind"
  if [ -f "${WINDSPOTS_WTAG}" ]; then
    IMAGE_WINDTAG=`cat $WINDSPOTS_WTAG`
    WINDTAG=${IMAGE_WINDTAG}
    # delete if WTAG is older than 130 seconds
    FILEDATE=`stat -c %Y $WINDSPOTS_WTAG`
    NOW=$(date +"%s")
    AGE=`expr $NOW - $FILEDATE`
    TESTI=130
    if [ "$AGE" -ge "$TESTI" ]; then
      rm $WINDSPOTS_WTAG
    fi
  fi
  # uplaoding image
  eval IMAGE_TEXT="$IMAGE_TEXT_FORMAT"
  curl \
    -X POST \
    -s \
    -F "tag=$IMAGE_TEXT" \
    -F "camrotate=$CAMROTATE" \
    -F file="@$IMAGE_PATH;filename=$IMAGE_NAME" \
    -o $CURL_LOG_PATH \
    -m $CURL_TIMEOUT \
    $STATION_IMAGE_URL
  curl_status=$?
  if [ $curl_status != 0 ]; then
    ws_log "Error, curl exited with status $curl_status - $STATION_IMAGE_URL"
  else
    ws_log "$IMAGE_TEXT"
    # ws_log "Ok $IMAGE_NAME($IMAGE_SIZE) $STATION_IMAGE_URL"
    ws_log "Ok $IMAGE_NAME($IMAGE_SIZE)"
  fi
else
  ws_log "No Internet connection"
fi
exit 0