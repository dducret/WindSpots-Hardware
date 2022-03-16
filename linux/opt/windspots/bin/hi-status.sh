#!/bin/sh
# This will get data from Huawei 3G dongle in HiLink mode
# http://forum.jdtech.pl/Watek-hilink-api-dla-urzadzen-huawei
# https://github.com/tom-2015/Huawei-HiLink
. `dirname $0`/common.sh        # common windspots scripts
#
if [ "$PPP" = "Y" ]; then
  echo "PPP=Y - up : Collecting HI information"
else
  exit 2
fi
FILE=$TMP/hi-information.xml
SESSION=$TMP/hi-token.xml
rm $FILE  2> /dev/null
rm $SESSION  2> /dev/null
# 
curl -s -X GET "http://$MODEM_IP/api/webserver/SesTokInfo" -m $CURL_TIMEOUT > $SESSION
COOKIE=`grep "SessionID=" $SESSION | cut -b 10-147`
TOKEN=`grep "TokInfo" $SESSION | cut -b 10-41`
#
curl -s -X GET "http://$MODEM_IP/api/monitoring/status" \
-m $CURL_TIMEOUT \
-H "Cookie: $COOKIE" -H "__RequestVerificationToken: $TOKEN" \
-H "Content-Type: text/xml" > $FILE
#
curl -s -X GET "http://$MODEM_IP/api/device/information" \
-H "Cookie: $COOKIE" -H "__RequestVerificationToken: $TOKEN" \
-H "Content-Type: text/xml" >> $FILE
#
curl -s -X GET "http://$MODEM_IP/api/monitoring/traffic-statistics" \
-m $CURL_TIMEOUT \
-H "Cookie: $COOKIE" -H "__RequestVerificationToken: $TOKEN" \
-H "Content-Type: text/xml" >> $FILE
#
curl -s -X GET "http://$MODEM_IP/api/net/current-plmn" \
-m $CURL_TIMEOUT \
-H "Cookie: $COOKIE" -H "__RequestVerificationToken: $TOKEN" \
-H "Content-Type: text/xml" >> $FILE
#
signalicon=$(awk -F '[<>]' '/SignalIcon/{print $3}' $FILE)
echo SIGNALICON=$signalicon
maxsignal=$(awk -F '[<>]' '/maxsignal/{print $3}' $FILE)
echo MAXSIGNAL=$maxsignal
ipaddress=$(awk -F '[<>]' '/WanIPAddress/{print $3}' $FILE)
echo IPADDRESS=$ipaddress
workmode=$(awk -F '[<>]' '/workmode/{print $3}' $FILE)
echo WORKMODE=$workmode
fullname=$(awk -F '[<>]' '/FullName/{print $3}' $FILE)
echo FULLNAME=$fullname
# ConnectionStatus:
#   2: "Connection failed, the profile is invalid",
#   3: "Connection failed, the profile is invalid",
#   5: "Connection failed, the profile is invalid",
#   7: "Network access not allowed",
#   8: "Connection failed, the profile is invalid",
#   11: "Network access not allowed",
#   12: "Connection failed, roaming not allowed",
#   13: "Connection failed, roaming not allowed",
#   14: "Network access not allowed",
#   20: "Connection failed, the profile is invalid",
#   21: "Connection failed, the profile is invalid",
#   23: "Connection failed, the profile is invalid",
#   27: "Connection failed, the profile is invalid",
#   28: "Connection failed, the profile is invalid",
#   29: "Connection failed, the profile is invalid",
#   30: "Connection failed, the profile is invalid",
#   31: "Connection failed, the profile is invalid",
#   32: "Connection failed, the profile is invalid",
#   33: "Connection failed, the profile is invalid",
#   37: "Network access not allowed",
#   112: "No automatic connection"
#   113: "No automatic roaming connection"
#   114: "No reconnection"
#   115: "No roaming reconnection"
#   201: "Connection failed, bandwidth exceeded",
#   900: "Connecting",
#   901: "Connected",
#   902: "Disconnected",
#   903: "Disconnecting",
#   904: "Connection failed",
#   905: "No connection, signal poor ",
#   906: "Connection error"
connectionstatus=$(awk -F '[<>]' '/ConnectionStatus/{print $3}' $FILE)
echo CONNECTIONSTATUS=$connectionstatus
# NetworkType + CurrentNetworkTypeEx
#  0: "No service",
#  1: "GSM",
#  2: "GPRS",
#  3: "EDGE",
#  4: "WCDMA",
#  5: "HSDPA",
#  6: "HSUPA",
#  7: "HSPA",
#  8: "TDSCDMA",
#  9: "HSPA+",
#  10: "EVDO rev 0",
#  11: "EVDO rev A",
#  12: "EVDO rev B",
#  13: "1xRTT",
#  14: "UMB",
#  15: "1xEVDV",
#  16: "3xRTT",
#  17: "HSPA+ 64QAM",
#  18: "HSPA+ MIMO",
#  19: "LTE",
#  21 - IS95A
#  22 - IS95B
#  23 - CDMA1x
#  24 - EVDO rev. 0
#  25 - EVDO rev. AND
#  26 - EVDO rev. B
#  27 - Hybrid CDMA1x
#  28 - Hybrid EVDO rev. 0
#  29 - Hybrid EVDO rev. AND
#  30 - Hybrid EVDO rev. B
#  31 - EHRPD rev. 0
#  32 - EHRPD rev. AND
#  33 - EHRPD rev. B
#  34 - Hybrid EHRPD rev. 0
#  35 - Hybrid EHRPD rev. AND
#  36 - Hybrid EHRPD rev. B
#  41 - WCDMA
#  42 - HSDPA
#  43 - HSUPA
#  44 - HSPA
#  45 - HSPA +
#  46 - DC HSPA +
#  61 - TD SCDMA
#  62 - TD HSDPA
#  63 - TD HSUPA
#  64 - TD HSPA
#  65 - TD HSPA +
#  81 - 802.16E
#  101 - LTE
networktype=$(awk -F '[<>]' '/NetworkType>/{print $3}' $FILE)
echo NETWORKTYPE=$networktype
networktypeex=$(awk -F '[<>]' '/NetworkTypeEx>/{print $3}' $FILE)
echo NETWORKTYPEEX=$networktypeex
#  SimStatus
#  0 - there is no SIM card or it is incorrect
#  1 - correct SIM card
#  2 - incorrect SIM card for commutation link (CS)
#  3 - incorrect SIM card for packet commutation (PS)
#  4 - incorrect SIM card for link and packet switching (PS + CS)
#  240 - ROMSIM
#  255 - no SIM card
simstatus=$(awk -F '[<>]' '/SimStatus>/{print $3}' $FILE)
echo SIMSTATUS=$simstatus
# RoamingStatus
#  0 - roaming off
#  1 - roaming enabled
#  2 - no changes
roamingstatus=$(awk -F '[<>]' '/RoamingSTatus>/{print $3}' $FILE)
echo ROAMINGSTATUS=$roamingstatus
# ServiceStatus:
#  2 - the service is available
servicestatus=$(awk -F '[<>]' '/ServiceStatus/{print $3}' $FILE)
echo SERVICESTATUS=$servicestatus
# Rat:
# 0 - 2G network
# 2 - 3G network
# 5 - HSPA / HSPA + network
# 7 - 4G network
rat=$(awk -F '[<>]' '/Rat>/{print $3}' $FILE)
echo RAT=$rat
currentconnectiontime=$(awk -F '[<>]' '/CurrentConnectTime/{print $3}' $FILE)
echo CURRENTCONNECTIONTIME=$currentconnectiontime
echo CURRENTCONNECTIONTIMEHMS=$(date -u -d @${currentconnectiontime} +"%T")
currentupload=$(awk -F '[<>]' '/CurrentUpload>/{print $3}' $FILE)
echo CURRENTUPLOAD=$currentupload
echo CURRENTUPLOADMB="$(( $currentupload / 1024))" 
currentdownload=$(awk -F '[<>]' '/CurrentDownload>/{print $3}' $FILE)
echo CURRENTDOWNLOAD=$currentdownload
echo CURRENTDOWNLOADMB="$(( $currentdownload / 1024))" 
currentuploadrate=$(awk -F '[<>]' '/CurrentUploadRate/{print $3}' $FILE)
echo CURRENTUPLOADRATE=$currentuploadrate
echo CURRENTUPLOADRATEMB="$(( $currentuploadrate / 1024))" 
currentdownloadrate=$(awk -F '[<>]' '/CurrentDownloadRate/{print $3}' $FILE)
echo CURRENTDOWNLOADRATE=$currentdownloadrate
echo CURRENTDOWNLOADRATEMB="$(( $currentdownloadrate / 1024))" 
totalupload=$(awk -F '[<>]' '/TotalUpload/{print $3}' $FILE)
echo TOTALUPLOAD=$totalupload
echo TOTALUPLOADMB="$(( $totalupload / 1024))" 
totaldownload=$(awk -F '[<>]' '/TotalDownload/{print $3}' $FILE)
echo TOTALDOWNLOAD=$totaldownload
echo TOTALDOWNLOADMB="$(( $totaldownload / 1024))" 
totalconnecttime=$(awk -F '[<>]' '/TotalConnectTime/{print $3}' $FILE)
echo TOTALCONNECTTIME=$totalconnecttime
echo TOTALCONNECTTIMEHMS=$(date -u -d @${totalconnecttime} +"%T")