#!/bin/sh
. "$(dirname "$0")/common.sh"  # Load common windspots scripts

rm -v "${TMP}/lastconnection"
ws_log "Shutdown initiated..."
ws_syslog "Shutdown initiated..."
/sbin/init 0
