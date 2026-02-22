#!/usr/bin/env bash
# install.sh
set -euo pipefail

## Must run as root
if [[ "${EUID}" -ne 0 ]]; then
  echo "ERROR: run as root (sudo -s)"; exit 1
fi

CONFIG_TXT="/boot/firmware/config.txt"
DEBIAN_HOME="/home/debian"
FSTAB="/etc/fstab"
INSTALL_URL='https://station.windspots.org/install/'
MESH_DIR="/opt/meshagent"
MESH_BIN="${MESH_DIR}/meshagent"
MESH_URL='https://mc.windspots.org:444/meshagents?id=c%40BuzaU7IfiIFtx6dIiDjgb479zsNloSnUaeXoLJQ3hgiqj5VS4IM1O9GzJbLjKF&installflags=2&meshinstall=25'
SYSCTL_FILE="/etc/sysctl.d/98-rpi.conf"

export DEBIAN_FRONTEND=noninteractive

TS="$(date +%Y%m%d-%H%M%S)"
BACKUP_DIR="${DEBIAN_HOME}/windspots-setup-backups-${TS}"
mkdir -p "$BACKUP_DIR"
LOGFILE="${DEBIAN_HOME}/log-${TS}.txt"
touch "$LOGFILE"

log() {
  local msg="[$(date +'%F %T')] $*"
  printf "\n%s\n" "$msg"
  printf "%s\n" "$msg" >>"$LOGFILE"
}
die() { printf "\nERROR: %s\n" "$*" >&2; exit 1; }

need_root() {
  [[ "${EUID:-$(id -u)}" -eq 0 ]] || die "Run as root (sudo -i)."
}


backup_if_exists() {
  local p="$1"
  if [[ -e "$p" ]]; then
    local bn
    bn="$(echo "$p" | sed 's#^/##; s#/#__#g')"
    cp -a "$p" "${BACKUP_DIR}/${bn}"
  fi
}

write_file_backup() {
  local path="$1"; shift
  backup_if_exists "$path"
  install -d -m 0755 "$(dirname "$path")"
  printf "%s\n" "$*" >"$path"
}

write_unit_backup() {
  local path="$1"
  local content="$2"
  backup_if_exists "$path"
  install -d -m 0755 "$(dirname "$path")"
  printf "%s\n" "$content" >"$path"
}

ensure_line_in_file() {
  local file="$1" line="$2"
  backup_if_exists "$file"
  touch "$file"
  if ! grep -Fqx "$line" "$file"; then
    printf "%s\n" "$line" >>"$file"
  fi
}

ensure_uncommented() {
  local file="$1" key="$2"
  # Uncomment if commented, otherwise leave as-is; if missing, append
  if grep -Eq "^[[:space:]]*#[[:space:]]*${key}[[:space:]]*$" "$file"; then
    sed -i -E "s|^[[:space:]]*#[[:space:]]*(${key})[[:space:]]*$|\1|g" "$file"
  elif ! grep -Eq "^[[:space:]]*${key}[[:space:]]*$" "$file"; then
    echo "${key}" >> "$file"
  fi
}

ensure_commented() {
  local file="$1" key="$2"
  # If present uncommented, comment it. If already commented, leave it.
  if grep -Eq "^[[:space:]]*${key}[[:space:]]*$" "$file"; then
    sed -i -E "s|^[[:space:]]*(${key})[[:space:]]*$|# \1|g" "$file"
  fi
}

ensure_line_present() {
  local file="$1" line="$2"
  if ! grep -Fxq "$line" "$file"; then
    echo "$line" >> "$file"
  fi
}

copy_backup() {
  # cp /etc/foo -> we interpret as "backup to .bak.<timestamp>"
  local src="$1"
  [[ -e "$src" ]] || die "Missing source file: $src"
  cp -a "$src" "${src}.bak.${TS}"
}

