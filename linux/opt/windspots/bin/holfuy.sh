#!/bin/sh
# This will get data from http://api.holfuy.com/live
. "$(dirname "$0")/common.sh"  # common windspots scripts
# settings for Creux de Genthod
STATION=1196
PASSWORD=xxxxxxxxxxxx
#
FILE="${TMP}/holfuy.json"
rm -f "$FILE" 2>/dev/null
#
curl -fsS -X GET "http://api.holfuy.com/live/?s=$STATION&pw=$PASSWORD&m=JSON&tu=C&su=m/s" \
  -H "Content-Type: text/xml" \
  -m "$CURL_TIMEOUT" > "$FILE" || {
  ws_log "HO100 - failed to download Holfuy data"
  exit 1
}
if ! jq -e . "$FILE" >/dev/null 2>&1; then
  ws_log "HO100 - invalid JSON payload"
  exit 1
fi
#
stationname=$(jq -r '.stationName' "$FILE")
datetime=$(jq -r '.dateTime' "$FILE")
direction=$(jq -r '.wind.direction' "$FILE")
speed=$(jq -r '.wind.speed' "$FILE")
gust=$(jq -r '.wind.gust' "$FILE")
min=$(jq -r '.wind.min' "$FILE")
temperature=$(jq -r '.temperature' "$FILE")
if ! ws_is_number "$direction" || ! ws_is_number "$speed" || ! ws_is_number "$gust" || ! ws_is_number "$temperature"; then
  ws_log "HO100 - invalid numeric data: direction=${direction}, speed=${speed}, gust=${gust}, temperature=${temperature}"
  exit 1
fi
lastupdate=$(date +'%F %T')
ws_log "HO100 - $lastupdate Temperature:$temperature, Wind Direction:$direction, Wind Speed:$gust, Average Speed:$speed"
#
DB="${TMP}/ws.db"
sqlite3 "$DB" "INSERT INTO data (last_update, name, channel, battery, temperature, temperature_sign, \
 relative_humidity, barometer, wind_direction, wind_speed, wind_speed_average) \
 values ('$lastupdate','HO100',1,0,$temperature,0,0,0,$direction,$gust,$speed);"
# test
#  sqlite3 /var/tmp/ws.db "select * from data;"
