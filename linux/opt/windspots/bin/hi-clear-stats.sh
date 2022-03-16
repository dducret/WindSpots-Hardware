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
COOKIE=`grep "SessionID=" $SESSION | cut -b 10-147`
TOKEN=`grep "TokInfo" $SESSION | cut -b 10-41`
#
curl -s -X POST "http://$MODEM_IP/api/monitoring/clear-traffic" \
-m $CURL_TIMEOUT \
-d "<request><ClearTraffic>1</ClearTraffic></request>" \
-H "Cookie: $COOKIE" -H "__RequestVerificationToken: $TOKEN" \
-H "Content-Type: text/xml" > $FILE