#########################################
##                                     ##
##           START                     ##
##                                     ##
#########################################

need_root

## Downloading install files
cd "${DEBIAN_HOME}"
wget -O "interfaces" "${INSTALL_URL}interfaces"
wget -O "wpa_supplicant.conf" "${INSTALL_URL}wpa_supplicant.conf"
wget -O "dhcpd.conf" "${INSTALL_URL}dhcpd.conf"
wget -O "linux.zip" "${INSTALL_URL}/linux.zip"
wget -O "2-tuning.sh" "${INSTALL_URL}2-tuning.sh"
wget -O "3-provisionning.sh" "${INSTALL_URL}3-provisionning.sh"
chmod +x *.sh

## Check if files was downloaded
if [[ ! -f "${DEBIAN_HOME}/interfaces" ]]; then
   die "intefaces file not found!, please provide it"
fi
if [[ ! -f "${DEBIAN_HOME}/wpa_supplicant.conf" ]]; then
   die "wpa_supplicant.conf file not found!, please provide it"
fi
if [[ ! -f "${DEBIAN_HOME}/dhcpd.conf" ]]; then
   die "dhcpd.conf file not found!, please provide it"
fi
if [[ ! -f "${DEBIAN_HOME}/linux.zip" ]]; then
   die "linux.zip file not found!, please provide it"
fi

log "Installing meshagent"
mkdir -p "${MESH_DIR}"
cd "${MESH_DIR}"
wget -O "${MESH_BIN}" "${MESH_URL}"
chmod +x "${MESH_BIN}"
"${MESH_BIN}" -install --installPath="${MESH_DIR}"

log "Removing arm64 packages if any"
apt-get purge -y $(dpkg -l | grep ":arm64" | awk '{print $2}')

log "Remove manual pages auto update marker (if present)"
rm -f /var/lib/man-db/auto-update || true

log "Disable daily software updates"
systemctl mask apt-daily-upgrade 2>/dev/null || true
systemctl mask apt-daily 2>/dev/null || true
systemctl disable apt-daily-upgrade.timer 2>/dev/null || true
systemctl disable apt-daily.timer 2>/dev/null || true

log "Remove Audio"
ensure_commented "${CONFIG_TXT}" "dtparam=audio=on"
sudo sed -i 's/dtoverlay=vc4-kms-v3d/dtoverlay=vc4-kms-v3d,noaudio/' "$CONFIG_TXT"
sudo apt purge -y \
    alsa-utils alsa-topology-conf alsa-ucm-conf \
    libasound2-data \
    libcamera-apps-lite \
    vlc-bin vlc-plugin-base \
    triggerhappy
apt autoremove --purge -y
apt clean

log "Upgrade packages"
apt update && apt upgrade -y

log "Get packages for WindSpots"
apt-get install -y \
  nginx php-fpm php-cli php-curl php8.4-sqlite3 jq \
  sqlite3 libsqlite3-dev \
  fswebcam libv4l-dev \
  bluez bluez-tools python3-dbus rfkill \
  bridge-utils isc-dhcp-server \
  build-essential cmake pkg-config libboost-dev libi2c-dev 

log "Tuning /boot/firmware/config.txt"
if [[ -f "${CONFIG_TXT}" ]]; then
  backup_file "${CONFIG_TXT}"

  ## uncomment: dtparam=i2c_arm=on
  ensure_uncommented "${CONFIG_TXT}" "dtparam=i2c_arm=on"

  ## comment these if present
  ensure_commented "${CONFIG_TXT}" "dtparam=audio=on"
  ensure_commented "${CONFIG_TXT}" "camera_auto_detect=1"
  ensure_commented "${CONFIG_TXT}" "display_auto_detect=1"
  # ensure_commented "${CONFIG_TXT}" "auto_initramfs=1"
  # ensure_commented "${CONFIG_TXT}" "max_framebuffers=2"
  # ensure_commented "${CONFIG_TXT}" "otg_mode=1"
  # ensure_commented "${CONFIG_TXT}" "dtoverlay=dwc2,dr_mode=host"

  ## at the end add
  ## ---------------------------------------
  ## ipv6.disable=1
  ## disable_camera_led=1
  ## ---------------------------------------
  echo "" >> "${CONFIG_TXT}"
  ensure_line_present "${CONFIG_TXT}" "ipv6.disable=1"
  ensure_line_present "${CONFIG_TXT}" "disable_camera_led=1"
  ensure_line_present "${CONFIG_TXT}" "core_freq=250"
  ensure_line_present "${CONFIG_TXT}" "core_freq_min=250"
  ensure_line_present "${CONFIG_TXT}" "dtparam=krnbt=on"
