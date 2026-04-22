#!/bin/sh
# Retrieve comprehensive status information from Huawei 3G dongle in HiLink mode
. "$(dirname "$0")/common.sh"  # common windspots scripts

# Proceed only if PPP is enabled
[ "$PPP" = "Y" ] || exit 2

ws_hi_prepare_session || exit 1

# Collect multiple pieces of information
ws_hi_request "GET" "/api/device/basic-information" "$HI_FILE"
ws_hi_request "GET" "/api/monitoring/status" "${HI_FILE}.tmp"
cat "${HI_FILE}.tmp" >> "$HI_FILE"
ws_hi_request "GET" "/api/device/information" "${HI_FILE}.tmp"
cat "${HI_FILE}.tmp" >> "$HI_FILE"
ws_hi_request "GET" "/api/monitoring/traffic-statistics" "${HI_FILE}.tmp"
cat "${HI_FILE}.tmp" >> "$HI_FILE"
ws_hi_request "GET" "/api/net/current-plmn" "${HI_FILE}.tmp"
cat "${HI_FILE}.tmp" >> "$HI_FILE"
rm -f "${HI_FILE}.tmp"

# Parse key values from the XML using awk with field delimiters
devicename=$(awk -F '[<>]' '/DeviceName/{print $3}' "$HI_FILE")
echo "DEVICENAME=$devicename"
imsi=$(awk -F '[<>]' '/Imsi/{print $3}' "$HI_FILE")
echo "IMSI=$imsi"
signalicon=$(awk -F '[<>]' '/SignalIcon/{print $3}' "$HI_FILE")
echo "SIGNALICON=$signalicon"
maxsignal=$(awk -F '[<>]' '/maxsignal/{print $3}' "$HI_FILE")
echo "MAXSIGNAL=$maxsignal"
ipaddress=$(awk -F '[<>]' '/WanIPAddress/{print $3}' "$HI_FILE")
echo "IPADDRESS=$ipaddress"
workmode=$(awk -F '[<>]' '/workmode/{print $3}' "$HI_FILE")
echo "WORKMODE=$workmode"
fullname=$(awk -F '[<>]' '/FullName/{print $3}' "$HI_FILE")
echo "FULLNAME=$fullname"
iccid=$(awk -F '[<>]' '/Iccid/{print $3}' "$HI_FILE")
echo "ICCID=$iccid"
msisdn=$(awk -F '[<>]' '/Msisdn/{print $3}' "$HI_FILE")
echo "PHONE_NUMBER=$msisdn"

connectionstatus=$(awk -F '[<>]' '/ConnectionStatus/{print $3}' "$HI_FILE")
echo "CONNECTIONSTATUS=$connectionstatus"

networktype=$(awk -F '[<>]' '/NetworkType>/{print $3}' "$HI_FILE")
echo "NETWORKTYPE=$networktype"
networktypeex=$(awk -F '[<>]' '/NetworkTypeEx>/{print $3}' "$HI_FILE")
echo "NETWORKTYPEEX=$networktypeex"
simstatus=$(awk -F '[<>]' '/SimStatus>/{print $3}' "$HI_FILE")
echo "SIMSTATUS=$simstatus"
roamingstatus=$(awk -F '[<>]' '/RoamingSTatus>/{print $3}' "$HI_FILE")
echo "ROAMINGSTATUS=$roamingstatus"
servicestatus=$(awk -F '[<>]' '/ServiceStatus/{print $3}' "$HI_FILE")
echo "SERVICESTATUS=$servicestatus"
rat=$(awk -F '[<>]' '/Rat>/{print $3}' "$HI_FILE")
echo "RAT=$rat"

currentconnectiontime=$(awk -F '[<>]' '/CurrentConnectTime/{print $3}' "$HI_FILE")
echo "CURRENTCONNECTIONTIME=$currentconnectiontime"
echo "CURRENTCONNECTIONTIMEHMS=$(date -u -d @${currentconnectiontime} +"%T")"

currentupload=$(awk -F '[<>]' '/CurrentUpload>/{print $3}' "$HI_FILE")
echo "CURRENTUPLOAD=$currentupload"
echo "CURRENTUPLOADMB=$(( currentupload / 1024 ))"

currentdownload=$(awk -F '[<>]' '/CurrentDownload>/{print $3}' "$HI_FILE")
echo "CURRENTDOWNLOAD=$currentdownload"
echo "CURRENTDOWNLOADMB=$(( currentdownload / 1024 ))"

currentuploadrate=$(awk -F '[<>]' '/CurrentUploadRate/{print $3}' "$HI_FILE")
echo "CURRENTUPLOADRATE=$currentuploadrate"
echo "CURRENTUPLOADRATEMB=$(( currentuploadrate / 1024 ))"

currentdownloadrate=$(awk -F '[<>]' '/CurrentDownloadRate/{print $3}' "$HI_FILE")
echo "CURRENTDOWNLOADRATE=$currentdownloadrate"
echo "CURRENTDOWNLOADRATEMB=$(( currentdownloadrate / 1024 ))"

totalupload=$(awk -F '[<>]' '/TotalUpload/{print $3}' "$HI_FILE")
echo "TOTALUPLOAD=$totalupload"
echo "TOTALUPLOADMB=$(( totalupload / 1024 ))"

totaldownload=$(awk -F '[<>]' '/TotalDownload/{print $3}' "$HI_FILE")
echo "TOTALDOWNLOAD=$totaldownload"
echo "TOTALDOWNLOADMB=$(( totaldownload / 1024 ))"

totalconnecttime=$(awk -F '[<>]' '/TotalConnectTime/{print $3}' "$HI_FILE")
echo "TOTALCONNECTTIME=$totalconnecttime"
echo "TOTALCONNECTTIMEHMS=$(date -u -d @${totalconnecttime} +"%T")"
