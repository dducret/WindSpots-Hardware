#!/bin/sh
# This will get data from Huawei 3G dongle in HiLink mode
# http://forum.jdtech.pl/Watek-hilink-api-dla-urzadzen-huawei
# https://github.com/tom-2015/Huawei-HiLink
. "$(dirname "$0")/common.sh"  # common windspots scripts
# ws_log "Start"
# 
ws_hi_prepare_session || exit 1
ws_hi_request "GET" "/api/device/information" "$HI_FILE"
ws_hi_request "GET" "/api/device/signal" "$HI_FILE"
mode=$(grep mode "$HI_FILE" | sed -e 's/<[^>]*>//g')
rssi=$(grep rssi "$HI_FILE" | sed -e 's/<[^>]*>//g')
echo "MODE=$mode"
echo "RSSI=$rssi"
