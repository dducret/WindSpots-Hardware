#!/usr/bin/env bash
# 1-prep.sh
set -euo pipefail

## Must run as root
if [[ "${EUID}" -ne 0 ]]; then
  echo "ERROR: run as root (sudo -s)"; exit 1
fi

DEBIAN_HOME="/home/debian"

MESH_DIR="/opt/meshagent"
MESH_BIN="${MESH_DIR}/meshagent"
MESH_URL='https://mc.windspots.org:444/meshagents?id=c%40BuzaU7IfiIFtx6dIiDjgb479zsNloSnUaeXoLJQ3hgiqj5VS4IM1O9GzJbLjKF&installflags=2&meshinstall=25'

## Check if interfaces and wpa_supplicant.conf exist
if [[ ! -f "${DEBIAN_HOME}/interfaces" ]]; then
   echo "intefaces file not found!, please provide it"; exit 1
fi
if [[ ! -f "${DEBIAN_HOME}/wpa_supplicant.conf" ]]; then
   echo "wpa_supplicant.conf file not found!, please provide it"; exit 1
fi
if [[ ! -f "${DEBIAN_HOME}/dhcpd.conf" ]]; then
   echo "dhcpd.conf file not found!, please provide it"; exit 1
fi

echo "## Removing arm64 packages if any"
apt-get purge -y $(dpkg -l | grep ":arm64" | awk '{print $2}')

echo "## Creating ${MESH_DIR}"
mkdir -p "${MESH_DIR}"
cd "${MESH_DIR}"

echo "## Downloading meshagent"
wget -O "${MESH_BIN}" "${MESH_URL}"
chmod +x "${MESH_BIN}"

echo "## Installing meshagent to ${MESH_DIR}"
"${MESH_BIN}" -install --installPath="${MESH_DIR}"

echo "## Full upgrade"
apt update -y && apt upgrade -y

echo "## Install ifupdown"
apt install -y ifupdown

echo "## Copy network configs (if linux/ tree exists)"
install -m 0644 -D "${DEBIAN_HOME}/interfaces" /etc/network/interfaces
install -m 0600 -D "${DEBIAN_HOME}/wpa_supplicant.conf" /etc/wpa_supplicant/wpa_supplicant.conf
install -m 0600 -D "${DEBIAN_HOME}/dhcpd.conf" /etc/dhcp/dhcpd.conf

echo "## Disable NetworkManager services"
systemctl disable --now NetworkManager.service 2>/dev/null || true
systemctl mask NetworkManager.service 2>/dev/null || true
systemctl disable --now NetworkManager-wait-online.service 2>/dev/null || true
systemctl disable --now NetworkManager-dispatcher.service 2>/dev/null || true

echo "## Rebooting now"
reboot