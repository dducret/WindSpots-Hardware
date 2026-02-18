#!/usr/bin/env bash
set -euo pipefail

# Raspberry Pi / Debian Trixie provisioning for WindSpots
# - Idempotent-ish: backs up files before overwriting; safe to re-run.
## Must run as root
if [[ "${EUID}" -ne 0 ]]; then
  echo "ERROR: run as root (sudo -s)"; exit 1
fi

DEBIAN_HOME="/home/debian"
LINUX_ZIP="${DEBIAN_HOME}/linux.zip"
LINUX_DIR="${DEBIAN_HOME}/linux"

TS="$(date +%Y%m%d-%H%M%S)"
BACKUP_DIR="/root/windspots-setup-backups-${TS}"
mkdir -p "$BACKUP_DIR"

log() { printf "\n[%s] %s\n" "$(date +'%F %T')" "$*"; }
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
  local path="$1"
  shift
  backup_if_exists "$path"
  install -d -m 0755 "$(dirname "$path")"
  cat >"$path" <<'EOF'
'"$*"'
EOF
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

copy_backup() {
  # cp /etc/foo -> we interpret as "backup to .bak.<timestamp>"
  local src="$1"
  [[ -e "$src" ]] || die "Missing source file: $src"
  cp -a "$src" "${src}.bak.${TS}"
}

command_exists() { command -v "$1" >/dev/null 2>&1; }

need_root

echo "## Unzipping linux.zip into /home/debian"
if [[ -f "${LINUX_ZIP}" ]]; then
  cd "${DEBIAN_HOME}"
  rm -rf "${LINUX_DIR}" || true
  unzip -o "${LINUX_ZIP}"
else
  echo "WARNING: ${LINUX_ZIP} not found. Skipping unzip."
  echo "         Place linux.zip in ${DEBIAN_HOME} and re-run if needed."
fi


log "APT update + install packages"
export DEBIAN_FRONTEND=noninteractive
apt-get update -y
apt-get install -y \
  nginx php-fpm php-cli php-curl php8.4-sqlite3 jq \
  sqlite3 libsqlite3-dev \
  fswebcam libv4l-dev \
  bluez bluez-tools python3-dbus \
  bridge-utils isc-dhcp-server \
  build-essential cmake pkg-config libboost-dev libi2c-dev \
  pigpio-tools pigpio

log "Set ISC DHCP server interface: /etc/default/isc-dhcp-server"
backup_if_exists /etc/default/isc-dhcp-server
# Replace or add INTERFACESv4="br0"
if grep -qE '^\s*INTERFACESv4=' /etc/default/isc-dhcp-server 2>/dev/null; then
  sed -i 's/^\s*INTERFACESv4=.*/INTERFACESv4="br0"/' /etc/default/isc-dhcp-server
else
  printf '\nINTERFACESv4="br0"\n' >> /etc/default/isc-dhcp-server
fi

log "Enable and start isc-dhcp-server (package unit, may be overridden later)"
systemctl daemon-reload
systemctl enable --now isc-dhcp-server || true
systemctl status isc-dhcp-server --no-pager || true

# Enable Bluetook
rfkill unblock bluetooth

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
systemctl daemon-reload
systemctl enable --now bt-nap.service
systemctl status bt-nap.service --no-pager || true


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
fi

log "Create windspots.service"
WINDSPOTS_UNIT='[Unit]
[Unit]
Description=WindSpots
After=var-tmp.mount
Requires=var-tmp.mount

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
write_unit_backup /etc/systemd/system/windspots.service "$WINDSPOTS_UNIT"
systemctl daemon-reload
systemctl enable --now windspots.service || true

log "Create systemd drop-in for isc-dhcp-server.service restart policy"
install -d -m 0755 /etc/systemd/system/isc-dhcp-server.service.d
backup_if_exists /etc/systemd/system/isc-dhcp-server.service.d/override.conf
cat >/etc/systemd/system/isc-dhcp-server.service.d/override.conf <<'EOF'
[Service]
Restart=on-failure
RestartSec=5
EOF
systemctl daemon-reload
systemctl enable --now isc-dhcp-server.service || true

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

log "Backup nginx default site config"
copy_backup /etc/nginx/sites-available/default
install -m 0644 -D "${LINUX_DIR}/etc/nginx/sites-available/default" /etc/nginx/sites-available/default

log "Enable php8.4-fpm and nginx"
systemctl enable --now php8.4-fpm nginx
nginx -t
systemctl reload nginx

log "Bluetooth rfkill unblock + chmod 555 /etc/bluetooth (as requested)"
rfkill unblock bluetooth || true
if [[ -e /etc/bluetooth ]]; then
  chmod 555 /etc/bluetooth || true
else
  log "WARNING: /etc/bluetooth does not exist; skipping chmod"
fi

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
systemctl enable --now bt-discovery.service || true
systemctl enable --now bt-agent.service || true
systemctl enable --now isc-dhcp-server.service || true

log "Disable cloud-init"
systemctl disable --now cloud-config.service cloud-final.service \
  cloud-init-local.service cloud-init-main.service cloud-init-network.service \
  cloud-init-hotplugd.service 2>/dev/null || true
systemctl disable --now cloud-init.service cloud-init.target 2>/dev/null || true
systemctl mask cloud-config.service cloud-final.service cloud-init-local.service \
  cloud-init-main.service cloud-init-network.service cloud-init-hotplugd.service \
  cloud-init.service cloud-init.target 2>/dev/null || true

log "========== RESULT =========="
echo "Backups saved in: $BACKUP_DIR"
echo
echo "Created/updated:"
printf " - %s\n" \
  "/etc/default/isc-dhcp-server" \
  "/etc/systemd/system/bt-nap.service" \
  "/etc/systemd/system/windspots.service" \
  "/etc/systemd/system/isc-dhcp-server.service.d/override.conf" \
  "/etc/cron.d/windspots" \
  "/etc/systemd/system/bt-discovery.service" \
  "/etc/systemd/system/isc-dhcp-server.service" \
  "/etc/systemd/system/bt-agent.service"
echo
echo "Service status (no-pager):"
systemctl status nginx php8.4-fpm bt-nap.service bt-discovery.service bt-agent.service windspots.service isc-dhcp-server.service --no-pager || true
echo
echo "Nginx config test:"
nginx -t || true
echo
echo "Cron file:"
sed -n '1,200p' /etc/cron.d/windspots || true
echo "============================"
