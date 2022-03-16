#!/bin/sh
. `dirname $0`/common.sh        # common windspots scripts
/usr/bin/killall w3rpi
/usr/bin/killall w3ble
/bin/sleep 1
if [ "$W3RPI" = "Y" ]; then
  $WINDSPOTS_BIN/w3rpi -s ${STATION} -d ${DIRADJ} -a ${ALTITUDE} -r ${WS433} -n ${WSANEMO} -p ${WSTEMP} -o ${WSSOLAR} -t ${TMP} -l ${LOG} --debug ${DEBUG} 2>&1 &
  status=$?
  if [ $status -ne 0 ]; then
    ws_log "process-weather w3rpi status=$status"
    exit 1
  fi
fi
if [ "$W3BLE" = "Y" ]; then
  $WINDSPOTS_BIN/w3ble -s ${STATION} -d ${DIRADJ} -a ${ADDRESS} -t ${TMP} -l ${LOG} --debug ${DEBUG} 2>&1 &
  status=$?
  if [ $status -ne 0 ]; then
    ws_log "process-weather w3ble status=$status"
    exit 2
  fi
fi
exit 0