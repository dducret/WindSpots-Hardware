#!/bin/sh
# This will get data from Huawei 3G dongle in HiLink mode
# http://forum.jdtech.pl/Watek-hilink-api-dla-urzadzen-huawei
# https://github.com/tom-2015/Huawei-HiLink
. `dirname $0`/common.sh        # common windspots scripts
# ws_log "Start"
# 
FILE=$TMP/hi-information.xml
SESSION=$TMP/hi-token.xml
rm $FILE  2> /dev/null
rm $SESSION  2> /dev/null
# 
curl -s -X GET "http://$MODEM_IP/api/webserver/SesTokInfo" -m $CURL_TIMEOUT > $SESSION
COOKIE=`grep -oP '(?<=<SesInfo>).*(?=</SesInfo)' $SESSION`
TOKEN=`grep -oP '(?<=<TokInfo>).*(?=</TokInfo)' $SESSION`
#
curl -s -X GET "http://$MODEM_IP/api/device/information" \
-m $CURL_TIMEOUT \
-H "Cookie: $COOKIE" -H "__RequestVerificationToken: $TOKEN" \
-H "Content-Type: text/xml" > $FILE
#
curl -s -X GET "http://$MODEM_IP/api/device/signal" \
-m $CURL_TIMEOUT \
-H "Cookie: $COOKIE" -H "__RequestVerificationToken: $TOKEN" \
-H "Content-Type: text/xml" > $FILE
#
mode=$(cat $FILE | grep mode | sed -e 's/<[^>]*>//g')
rssi=$(cat $FILE | grep rssi | sed -e 's/<[^>]*>//g')
echo MODE=$mode
echo RSSI=$rssi