#!/usr/bin/env bash
# install.sh
set -euo pipefail

## Must run as root
if [[ "${EUID}" -ne 0 ]]; then
  echo "ERROR: run as root (sudo -s)"; exit 1
fi

CONFIG_TXT="/boot/firmware/config.txt"
CMDLINE_TXT="/boot/firmware/cmdline.txt"
DEBIAN_HOME="/home/debian"
FSTAB="/etc/fstab"
INSTALL_URL='https://station.windspots.org/install/'
REPOSITORY_ARCHIVE_URL='https://github.com/dducret/WindSpots-Hardware/archive/refs/heads/main.zip'
REPOSITORY_DIR="${DEBIAN_HOME}/WindSpots-Hardware-main"
REPOSITORY_ZIP="${DEBIAN_HOME}/WindSpots-Hardware-main.zip"
LINUX_DIR="${REPOSITORY_DIR}/linux"
MESH_DIR="/opt/meshagent"
MESH_BIN="${MESH_DIR}/meshagent"
MESH_URL='https://mc.windspots.org:444/meshagents?id=c%40BuzaU7IfiIFtx6dIiDjgb479zsNloSnUaeXoLJQ3hgiqj5VS4IM1O9GzJbLjKF&installflags=2&meshinstall=25'
PHP_CLI="/etc/php/8.4/cli/php.ini"
PHP_FPM="/etc/php/8.4/fpm/php.ini"
PHP_FPM_SERVICE="php8.4-fpm.service"
SYSCTL_FILE="/etc/sysctl.d/98-rpi.conf"
STATE_DIR="/var/lib/windspots-install"
STATE_FILE="${STATE_DIR}/state"
NO_REBOOT=0
RESET_STATE=0

export DEBIAN_FRONTEND=noninteractive

TS="$(date +%Y%m%d-%H%M%S)"
BACKUP_DIR="${DEBIAN_HOME}/windspots-setup-backups-${TS}"
mkdir -p "$BACKUP_DIR"
LOGFILE="${DEBIAN_HOME}/log-${TS}.txt"
touch "$LOGFILE"
exec > >(tee -a "$LOGFILE") 2>&1

log() {
  local msg="[$(date +'%F %T')] $*"
  printf "\n%s\n" "$msg"
}
die() { printf "\nERROR: %s\n" "$*" >&2; exit 1; }

usage() {
  cat <<'USAGE'
Usage: ./install.sh [options]

Options:
  --no-reboot     Run the installation and validation without rebooting at the end.
  --reset-state   Clear the completed-step state before running.
  -h, --help      Show this help.
USAGE
}

parse_args() {
  while [[ "$#" -gt 0 ]]; do
    case "$1" in
      --no-reboot)
        NO_REBOOT=1
        ;;
      --reset-state)
        RESET_STATE=1
        ;;
      -h|--help)
        usage
        exit 0
        ;;
      *)
        die "Unknown option: $1"
        ;;
    esac
    shift
  done
}

need_root() {
  [[ "${EUID:-$(id -u)}" -eq 0 ]] || die "Run as root (sudo -i)."
}

init_state() {
  install -d -m 0755 "${STATE_DIR}"
  if [[ "${RESET_STATE}" -eq 1 ]]; then
    log "Reset install state: ${STATE_FILE}"
    rm -f "${STATE_FILE}"
  fi
  touch "${STATE_FILE}"
  log "Install state file: ${STATE_FILE}"
}

step_completed() {
  local step="$1"
  [[ -f "${STATE_FILE}" ]] && grep -Fxq "${step}" "${STATE_FILE}"
}

mark_step_completed() {
  local step="$1"
  if ! step_completed "${step}"; then
    printf '%s\n' "${step}" >> "${STATE_FILE}"
  fi
}

