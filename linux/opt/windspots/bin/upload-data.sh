#!/bin/sh
# legacy data sent php/upload_data.php
/usr/bin/killall php
/bin/rm /tmp/upload-data.sh.pid
. `dirname $0`/common.sh        # common windspots scripts
FILEDATE=`stat -c %Y $WINDSPOTS_LASTIMAGE`
NOW=$(date +"%s")
IMAGE_AGE=`expr $NOW - $FILEDATE`
/usr/bin/php $WINDSPOTS_BIN/php/upload_data.php ${STATION} "${STATION_NAME}" ${ALTITUDE} ${LATITUDE} ${LONGITUDE} ${STATION_URL} ${STATUS_URL} ${TMP} ${WINDSPOTS_WTAG} ${WINDSPOTS_LASTTRANSMISSION} ${IMAGE_AGE} ${LOG} ${VERSION}
status=$?
if [ $status -ne 0 ]; then
  ws_log "status=$status"
fi