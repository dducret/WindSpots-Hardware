#!/bin/sh
. "$(dirname "$0")/common.sh"  # common windspots scripts
# kill running
/bin/sleep 5
/usr/bin/killall w3rpi
# set serial PI
SERIAL="$(cat /proc/cpuinfo | grep Serial | cut -d ':' -f 2)"
echo "$SERIAL" > /opt/windspots/etc/serial
/bin/chown windspots:windspots /opt/windspots/etc/serial
# init database
/usr/bin/install -d -o windspots -g www-data -m 2775 "$WEATHER_DB_DIR"
cd "$WEATHER_DB_DIR" || exit 1
/bin/gzip -f -9 ws.db
/bin/rm -f ws.db-journal ws.db-wal ws.db-shm
if ! "$WINDSPOTS_BIN"/initwsdb -s "${STATION}" -l "${LOG}" -t "${WEATHER_DB_DIR}"; then
  ws_log_console "ws-configure: weather database initialization failed"
  exit 1
fi
/bin/chown windspots:www-data ws.db
/bin/chmod 664 ws.db
# setup welcome message
/bin/echo '127.0.0.1 localhost' > /etc/hosts
/bin/echo "127.0.1.1 ${STATION}" >> /etc/hosts
/bin/echo "Debian 11 - ${VERSION}" > /etc/issue
/bin/echo "${STATION}" > /etc/hostname
/usr/bin/hostnamectl set-hostname "${STATION}"
# register the station
/usr/bin/php "$WINDSPOTS_BIN/php/upload_station.php" "${STATION}" "${STATION_NAME}" "${STATION_SHORT}" "${MS_NAME}" "${INFORMATION}" "${SPOT_TYPE}" "${ONLINE}" "${MAINTENANCE}" "${REASON}" "${ALTITUDE}" "${LATITUDE}" "${LONGITUDE}" "${GMT}" "${STATION_URL}" "${LOG}"
#
exit 0
