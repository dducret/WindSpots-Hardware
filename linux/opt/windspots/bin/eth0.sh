#!/bin/bash
. `dirname $0`/common.sh        # common windspots scripts
ws_log "request RJ45 $1"
STATUS=`cat /sys/class/net/eth0/operstate`
PARAMETER="$1"
if [[ ${STATUS:0:2} == "up" ]];then
  if [[ ${PARAMETER} == "down" ]];then
     sudo ip link set eth0 down
     ws_log "RJ45 going down..."
  else
     ws_log "RJ45 is already up."
  fi
else
  if [[ ${PARAMETER} == "up" ]];then
     sudo ip link set eth0 up
     ws_log "RJ45 going up..."
  else
     ws_log "RJ45 is already down."
  fi
fi