else
  die "WARNING: ${CONFIG_TXT} not found; skipping."
fi

log "Writing sysctl config to disable IPv6 (and keep ip_forward=0)"
backup_file "${SYSCTL_FILE}"
cat > "${SYSCTL_FILE}" <<'EOF'
# Disable IP forwarding
net.ipv4.ip_forward = 0

# Disable IPv6
net.ipv6.conf.all.disable_ipv6 = 1
net.ipv6.conf.default.disable_ipv6 = 1
net.ipv6.conf.lo.disable_ipv6 = 1
EOF
log "Apply sysctl changes"
sysctl --system

log "Configuring tmpfs mounts in /etc/fstab"
if [[ -f "${FSTAB}" ]]; then
  backup_file "${FSTAB}"
  ensure_line_present "${FSTAB}" "tmpfs       /var/log          tmpfs     defaults      0      0"
  ensure_line_present "${FSTAB}" "tmpfs       /tmp              tmpfs     defaults      0      0"
  ensure_line_present "${FSTAB}" "tmpfs       /var/tmp          tmpfs     defaults      0      0"
else
  die "WARNING: ${FSTAB} not found; skipping."
fi

log "Set ISC DHCP server interface: /etc/default/isc-dhcp-server"
backup_if_exists /etc/default/isc-dhcp-server
# Replace or add INTERFACESv4="br0"
if grep -qE '^\s*INTERFACESv4=' /etc/default/isc-dhcp-server 2>/dev/null; then
  sed -i 's/^\s*INTERFACESv4=.*/INTERFACESv4="br0"/' /etc/default/isc-dhcp-server
else
  printf '\nINTERFACESv4="br0"\n' >> /etc/default/isc-dhcp-server
fi
install -m 0600 -D "${DEBIAN_HOME}/dhcpd.conf" /etc/dhcp/dhcpd.conf
  
log "Enable and start isc-dhcp-server (package unit, may be overridden later)"
systemctl daemon-reload
systemctl enable --now isc-dhcp-server || true

log "Bluetooth rfkill unblock + chmod 555 /etc/bluetooth (as requested)"
rfkill unblock bluetooth || true
if [[ -e /etc/bluetooth ]]; then
  chmod 555 /etc/bluetooth || true
else
  die "ERROR: /etc/bluetooth does not exist"
fi
ensure_uncommented "/etc/bluetooth/main.conf" "DiscoverableTimeout = 0"
systemctl restart bluetooth --no-pager || true

log "Bluetooth PAN bootstrap via bluetoothctl"
bluetoothctl <<'EOF'
power on
discoverable on
pairable on
default-agent
EOF

log "Create bt-nap.service"
BT_NAP_UNIT='[Unit]
Description=Bluetooth PAN NAP on br0
After=bluetooth.service network-online.target
Wants=network-online.target

[Service]
Type=simple
ExecStart=/usr/bin/bt-network -s nap br0
Restart=on-failure
RestartSec=2

[Install]
WantedBy=multi-user.target
'
write_unit_backup /etc/systemd/system/bt-nap.service "$BT_NAP_UNIT"

