# common initialization for windspots scripts
# read config
ARCH=$(uname -m)
. /opt/windspots/etc/main
export UPLOAD_IMAGE_URL
export UPLOAD_WEATHER_URL
WINDSPOTS_LASTTRANSMISSION=${TMP}/lasttransmission
WINDSPOTS_LASTIMAGE=${TMP}/lastimage
STATION_ID=${STATION}1
PATH=/opt/windspots/bin:/bin:/sbin:/usr/bin:/usr/sbin
ws_assert_configuration()
{
  if [ -z "$STATION" ]; then
    ws_log "STATION IS NOT CONFIGURED - aborting"
    exit 250
  fi
}
ws_log()
{
  local DATE
  DATE=`date +"%T.%3N"`
  # echo "$DATE `basename $0`($$) - $@" >> ${WINDSPOTS_LOG}
  echo "$DATE `basename $0` - $@" >> ${WINDSPOTS_LOG}
  echo "$@"
}
ws_syslog()
{
  local DATE
  DATE=`date +"%Y-%m-%d %H:%M:%S"`
  # echo "$DATE `readlink -f $0`($$) - $@" >> /opt/windspots/log/sys.log
  echo "$DATE `readlink -f $0` - $@" >> /opt/windspots/log/sys.log
  echo "$@"
}
ws_night()
{
  YEAR=$(date +%Y)
  if [ "$YEAR" -le "2010" ];then
     return 0
  fi
  if [ $SUNTEST = "y" -o $SUNTEST = "Y" ];then 
    TIMEHHMM=$(date +%k%M)
    # ws_log "Sunset test - time: $TIMEHHMM"
    if [ "$TIMEHHMM" -ge "$SUNSET" ];then
       return 1
    fi
    if [ "$TIMEHHMM" -le "$SUNRISE" ];then
       return 2
    fi
  fi
  return 0
}
# start
#date +"%T.%3N"
# locking
scriptname=$(basename $0)
pidfile="/tmp/${scriptname}.pid"
touch $pidfile
read lastPID < $pidfile
if [ ! -z "$lastPID" -a -d /proc/$lastPID ]; then
  ws_log "Process already running ${scriptname}"
  exit 1
fi
echo $$ > $pidfile