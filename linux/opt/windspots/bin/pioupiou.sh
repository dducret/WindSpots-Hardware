#!/bin/sh
# This will get data from http://developers.pioupiou.fr/api/live/
. `dirname $0`/common.sh        # common windspots scripts
#
STATION=23
#
FILE=$TMP/pioupiou.json
rm $FILE  2> /dev/null
#
curl -s -X GET "http://api.pioupiou.fr/v1/live/$STATION" \
-H "Content-Type: text/xml" > $FILE
#
stationname=$(jq -r '.data.meta.name' $FILE)
datetime=$(jq -r '.data.measurements.date' $FILE)
direction=$(jq -r '.data.measurements.wind_heading' $FILE)
speedkm=$(jq -r '.data.measurements.wind_speed_avg' $FILE)
speed=`printf "%.1f" $(jq -n $speedkm/3.6)`
gustkm=$(jq -r '.data.measurements.wind_speed_max' $FILE)
gust=`printf "%.1f" $(jq -n $gustkm/3.6)`
minkm=$(jq -r '.data.measurements.wind_speed_min' $FILE)
min=`printf "%.1f" $(jq -n $minkm/3.6)`
lastupdate=$(date +'%F %T')
ws_log "PP001 - $lastupdate Wind Direction:$direction, Wind Speed:$gust, Average Speed:$min"
#
DB=$TMP/ws.db
sqlite3 $DB "INSERT INTO data (last_update, name, channel, battery, temperature, temperature_sign, \
 relative_humidity, barometer, wind_direction, wind_speed, wind_speed_average) \
 values ('$lastupdate','PP001',1,0,0,0,0,0,$direction,$gust,$min);"
# test
#  sqlite3 /var/tmp/ws.db "select * from data;"