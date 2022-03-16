#!/bin/sh
. `dirname $0`/common.sh        # common windspots scripts
rm -v ${TMP}/lastconnection
ws_log "Shutdown....\n\n\n"
ws_syslog "Shutdown...\n"
/sbin/init 0