run_step() {
  local step="$1"
  local label="$2"
  shift 2

  if step_completed "${step}"; then
    log "Skipping completed step: ${label} (${step})"
    return 0
  fi

  log "Starting step: ${label} (${step})"
  "$@"
  mark_step_completed "${step}"
  log "Completed step: ${label} (${step})"
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

backup_file() {
  local src="$1"
  [[ -e "$src" ]] || die "Missing source file: $src"
  cp -a "$src" "${src}.bak.${TS}"
}

#########################################
##                                     ##
##           START                     ##
##                                     ##
#########################################

download_install_files() {
  log "Downloading install files"
  cd "${DEBIAN_HOME}"
  wget -O "interfaces" "${INSTALL_URL}interfaces"
  wget -O "wpa_supplicant.conf" "${INSTALL_URL}wpa_supplicant.conf"
  wget -O "dhcpd.conf" "${INSTALL_URL}dhcpd.conf"
  wget -O "${REPOSITORY_ZIP}" "${REPOSITORY_ARCHIVE_URL}"

  log "Checking downloaded install files"
  [[ -f "${DEBIAN_HOME}/interfaces" ]] || die "interfaces file not found!, please provide it"
  [[ -f "${DEBIAN_HOME}/wpa_supplicant.conf" ]] || die "wpa_supplicant.conf file not found!, please provide it"
  [[ -f "${DEBIAN_HOME}/dhcpd.conf" ]] || die "dhcpd.conf file not found!, please provide it"
  [[ -f "${REPOSITORY_ZIP}" ]] || die "Repository archive not found: ${REPOSITORY_ZIP}"
}

configure_i2c_and_firmware() {
  log "raspi-config to activate i2c correctly"
  raspi-config

  log "rpi-update Mandatory for i2c"
  rpi-update
}

install_meshagent() {
  log "Installing meshagent"
  local mesh_bin_new="${MESH_BIN}.new"
  local meshagent_active=0

  mkdir -p "${MESH_DIR}"
  rm -f "${mesh_bin_new}"
  wget -O "${mesh_bin_new}" "${MESH_URL}"
  chmod 0755 "${mesh_bin_new}"

  if systemctl is-active --quiet meshagent.service; then
    meshagent_active=1
  fi

  mv -f "${mesh_bin_new}" "${MESH_BIN}"

  if [[ "${meshagent_active}" -eq 1 ]]; then
    log "MeshAgent is already running; the new binary will be used after restart"
  else
    "${MESH_BIN}" -install --installPath="${MESH_DIR}"
    systemctl restart meshagent.service
  fi
}

prepare_system_packages() {
  log "Removing arm64 packages if any"
  local arm64_packages
  arm64_packages="$(dpkg -l | awk '/:arm64/ {print $2}')"
  if [[ -n "${arm64_packages}" ]]; then
    apt purge -y ${arm64_packages}
  fi

  log "Remove manual pages auto update marker (if present)"
  rm -f /var/lib/man-db/auto-update || true

  log "Disable daily software updates"
  systemctl mask apt-daily-upgrade 2>/dev/null || true
  systemctl mask apt-daily 2>/dev/null || true
  systemctl disable apt-daily-upgrade.timer 2>/dev/null || true
  systemctl disable apt-daily.timer 2>/dev/null || true

  log "Remove Audio"
  ensure_commented "${CONFIG_TXT}" "dtparam=audio=on"
  sed -i 's/dtoverlay=vc4-kms-v3d/dtoverlay=vc4-kms-v3d,noaudio/' "$CONFIG_TXT"
  apt purge -y \
      libcamera-apps-lite \
      vlc-bin vlc-plugin-base \
      triggerhappy
  apt autoremove --purge -y
  apt clean

  log "Upgrade packages"
  apt update && apt full-upgrade -y

  log "Get packages for WindSpots"
  apt install -y \
    sudo nginx php-fpm php-cli php-curl php8.4-sqlite3 jq \
    sqlite3 libsqlite3-dev \
    fswebcam libv4l-dev \
    bluez bluez-tools python3-dbus rfkill \
    bridge-utils isc-dhcp-server \
    build-essential cmake pkg-config libboost-dev libi2c-dev
}

configure_boot_files() {
log "Tuning /boot/firmware/config.txt"
if [[ -f "${CONFIG_TXT}" ]]; then
  backup_file "${CONFIG_TXT}"

  ## Not realy working - replaced by raspi-config at the beginning
  ## Could be invetigated
  ## ensure_uncommented "${CONFIG_TXT}" "dtparam=i2c_arm=on"

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

log "Fixing cmdline.txt for bluetooth for RPI 3B"
SERIAL_TOKEN="console=serial0,115200"
TTY_TOKEN="console=tty1"
backup_if_exists "${CMDLINE_TXT}"
current="$(tr '\n' ' ' < "${CMDLINE_TXT}" | sed -E 's/[[:space:]]+/ /g; s/^ //; s/ $//')"
updated="$(printf '%s\n' "$current" | sed -E "s/(^| )(${SERIAL_TOKEN}|${TTY_TOKEN})( |$)/ /g; s/[[:space:]]+/ /g; s/^ //; s/ $//")"
updated="${SERIAL_TOKEN} ${TTY_TOKEN}${updated:+ ${updated}}"
if [[ "$current" == "$updated" ]]; then
  log "    No change needed: ${CMDLINE_TXT} already contains ${SERIAL_TOKEN} ${TTY_TOKEN}"
else
  printf '%s\n' "$updated" > "${CMDLINE_TXT}"
  log "    Ensured ${SERIAL_TOKEN} ${TTY_TOKEN} in ${CMDLINE_TXT}"
fi

log "Writing sysctl config to disable IPv6 (and keep ip_forward=0)"
backup_if_exists "${SYSCTL_FILE}"
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
}

configure_dhcp() {
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
}

configure_bluetooth() {
log "Bluetooth rfkill unblock + chmod 555 /etc/bluetooth (as requested)"
rfkill unblock bluetooth || true
if [[ -e /etc/bluetooth ]]; then
	ensure_uncommented "/etc/bluetooth/main.conf" "DiscoverableTimeout = 0"
  chmod 555 /etc/bluetooth || true
else
  die "ERROR: /etc/bluetooth does not exist"
fi
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
ExecStart=/usr/bin/bluetoothctl power on
ExecStart=/usr/bin/bluetoothctl discoverable on
ExecStart=/usr/bin/bluetoothctl pairable on

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
}

#########################################
##                                     ##
##           WINDSPOTS                 ##
##                                     ##
#########################################

install_windspots_payload() {
log "Unzipping the Git repository into /home/debian"
if [[ -f "${REPOSITORY_ZIP}" ]]; then
  cd "${DEBIAN_HOME}"
  rm -rf "${REPOSITORY_DIR}"
  unzip -o "${REPOSITORY_ZIP}"
else
  die "Repository archive not found: ${REPOSITORY_ZIP}"
fi

log "Copy WindSpots payload into /opt"
SRC_WINDSPOTS="${LINUX_DIR}/opt/windspots"
if [[ -d "$SRC_WINDSPOTS" ]]; then
  backup_if_exists /opt/windspots
  cp -a "$SRC_WINDSPOTS" /opt/.
else
  log "WARNING: $SRC_WINDSPOTS not found; skipping copy. (You can copy it later and re-run.)"
fi
}

configure_windspots_runtime() {
log "Create users, dirs, symlinks, permissions"
install -d -o root -g root -m 0755 /opt/windspots

if ! getent group windspots >/dev/null 2>&1; then
  groupadd --system windspots
fi

if ! id -u windspots >/dev/null 2>&1; then
  useradd -r -g windspots -d /opt/windspots -s /bin/bash windspots
fi

  usermod -g windspots windspots
  usermod -aG dialout,video,bluetooth windspots || true
  usermod -aG windspots www-data || true

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
ln -sfn /var/tmp/img /opt/windspots/html/img

  touch /var/log/windspots.log
  chown windspots:www-data /var/log/windspots.log
  chmod 0664 /var/log/windspots.log
  install -d -o windspots -g www-data -m 0775 /opt/windspots/log
  ln -sfn /var/log/windspots.log /opt/windspots/log/windspots.log

  if [[ -d /opt/windspots ]]; then
	chmod o+rx /opt 2>/dev/null || true
  chmod o+rx /opt/windspots 2>/dev/null || true
  chmod o+x  /opt/windspots/bin /opt/windspots/bin/cpp /opt/windspots/bin/cpp/WS200 2>/dev/null || true
  chown -R windspots:windspots /opt/windspots || true
  chmod 0755 /opt/windspots/bin/*.sh 2>/dev/null || true
  install -d -o windspots -g www-data -m 2775 /opt/windspots/etc
  touch /opt/windspots/etc/wpa
  chown -R windspots:www-data /opt/windspots/etc 2>/dev/null || true
  find /opt/windspots/etc -type d -exec chmod 2775 {} \; 2>/dev/null || true
  find /opt/windspots/etc -type f -exec chmod 0664 {} \; 2>/dev/null || true
  chown -R windspots:www-data /opt/windspots/log 2>/dev/null || true
  chmod 0775 /opt/windspots/log 2>/dev/null || true
  chmod 0755 /opt/windspots/html -R 2>/dev/null || true
fi

log "Install limited sudo policy for WUI management actions"
cat >/etc/sudoers.d/windspots-wui <<'EOF'
www-data ALL=(root) NOPASSWD: /opt/windspots/bin/eth0.sh up, /opt/windspots/bin/eth0.sh down
www-data ALL=(root) NOPASSWD: /opt/windspots/bin/wlan0.sh up, /opt/windspots/bin/wlan0.sh down
www-data ALL=(root) NOPASSWD: /opt/windspots/bin/ppp0.sh up, /opt/windspots/bin/ppp0.sh down
www-data ALL=(root) NOPASSWD: /opt/windspots/bin/ws-configure.sh
www-data ALL=(root) NOPASSWD: /usr/bin/php /opt/windspots/bin/php/generate_wpa.php
EOF
chmod 0440 /etc/sudoers.d/windspots-wui
visudo -cf /etc/sudoers.d/windspots-wui

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
ReadWritePaths=/var/tmp /var/log/windspots.log /opt/windspots/log

[Install]
WantedBy=multi-user.target
'
write_unit_backup /etc/systemd/system/windspots.service "$WINDSPOTS_UNIT"
systemctl daemon-reload
systemctl enable windspots.service
}

windspots_config_value() {
  local name="$1"
  (
    set +u
    # shellcheck disable=SC1091
    . /opt/windspots/etc/main
    printf '%s' "${!name}"
  )
}

weather_database_has_schema() {
  local db_path="$1"
  local table_count

  table_count="$(sqlite3 "${db_path}" \
    "SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name IN ('data', 'log');")"
  [[ "${table_count}" == "2" ]]
}

weather_database_is_writable_by() {
  local db_path="$1"
  local user="$2"

  runuser -u "${user}" -- sqlite3 -cmd '.timeout 5000' "${db_path}" \
    "BEGIN IMMEDIATE;
     INSERT INTO data (last_update, name, channel, battery, temperature, temperature_sign,
                       relative_humidity, barometer, wind_direction, wind_speed, wind_speed_average)
     VALUES (CURRENT_TIMESTAMP, 'INSTALL-TEST', 0, 100, 20, '0', 50, 1013, 180, 5, 4);
     ROLLBACK;"
}

prepare_weather_database() {
  local station
  local tmp_dir
  local log_dir
  local db_path

  station="$(windspots_config_value STATION)"
  tmp_dir="$(windspots_config_value TMP)"
  log_dir="$(windspots_config_value LOG)"
  db_path="${tmp_dir}/ws.db"

  log "Initialize and validate the weather database"
  install -d -m 1777 "${tmp_dir}"
  chmod 1777 "${tmp_dir}"
  /opt/windspots/bin/initwsdb -s "${station}" -l "${log_dir}" -t "${tmp_dir}"
  chown windspots:www-data "${db_path}"
  chmod 0664 "${db_path}"

  weather_database_has_schema "${db_path}" || \
    die "Weather database schema is missing from ${db_path}"
  weather_database_is_writable_by "${db_path}" windspots || \
    die "Weather database is not writable by windspots: ${db_path}"
  weather_database_is_writable_by "${db_path}" www-data || \
    die "Weather database is not writable by www-data: ${db_path}"

  log "Start WindSpots service"
  systemctl restart windspots.service
}

configure_web_stack() {
log "Backup nginx default site config"
backup_file /etc/nginx/sites-available/default
install -m 0644 -D "${LINUX_DIR}/etc/nginx/sites-available/default" /etc/nginx/sites-available/default

log "Enable nginx using i2c"
usermod -aG i2c www-data

chmod -R 755 /opt/windspots/log

log "Enable php8.4-fpm and nginx"
systemctl enable --now php8.4-fpm nginx
nginx -t
systemctl reload nginx

log "## Generate exe and copy to bin"
chmod +x /opt/windspots/bin/cpp/makeall.sh
/opt/windspots/bin/cpp/makeall.sh -B

log "## Set time zone for php cli"
ensure_line_present "${PHP_CLI}" "date.timezone = Europe/Zurich"
ensure_line_present "${PHP_FPM}" "date.timezone = Europe/Zurich"

log "## Configure station"
/opt/windspots/bin/ws-configure.sh
}

configure_network_stack() {
log "## Install ifupdown"
apt install -y ifupdown

log "## Copy network configs (if linux/ tree exists)"
install -m 0644 -D "${DEBIAN_HOME}/interfaces" /etc/network/interfaces
install -m 0600 -D "${DEBIAN_HOME}/wpa_supplicant.conf" /etc/wpa_supplicant/wpa_supplicant.conf
chown root:www-data /etc/wpa_supplicant/wpa_supplicant.conf
chmod 0660 /etc/wpa_supplicant/wpa_supplicant.conf

log "## Set Huawei 4G USB Dongle"
printf '%s\n' 'SUBSYSTEM=="net", ACTION=="add", DRIVERS=="?*", ATTR{type}=="1", KERNEL=="usb*", NAME="eth1"' > /etc/udev/rules.d/70-usb-to-eth1.rules

log "## Disable NetworkManager services"
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
}

validate_installation() {
  log "Validating WindSpots installation"
  local failures=0
  local weather_db

  weather_db="$(windspots_config_value TMP)/ws.db"

  require_path() {
    local label="$1"
    local path="$2"
    if [[ -e "${path}" ]]; then
      log "OK: ${label}: ${path}"
    else
      log "ERROR: missing ${label}: ${path}"
      failures=1
    fi
  }

  require_executable() {
    local label="$1"
    local path="$2"
    if [[ -x "${path}" ]]; then
      log "OK: ${label}: ${path}"
    else
      log "ERROR: not executable ${label}: ${path}"
      failures=1
    fi
  }

  require_symlink() {
    local label="$1"
    local path="$2"
    if [[ -L "${path}" ]]; then
      log "OK: ${label}: ${path}"
    else
      log "ERROR: missing symlink ${label}: ${path}"
      failures=1
    fi
  }

  require_command() {
    local label="$1"
    shift
    if "$@"; then
      log "OK: ${label}"
    else
      log "ERROR: validation command failed: ${label}"
      failures=1
    fi
  }

  require_service_enabled() {
    local service="$1"
    if systemctl is-enabled --quiet "${service}"; then
      log "OK: enabled service ${service}"
    else
      log "ERROR: service is not enabled: ${service}"
      failures=1
    fi
  }

  require_path "WindSpots directory" /opt/windspots
  require_path "WindSpots configuration" /opt/windspots/etc/main
  require_executable "WindSpots boot script" /opt/windspots/bin/boot.sh
  require_executable "WindSpots main binary" /opt/windspots/bin/w3rpi
  require_executable "WindSpots database initializer" /opt/windspots/bin/initwsdb
  require_symlink "HTML image directory" /opt/windspots/html/img
  require_symlink "WindSpots log link" /opt/windspots/log/windspots.log
  require_path "network interfaces" /etc/network/interfaces
  require_path "wpa supplicant config" /etc/wpa_supplicant/wpa_supplicant.conf
  require_path "DHCP config" /etc/dhcp/dhcpd.conf
  require_path "nginx default site" /etc/nginx/sites-available/default
  require_path "PHP CLI config" "${PHP_CLI}"
  require_path "PHP FPM config" "${PHP_FPM}"
  require_path "WUI sudoers policy" /etc/sudoers.d/windspots-wui
  require_path "WindSpots weather database" "${weather_db}"

  require_command "nginx configuration" nginx -t
  require_command "WUI sudoers policy" visudo -cf /etc/sudoers.d/windspots-wui
  require_command "weather database schema" weather_database_has_schema "${weather_db}"
  require_command "weather database writable by windspots" weather_database_is_writable_by "${weather_db}" windspots
  require_command "weather database writable by www-data" weather_database_is_writable_by "${weather_db}" www-data
  if command -v dhcpd >/dev/null 2>&1; then
    require_command "DHCP configuration" dhcpd -t -cf /etc/dhcp/dhcpd.conf
  else
    log "ERROR: dhcpd command not found"
    failures=1
  fi

  require_service_enabled nginx.service
  require_service_enabled "${PHP_FPM_SERVICE}"
  require_service_enabled isc-dhcp-server.service
  require_service_enabled bt-nap.service
  require_service_enabled bt-discovery.service
  require_service_enabled bt-agent.service
  require_service_enabled windspots.service

  if [[ "${failures}" -ne 0 ]]; then
    die "Installation validation failed. Check ${LOGFILE} before rebooting."
  fi

  log "Installation validation completed successfully"
}

main() {
  parse_args "$@"
  need_root
  log "Installer options: reset-state=${RESET_STATE}, no-reboot=${NO_REBOOT}"
  init_state

  run_step "download_install_files" "Download install files" download_install_files
  run_step "configure_i2c_and_firmware" "Configure I2C and firmware" configure_i2c_and_firmware
  run_step "install_meshagent" "Install meshagent" install_meshagent
  run_step "prepare_system_packages" "Prepare system packages" prepare_system_packages
  run_step "configure_boot_files" "Configure boot files" configure_boot_files
  run_step "configure_dhcp" "Configure DHCP" configure_dhcp
  run_step "configure_bluetooth" "Configure Bluetooth" configure_bluetooth
  run_step "install_windspots_payload" "Install WindSpots payload" install_windspots_payload
  run_step "configure_windspots_runtime" "Configure WindSpots runtime" configure_windspots_runtime
  run_step "configure_web_stack" "Configure web stack and build binaries" configure_web_stack
  run_step "prepare_weather_database" "Initialize and validate weather database" prepare_weather_database
  run_step "configure_network_stack" "Configure network stack" configure_network_stack

  validate_installation

  if [[ "${NO_REBOOT}" -eq 1 ]]; then
    log "Skipping reboot because --no-reboot was provided"
  else
    log "Requesting reboot through systemd"
    systemctl reboot || die "Failed to request reboot through systemd"
  fi
}

main "$@"
