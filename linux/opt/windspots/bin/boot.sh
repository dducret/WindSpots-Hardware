#!/bin/sh
# Load common functions and configuration
. "$(dirname "$0")/common.sh"
ws_syslog "WS ${STATION} boot"

# Clean-up temporary files and logs
rm -f "${TMP}/lastconnection" "${TMP}/lastimage" "${TMP}/input.jpg" "${LOG}/windspots.log"
touch "${TMP}/lastconnection" "${TMP}/lastimage" "${LOG}/windspots.log"

# Launch configuration and network check in background
"${WINDSPOTS_BIN}/ws-configure.sh" > /dev/null 2>&1 &
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

# Prepare PHP log file for web server access
touch /var/log/windspots.log
ln -sf /var/log/windspots.log /opt/windspots/log/windspots.log
chown windspots:windspots /var/log/windspots.log
chmod 755 /var/log/windspots.log

# Adjust rights for update and serial configuration
chown -R windspots:windspots /tmp /opt/windspots/etc/serial
chmod 777 /etc/hosts /etc/hostname /etc/issue

# Create and set permissions for HTML info file
touch /var/tmp/infos
chmod 777 /var/tmp/infos

#  log message
echo "[ ok ] WindSpots Station started."
exit 0
