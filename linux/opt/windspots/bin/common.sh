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

ws_log_console() {
  ws_log "$@"
  echo "$@"
}

ws_syslog() {
  local DATE
  DATE=$(date +"%Y-%m-%d %H:%M:%S")
  echo "$DATE $(readlink -f "$0") - $@" >> /opt/windspots/log/sys.log
  echo "$@"
}

ws_hi_prepare_session() {
  HI_FILE="${TMP}/hi-information.xml"
  HI_SESSION="${TMP}/hi-token.xml"

  rm -f "$HI_FILE" "$HI_SESSION" 2>/dev/null
  curl -s -X GET "http://$MODEM_IP/api/webserver/SesTokInfo" -m "$CURL_TIMEOUT" > "$HI_SESSION" || return 1

  HI_COOKIE=$(grep -oP '(?<=<SesInfo>).*?(?=</SesInfo>)' "$HI_SESSION")
  HI_TOKEN=$(grep -oP '(?<=<TokInfo>).*?(?=</TokInfo>)' "$HI_SESSION")

  [ -n "$HI_COOKIE" ] && [ -n "$HI_TOKEN" ]
}

ws_hi_request() {
  local method="$1"
  local endpoint="$2"
  local output="$3"
  local data="$4"

  if [ -n "$data" ]; then
    curl -s -X "$method" "http://$MODEM_IP$endpoint" \
      -m "$CURL_TIMEOUT" \
      -d "$data" \
      -H "Cookie: $HI_COOKIE" \
      -H "__RequestVerificationToken: $HI_TOKEN" \
      -H "Content-Type: text/xml" > "$output"
  else
    curl -s -X "$method" "http://$MODEM_IP$endpoint" \
      -m "$CURL_TIMEOUT" \
      -H "Cookie: $HI_COOKIE" \
      -H "__RequestVerificationToken: $HI_TOKEN" \
      -H "Content-Type: text/xml" > "$output"
  fi
}

ws_night() {
  local YEAR TIMEHHMM
  YEAR=$(date +%Y)
  [ "$YEAR" -le 2010 ] && return 0
  if [ "$SUNTEST" = "y" ] || [ "$SUNTEST" = "Y" ]; then
    TIMEHHMM=$(date +%k%M)
    [ "$TIMEHHMM" -ge "$SUNSET" ] && return 1
    [ "$TIMEHHMM" -le "$SUNRISE" ] && return 2
  fi
  return 0
}

# Prevent multiple script instances using a PID file
scriptname=$(basename "$0")
pidfile="${TMP}/${scriptname}.pid"
touch "$pidfile"
read lastPID < "$pidfile"
if [ -n "$lastPID" ] && [ -d "/proc/$lastPID" ]; then
  ws_log "Process already running: ${scriptname}"
  exit 1
fi
echo $$ > "$pidfile"
