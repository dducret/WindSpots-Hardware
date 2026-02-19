#!/bin/sh
# Common initialization for WindSpots scripts

ARCH=$(uname -m)
. /opt/windspots/etc/main

export UPLOAD_IMAGE_URL
export UPLOAD_WEATHER_URL

WINDSPOTS_LASTTRANSMISSION=${TMP}/lasttransmission
WINDSPOTS_LASTIMAGE=${TMP}/lastimage
STATION_ID="${STATION}1"

# Update PATH as needed
PATH="/opt/windspots/bin:/bin:/sbin:/usr/bin:/usr/sbin"
echo $LOG

ws_assert_configuration() {
  if [ -z "$STATION" ]; then
    ws_log "STATION IS NOT CONFIGURED - aborting"
    exit 250
  fi
}

ws_log() {
  local DATE
  DATE=$(date +"%T.%3N")
  echo "$DATE $(basename "$0") - $@" >> ${WINDSPOTS_LOG}
  # echo "$@"
}

ws_syslog() {
  local DATE
  DATE=$(date +"%Y-%m-%d %H:%M:%S")
  echo "$DATE $(readlink -f "$0") - $@" >> /opt/windspots/log/sys.log
  echo "$@"
}

ws_night() {
  local YEAR TIMEHHMM
  YEAR=$(date +%Y)
  [ "$YEAR" -le 2010 ] && return 0
  if [ $SUNTEST = "y" ] || [ $SUNTEST = "Y" ]; then
    TIMEHHMM=$(date +%k%M)
    [ "$TIMEHHMM" -ge "$SUNSET" ] && return 1
    [ "$TIMEHHMM" -le "$SUNRISE" ] && return 2
  fi
  return 0
}

# Prevent multiple script instances using a PID file
scriptname=$(basename "$0")
pidfile="${TMP}/${scriptname}.pid"
touch $pidfile
read lastPID < "$pidfile"
if [ -n "$lastPID" ] && [ -d /proc/$lastPID ]; then
  ws_log "Process already running: ${scriptname}"
  exit 1
fi
echo $$ > $pidfile
