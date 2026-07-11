#!/bin/sh
BOOT_MARKER=/run/windspots-booting
touch "${BOOT_MARKER}"
trap 'rm -f "${BOOT_MARKER}"' EXIT
trap 'exit 1' HUP INT TERM

# Prepare the log before common.sh writes its first message.
install -d -o windspots -g www-data -m 0775 /opt/windspots/log
touch /var/log/windspots.log
chown windspots:www-data /var/log/windspots.log
chmod 0664 /var/log/windspots.log
ln -sfn /var/log/windspots.log /opt/windspots/log/windspots.log

# Load common functions and configuration
. "$(dirname "$0")/common.sh"
ws_syslog "WS ${STATION} boot"
ws_log_console "[startup] Booting station ${STATION}"

# Clean-up temporary files and logs
rm -f "${TMP}/lastconnection" "${TMP}/lastimage" "${TMP}/input.jpg"
touch "${TMP}/lastconnection" "${TMP}/lastimage"

# Create the temporary weather database if /var/tmp was cleaned between boots.
# Do not call ws-configure.sh here: it archives and replaces an existing database.
/usr/bin/install -d -o windspots -g www-data -m 2775 "${WEATHER_DB_DIR}"
if ! "${WINDSPOTS_BIN}/initwsdb" -s "${STATION}" -l "${LOG}" -t "${WEATHER_DB_DIR}"; then
  ws_log_console "ERROR: weather database initialization failed"
  exit 1
fi

chown windspots:www-data "${WEATHER_DB}"
chmod 0664 "${WEATHER_DB}"

TABLE_COUNT=$(sqlite3 "${WEATHER_DB}" \
  "SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name IN ('data', 'log');")
if [ "${TABLE_COUNT}" != "2" ]; then
  ws_log_console "ERROR: weather database schema is incomplete"
  exit 1
fi

for WEATHER_USER in windspots www-data; do
  if ! runuser -u "${WEATHER_USER}" -- sqlite3 -cmd '.timeout 5000' "${WEATHER_DB}" \
    "BEGIN IMMEDIATE;
     INSERT INTO data (last_update, name, channel, battery, temperature, temperature_sign,
                       relative_humidity, barometer, wind_direction, wind_speed, wind_speed_average)
     VALUES (CURRENT_TIMESTAMP, 'BOOT-TEST', 0, 100, 20, '0', 50, 1013, 180, 5, 4);
     ROLLBACK;"; then
    ws_log_console "ERROR: weather database is not writable by ${WEATHER_USER}"
    exit 1
  fi
done

# Network checks can continue in background once the database is ready.
"${WINDSPOTS_BIN}/check-network.sh" > /dev/null 2>&1 &

# Disable SysRq
echo 0 > /proc/sys/kernel/sysrq

# LED configuration: set led0 to heartbeat and turn off led1 power indicator
echo heartbeat > /sys/class/leds/ACT/trigger
echo 0 | tee /sys/class/leds/PWR/brightness

# Create img directory
mkdir -p /var/tmp/img
chmod 755 /var/tmp/img
# Start nginx service and prepare logging directories
mkdir -p /var/log/nginx
service nginx start

"${WINDSPOTS_BIN}/mesh-check.sh" > /dev/null 2>&1 &
"${WINDSPOTS_BIN}/mesh-selfheal.sh" > /dev/null 2>&1 &

# Adjust rights for update and serial configuration
chown -R windspots:windspots /tmp /opt/windspots/etc/serial

# Create and set permissions for HTML info file
touch /var/tmp/infos
chown www-data:windspots /var/tmp/infos
chmod 664 /var/tmp/infos

# Mesh debug
# add in etc/main MESH_DEBUG=Y
# "${WINDSPOTS_BIN}/mesh-check.sh" > /dev/null 2>&1 &

#  log message
echo "[ ok ] WindSpots Station started."
ws_log_console "[ ok ] WindSpots Station started."
exit 0
