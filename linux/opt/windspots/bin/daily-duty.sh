#!/bin/sh
. "$(dirname "$0")/common.sh"  # common windspots scripts
DESC="Daily Duty - Clean tmp"
echo "$DESC"
rm -f "${TMP}/lastconnection"
rm -f "${TMP}/lastimage"
rm -f "${TMP}/input.jpg"
rm -f "${LOG}/windspots.log"
touch "${TMP}/lastconnection"
touch "${TMP}/lastimage"
touch "${LOG}/windspots.log"
chown windspots:www-data /var/log/windspots.log
chmod 664 /var/log/windspots.log
rm -f /var/log/nginx/*
service nginx restart
>/var/log/autossh.log
>/var/log/auth.log
>/var/log/bluetooth.log
>/var/log/boot.log
>/var/log/btmp
>/var/log/curl.log
>/var/log/daemon.log
>/var/log/debug
>/var/log/kern.log
>/var/log/lastlog
>/var/log/messages
>/var/log/php8.4-fpm.log
>/var/log/ppp.log
>/var/log/syslog
>/var/log/user.log
>/var/log/wtmp
rm -f /var/log/*.1
# reset database
cd "$TMP" || exit 1
/bin/gzip -f -9 ws.db
/usr/bin/touch ws.db
/bin/chown www-data:windspots ws.db
/bin/chmod 664 ws.db
if ! "$WINDSPOTS_BIN"/initwsdb -s "${STATION}" -l "${LOG}" -t "${TMP}"; then
  ws_log_console "daily-duty: weather database initialization failed"
  exit 1
fi
