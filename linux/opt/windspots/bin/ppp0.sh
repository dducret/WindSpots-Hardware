#!/bin/bash
. `dirname $0`/common.sh        # common windspots scripts
ws_log "request 3G $1"
STATUS=/sys/class/net/eth1/operstate
PARAMETER="$1"
if [[ ${STATUS:0:2} == "up" ]];then
  if [[ ${PARAMETER} == "down" ]];then
     sudo ip link set eth1 down
     ws_log "3G going down..."
  else
     ws_log "3G is already up."
  fi
else
  if [[ ${PARAMETER} == "up" ]];then
     sudo ip link set eth1 up
     ws_log "3G going up..."
  else
     ws_log "3G is already down."
  fi
fi