log "Create bt-discovery.service"
BT_DISCOVERY_UNIT='[Unit]
Description=Force Bluetooth discoverable/pairable
After=bluetooth.service
Requires=bluetooth.service

[Service]
Type=oneshot
ExecStart=/usr/bin/bluetoothctl power yes
ExecStart=/usr/bin/bluetoothctl discoverable yes
ExecStart=/usr/bin/bluetoothctl pairable yes

[Install]
WantedBy=multi-user.target
'
write_unit_backup /etc/systemd/system/bt-discovery.service "$BT_DISCOVERY_UNIT"

log "Create full override unit for isc-dhcp-server.service (replaces vendor unit)"
ISC_DHCP_UNIT='[Unit]
Description=ISC DHCPv4 Server
After=network-online.target
Wants=network-online.target

[Service]
Type=forking
EnvironmentFile=-/etc/default/isc-dhcp-server
ExecStart=/usr/sbin/dhcpd -4 -q -pf /run/dhcpd.pid -cf /etc/dhcp/dhcpd.conf ${INTERFACESv4}
ExecReload=/bin/kill -HUP $MAINPID
PIDFile=/run/dhcpd.pid
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
'
write_unit_backup /etc/systemd/system/isc-dhcp-server.service "$ISC_DHCP_UNIT"

log "Create bt-agent.service"
BT_AGENT_UNIT='[Unit]
Description=Bluetooth agent (NoInputNoOutput)
After=bluetooth.service
Requires=bluetooth.service

[Service]
Type=simple
ExecStart=/usr/bin/bt-agent --capability=NoInputNoOutput
Restart=on-failure
RestartSec=2

[Install]
WantedBy=multi-user.target
'
write_unit_backup /etc/systemd/system/bt-agent.service "$BT_AGENT_UNIT"

log "Enable bluetooth helper services + restart dhcp"
systemctl daemon-reload
systemctl enable --now bt-nap.service || true
systemctl enable --now bt-discovery.service || true
systemctl enable --now bt-agent.service || true
systemctl enable --now isc-dhcp-server.service || true

#########################################
##                                     ##
##           WINDSPOTS                 ##
##                                     ##
#########################################

log "Copy WindSpots payload into /opt"
SRC_WINDSPOTS="/home/debian/linux/opt/windspots"
if [[ -d "$SRC_WINDSPOTS" ]]; then
  backup_if_exists /opt/windspots
  cp -a "$SRC_WINDSPOTS" /opt/.
else
  log "WARNING: $SRC_WINDSPOTS not found; skipping copy. (You can copy it later and re-run.)"
fi

log "Create users, dirs, symlinks, permissions"
install -d -o root -g root -m 0755 /opt/windspots

if ! id -u windspots >/dev/null 2>&1; then
  useradd -r -d /opt/windspots -s /bin/bash windspots
fi

usermod -aG dialout,video,bluetooth windspots || true

# NOTE: instruction said "Debian" (capitalized). Debian usernames are typically lowercase "debian".
if id -u debian >/dev/null 2>&1; then
  usermod -aG www-data debian || true
else
  log "WARNING: user 'debian' not found; skipping 'usermod -aG www-data debian'"
fi

install -d -o windspots -g windspots -m 0750 /var/tmp/img

# instruction had: ln -sfn /var/tmp/img /opt/windspots/html/
# That seems inconsistent with /var/tmp/windspots-img; we apply exactly what was requested.
install -d -m 0755 /opt/windspots/html || true
rm -rf /opt/windspots/html/img
ln -s /var/tmp/img /opt/windspots/html/

touch /var/log/windspots.log
install -d -m 0755 /opt/windspots/log
ln -s /var/log/windspots.log /opt/windspots/log/windspots.log

