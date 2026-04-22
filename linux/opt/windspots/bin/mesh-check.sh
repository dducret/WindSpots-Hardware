#!/bin/sh
. "$(dirname "$0")/common.sh"

MESH_DEBUG=${MESH_DEBUG:-N}

[ "$MESH_DEBUG" = "Y" ] || [ "$MESH_DEBUG" = "y" ] || exit 0

log_cmd_output() {
  label="$1"
  shift

  if ! command -v "$1" >/dev/null 2>&1; then
    ws_syslog "[mesh-check] ${label}: command $1 not available"
    return 0
  fi

  output=$("$@" 2>&1)
  rc=$?

  if [ -n "$output" ]; then
    printf '%s\n' "$output" | while IFS= read -r line; do
      ws_syslog "[mesh-check] ${label}: ${line}"
    done
  else
    ws_syslog "[mesh-check] ${label}: no output"
  fi

  ws_syslog "[mesh-check] ${label}: exit=${rc}"
}

read_first_line() {
  file_path="$1"
  if [ -r "$file_path" ]; then
    IFS= read -r first_line < "$file_path"
    printf '%s' "$first_line"
  fi
}

log_mesh_file() {
  file_path="$1"
  if [ -f "$file_path" ]; then
    ws_syslog "[mesh-check] file=${file_path} size=$(wc -c < "$file_path" 2>/dev/null)"
    log_cmd_output "sha256 ${file_path}" sha256sum "$file_path"
    log_cmd_output "head ${file_path}" sed -n '1,20p' "$file_path"
  fi
}

ws_syslog "[mesh-check] start station=${STATION} hostname=$(hostname 2>/dev/null) machine_id=$(read_first_line /etc/machine-id) boot_id=$(read_first_line /proc/sys/kernel/random/boot_id)"

log_cmd_output "ip-link" ip -details link
log_cmd_output "ip-address" ip address
log_cmd_output "ip-route" ip route
log_cmd_output "resolv-conf" sed -n '1,40p' /etc/resolv.conf
log_cmd_output "network-manager" nmcli --terse --fields NAME,UUID,TYPE,DEVICE connection show
log_cmd_output "modem-manager" mmcli -L
log_cmd_output "meshagent-service" systemctl status meshagent --no-pager
log_cmd_output "meshagent-unit-files" systemctl list-unit-files meshagent.service
log_cmd_output "meshagent-process" ps -ef

for mesh_file in \
  /usr/local/mesh_services/meshagent/meshagent.msh \
  /usr/local/mesh_services/meshagent/meshagent.db \
  /opt/meshagent/meshagent.msh \
  /opt/meshagent/meshagent.db \
  /usr/local/meshagent/meshagent.msh \
  /usr/local/meshagent/meshagent.db
do
  log_mesh_file "$mesh_file"
done

ws_syslog "[mesh-check] end"
exit 0
