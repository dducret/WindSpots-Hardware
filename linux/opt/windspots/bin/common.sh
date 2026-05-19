#!/bin/sh
# Common initialization for WindSpots scripts

ARCH=$(uname -m)
WINDSPOTS_CONFIG="/opt/windspots/etc/main"

if [ ! -r "$WINDSPOTS_CONFIG" ]; then
  echo "ERROR: missing WindSpots configuration: $WINDSPOTS_CONFIG" >&2
  exit 250
fi

. "$WINDSPOTS_CONFIG"

: "${TMP:=/var/tmp}"
: "${LOG:=/var/log}"
: "${WINDSPOTS_BIN:=/opt/windspots/bin}"
: "${WINDSPOTS_ETC:=/opt/windspots/etc}"
: "${WINDSPOTS_LOG:=${LOG}/windspots.log}"
: "${CURL_TIMEOUT:=20}"
: "${SUNRISE:=0}"
: "${SUNSET:=2400}"

mkdir -p "$TMP" "$LOG" 2>/dev/null
touch "$WINDSPOTS_LOG" 2>/dev/null

export UPLOAD_IMAGE_URL
export UPLOAD_WEATHER_URL

WINDSPOTS_LASTTRANSMISSION="${TMP}/lasttransmission"
WINDSPOTS_LASTIMAGE="${TMP}/lastimage"
STATION_ID="${STATION}1"

# Update PATH as needed
PATH="/opt/windspots/bin:/bin:/sbin:/usr/bin:/usr/sbin"

UPTIME=$(cut -d. -f1 /proc/uptime 2>/dev/null || echo 0)
UPMIN=$(( (UPTIME / 60) % 60 ))
UPHOUR=$(( (UPTIME / 3600) % 24 ))
UPDAY=$(( UPTIME / 86400 ))

ws_assert_configuration() {
  if [ -z "${STATION:-}" ]; then
    ws_log "STATION IS NOT CONFIGURED - aborting"
    exit 250
  fi
}

ws_log() {
  local DATE
  DATE=$(date +"%T.%3N")
  echo "$DATE $(basename "$0") - $*" >> "$WINDSPOTS_LOG"
  # echo "$@"
}

ws_log_console() {
  ws_log "$@"
  echo "$@"
}

ws_syslog() {
  local DATE
  local SYSLOG_FILE
  DATE=$(date +"%Y-%m-%d %H:%M:%S")
  SYSLOG_FILE=/opt/windspots/log/sys.log
  mkdir -p /opt/windspots/log 2>/dev/null
  touch "$SYSLOG_FILE" 2>/dev/null
  echo "$DATE $(readlink -f "$0") - $*" >> "$SYSLOG_FILE"
  echo "$*"
}

ws_file_mtime() {
  local file="$1"
  stat -c %Y "$file" 2>/dev/null || echo 0
}

ws_file_age_seconds() {
  local file="$1"
  local fallback="${2:-999999}"
  local filedate
  local now

  filedate=$(ws_file_mtime "$file")
  if [ "$filedate" -eq 0 ] 2>/dev/null; then
    echo "$fallback"
    return 1
  fi

  now=$(date +"%s")
  echo $((now - filedate))
}

ws_is_number() {
  printf '%s\n' "$1" | grep -Eq '^-?[0-9]+([.][0-9]+)?$'
}

ws_get_ppp_iface() {
  local iface
  local interfaces_file

  for interfaces_file in /etc/network/interfaces "$(dirname "$0")/../../../../interfaces"; do
    if [ -f "$interfaces_file" ]; then
      iface=$(awk '
        $1 == "iface" && ($2 == "eth1" || $2 == "usb0") {
          print $2
          exit
        }
      ' "$interfaces_file")
      if [ -n "$iface" ]; then
        echo "$iface"
        return
      fi
    fi
  done

  for iface in eth1 usb0; do
    if [ -d "/sys/class/net/${iface}" ]; then
      echo "$iface"
      return
    fi
  done

  echo "eth1"
}

ws_hi_prepare_session() {
  HI_FILE="${TMP}/hi-information.xml"
  HI_SESSION="${TMP}/hi-token.xml"

  rm -f "$HI_FILE" "$HI_SESSION" 2>/dev/null
  curl -fsS -X GET "http://$MODEM_IP/api/webserver/SesTokInfo" -m "$CURL_TIMEOUT" > "$HI_SESSION" || return 1

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
    curl -fsS -X "$method" "http://$MODEM_IP$endpoint" \
      -m "$CURL_TIMEOUT" \
      -d "$data" \
      -H "Cookie: $HI_COOKIE" \
      -H "__RequestVerificationToken: $HI_TOKEN" \
      -H "Content-Type: text/xml" > "$output"
  else
    curl -fsS -X "$method" "http://$MODEM_IP$endpoint" \
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
  if [ "${SUNTEST:-N}" = "y" ] || [ "${SUNTEST:-N}" = "Y" ]; then
    TIMEHHMM=$(date +%k%M)
    [ "$TIMEHHMM" -ge "$SUNSET" ] && return 1
    [ "$TIMEHHMM" -le "$SUNRISE" ] && return 2
  fi
  return 0
}

# Prevent multiple script instances with an atomic lock directory.
scriptname=$(basename "$0")
lockdir="${TMP}/${scriptname}.lock"
pidfile="${lockdir}/pid"

if ! mkdir "$lockdir" 2>/dev/null; then
  lastPID=""
  [ -r "$pidfile" ] && read lastPID < "$pidfile"
  if [ -n "$lastPID" ] && [ -d "/proc/$lastPID" ]; then
    ws_log "Process already running: ${scriptname}"
    exit 1
  fi
  rm -rf "$lockdir" 2>/dev/null
  if ! mkdir "$lockdir" 2>/dev/null; then
    ws_log "Unable to create process lock: ${lockdir}"
    exit 1
  fi
fi

trap 'rm -rf "$lockdir"' EXIT HUP INT TERM
echo $$ > "$pidfile"
