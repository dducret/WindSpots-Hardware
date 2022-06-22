#!/bin/sh
. `dirname $0`/common.sh        # common windspots scripts
ws_syslog "WS ${STATION} boot"
#
rm -f ${TMP}/lastconnection
rm -f ${TMP}/lastimage
rm -f ${TMP}/input.jpg
rm -f ${LOG}/windspots.log
touch ${TMP}/lastconnection
touch ${TMP}/lastimage
touch ${LOG}/windspots.log
#
#/sbin/ifconfig wlan0 down
#/sbin/ifconfig eth0 down
#/sbin/ifconfig eth1 down
#
${WINDSPOTS_BIN}/ws-configure.sh > /dev/null 2>&1 &
#
${WINDSPOTS_BIN}/check-network.sh > /dev/null 2>&1 &
# disable SysRq
echo 0 > /proc/sys/kernel/sysrq
# led 0 - PI default mmc0 - mmc0, cpu0, heartbeat, timer ... cat /sys/class/leds
/led1/trigger
echo heartbeat | tee /sys/class/leds/led0/trigger
# led 1 - B+ turn off power led
/opt/vc/bin/vcmailbox 0x00038041 8 8 130 1
echo 0 | tee /sys/class/leds/led1/brightness
# nginx
mkdir /var/log/nginx
service nginx start
chmod 666 /dev/i2c-1
mkdir /var/tmp/img
# log write for php
touch /var/log/windspots.log
ln -s /var/log/windspots.log /opt/windspots/log/windspots.log
chown windspots:windspots /var/log/windspots.log
chmod 755 /var/log/windspots.log
# rights for /var/www/html/config/update.php
chown windspots:windspots /tmp -R
chown windspots:windspots /opt/windspots/etc/serial
chmod 777 /etc/hosts
chmod 777 /etc/hostname
chmod 777 /etc/issue
# infos for html/config
touch /var/tmp/infos
chmod 777 /var/tmp/infos
echo "[ ok ] Starting WindSpots Station... [$MYDATE] done."
# bug ???
exit 0