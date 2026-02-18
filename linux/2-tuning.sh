#!/usr/bin/env bash
# 02-windspots-tuning.sh
set -euo pipefail

## Must run as root
if [[ "${EUID}" -ne 0 ]]; then
  echo "ERROR: run as root (sudo -s)"; exit 1
fi

CONFIG_TXT="/boot/firmware/config.txt"
SYSCTL_FILE="/etc/sysctl.d/98-rpi.conf"
FSTAB="/etc/fstab"

backup_file() {
  local f="$1"
  if [[ -f "$f" ]]; then
    cp -a "$f" "${f}.bak.$(date +%Y%m%d-%H%M%S)"
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

echo "## Tuning /boot/firmware/config.txt"
if [[ -f "${CONFIG_TXT}" ]]; then
  backup_file "${CONFIG_TXT}"

  ## uncomment: dtparam=i2c_arm=on
  ensure_uncommented "${CONFIG_TXT}" "dtparam=i2c_arm=on"

  ## comment these if present
  ensure_commented "${CONFIG_TXT}" "dtparam=audio=on"
  ensure_commented "${CONFIG_TXT}" "camera_auto_detect=1"
  ensure_commented "${CONFIG_TXT}" "display_auto_detect=1"
  ensure_commented "${CONFIG_TXT}" "auto_initramfs=1"
  ensure_commented "${CONFIG_TXT}" "dtoverlay=vc4-kms-v3d"
  ensure_commented "${CONFIG_TXT}" "max_framebuffers=2"
  ensure_commented "${CONFIG_TXT}" "otg_mode=1"
  ensure_commented "${CONFIG_TXT}" "dtoverlay=dwc2,dr_mode=host"

  ## at the end add
  ## ---------------------------------------
  ## ipv6.disable=1
  ## disable_camera_led=1
  ## ---------------------------------------
  echo "" >> "${CONFIG_TXT}"
  ensure_line_present "${CONFIG_TXT}" "ipv6.disable=1"
  ensure_line_present "${CONFIG_TXT}" "disable_camera_led=1"
else
  echo "WARNING: ${CONFIG_TXT} not found; skipping."
fi

echo "## Writing sysctl config to disable IPv6 (and keep ip_forward=0)"
backup_file "${SYSCTL_FILE}"
cat > "${SYSCTL_FILE}" <<'EOF'
# Disable IP forwarding
net.ipv4.ip_forward = 0

# Disable IPv6
net.ipv6.conf.all.disable_ipv6 = 1
net.ipv6.conf.default.disable_ipv6 = 1
net.ipv6.conf.lo.disable_ipv6 = 1
EOF

echo "## Apply sysctl changes"
sysctl --system

echo "## Configure tmpfs mounts in /etc/fstab"
if [[ -f "${FSTAB}" ]]; then
  backup_file "${FSTAB}"
  grep -qE '^[[:space:]]*tmpfs[[:space:]]+/var/log[[:space:]]' "${FSTAB}" || echo 'tmpfs       /var/log          tmpfs     defaults      0      0' >> "${FSTAB}"
  grep -qE '^[[:space:]]*tmpfs[[:space:]]+/tmp[[:space:]]' "${FSTAB}"     || echo 'tmpfs       /tmp              tmpfs     defaults      0      0' >> "${FSTAB}"
  grep -qE '^[[:space:]]*tmpfs[[:space:]]+/var/tmp[[:space:]]' "${FSTAB}" || echo 'tmpfs       /var/tmp          tmpfs     defaults      0      0' >> "${FSTAB}"
else
  echo "WARNING: ${FSTAB} not found; skipping."
fi

echo "## Remove manual pages auto update marker (if present)"
rm -f /var/lib/man-db/auto-update || true

echo "## Disable daily software updates"
systemctl mask apt-daily-upgrade 2>/dev/null || true
systemctl mask apt-daily 2>/dev/null || true
systemctl disable apt-daily-upgrade.timer 2>/dev/null || true
systemctl disable apt-daily.timer 2>/dev/null || true

echo "## Clean"
apt autoremove --purge -y

echo "## Rebooting now"
reboot