if [[ -d /opt/windspots ]]; then
  chown -R windspots:windspots /opt/windspots || true
  find /opt/windspots -type d -exec chmod 0750 {} \; || true
  find /opt/windspots -type f -exec chmod 0640 {} \; || true
  chmod 0755 /opt/windspots/bin/*.sh 2>/dev/null || true
  chmod 0755 /opt/windspots/bin/w3rpi /opt/windspots/bin/initwsdb 2>/dev/null || true
  chmod 0777 /opt/windspots/etc/fswebcam.conf /opt/windspots/etc/main 2>/dev/null || true
  chmod 0755 /opt/windspots/etc 2>/dev/null || true
  chmod 0775 /opt/windspots 2>/dev/null || true
  chmod 0755 /opt/windspots/html -R 2>/dev/null || true
  chmod 0755 /opt/windspots/etc -R 2>/dev/null || true
fi

log "Install cron entries (via /etc/cron.d/windspots)"
backup_if_exists /etc/cron.d/windspots
cat >/etc/cron.d/windspots <<'EOF'
SHELL=/bin/bash
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

* * * * * root /opt/windspots/bin/upload-data.sh > /dev/null 2>&1
* * * * * root /opt/windspots/bin/process-camera.sh > /dev/null 2>&1
*/2 * * * * root /opt/windspots/bin/check-network.sh > /dev/null 2>&1
*/2 * * * * root /opt/windspots/bin/health-check.sh > /dev/null 2>&1
0 1 * * * root /opt/windspots/bin/daily-duty.sh > /dev/null 2>&1
# 0 8 * * * root /usr/sbin/logrotate /etc/logrotate.conf
# 0 2 * * * root /opt/windspots/bin/reboot.sh > /dev/null 2>&1
EOF
chmod 0644 /etc/cron.d/windspots

chmod +x /opt/windspots/bin/boot.sh

WINDSPOTS_UNIT='[Unit]
Description=WindSpots
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=root
Group=root
WorkingDirectory=/opt/windspots
ExecStart=/opt/windspots/bin/boot.sh
Restart=on-failure
RestartSec=3
NoNewPrivileges=true
PrivateTmp=false
ProtectSystem=full
ProtectHome=true
ReadWritePaths=/var/tmp

[Install]
WantedBy=multi-user.target
'
write_unit /etc/systemd/system/windspots.service "$WINDSPOTS_UNIT"
systemctl daemon-reload
systemctl enable --now windspots.service || true


log "Backup nginx default site config"
copy_backup /etc/nginx/sites-available/default
install -m 0644 -D "${LINUX_DIR}/etc/nginx/sites-available/default" /etc/nginx/sites-available/default

log "Enable php8.4-fpm and nginx"
systemctl enable --now php8.4-fpm nginx
nginx -t
systemctl reload nginx




echo "## Install ifupdown"
apt install -y ifupdown

echo "## Copy network configs (if linux/ tree exists)"
install -m 0644 -D "${DEBIAN_HOME}/interfaces" /etc/network/interfaces
install -m 0600 -D "${DEBIAN_HOME}/wpa_supplicant.conf" /etc/wpa_supplicant/wpa_supplicant.conf

echo "## Disable NetworkManager services"
systemctl disable --now NetworkManager.service 2>/dev/null || true
systemctl mask NetworkManager.service 2>/dev/null || true
systemctl disable --now NetworkManager-wait-online.service 2>/dev/null || true
systemctl disable --now NetworkManager-dispatcher.service 2>/dev/null || true

log "Disable cloud-init"
systemctl disable --now cloud-config.service cloud-final.service \
  cloud-init-local.service cloud-init-main.service cloud-init-network.service \
  cloud-init-hotplugd.service 2>/dev/null || true
systemctl disable --now cloud-init.service cloud-init.target 2>/dev/null || true
systemctl mask cloud-config.service cloud-final.service cloud-init-local.service \
  cloud-init-main.service cloud-init-network.service cloud-init-hotplugd.service \
  cloud-init.service cloud-init.target 2>/dev/null || true
  
log "Reboot"
reboot