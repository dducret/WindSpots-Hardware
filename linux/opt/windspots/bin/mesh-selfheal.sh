#!/bin/sh
. "$(dirname "$0")/common.sh"

MESH_SELFHEAL=${MESH_SELFHEAL:-N}
MESH_URL=${MESH_URL:-}
MESH_TARGET_DIR=/opt/meshagent
MESH_WAIT_RETRIES=${MESH_WAIT_RETRIES:-12}
MESH_WAIT_SECONDS=${MESH_WAIT_SECONDS:-10}

[ "$MESH_SELFHEAL" = "Y" ] || [ "$MESH_SELFHEAL" = "y" ] || exit 0

mesh_log() {
  ws_syslog "[mesh-selfheal] $*"
}

mesh_log_cmd() {
  label="$1"
  shift

  output=$("$@" 2>&1)
  rc=$?

  if [ -n "$output" ]; then
    printf '%s\n' "$output" | while IFS= read -r line; do
      mesh_log "${label}: ${line}"
    done
  else
    mesh_log "${label}: no output"
  fi

  mesh_log "${label}: exit=${rc}"
  return "$rc"
}

mesh_detect_dir() {
  service_exec=$(systemctl show -p ExecStart --value meshagent 2>/dev/null | sed -n 's#.*=\([^ ;"]*/meshagent\).*#\1#p' | head -n 1)
  if [ -n "$service_exec" ]; then
    dirname "$service_exec"
    return
  fi

  process_exec=$(ps -ef 2>/dev/null | awk '/[m]eshagent/ {print $8; exit}')
  if [ -n "$process_exec" ]; then
    dirname "$process_exec"
    return
  fi

  for dir_path in \
    /opt/meshagent \
    /usr/local/mesh_services/meshagent \
    /usr/local/meshagent
  do
    if [ -d "$dir_path" ] || [ -f "$dir_path/meshagent.msh" ] || [ -f "$dir_path/meshagent.db" ]; then
      echo "$dir_path"
      return
    fi
  done

  echo "$MESH_TARGET_DIR"
}

mesh_has_default_route() {
  ip route 2>/dev/null | grep -q '^default'
}

mesh_can_resolve_host() {
  host=$(printf '%s\n' "$MESH_URL" | sed -n 's#^[a-zA-Z]*://\([^/:]*\).*#\1#p')
  [ -n "$host" ] || return 1
  getent hosts "$host" >/dev/null 2>&1
}

mesh_wait_for_network() {
  attempt=1

  while [ "$attempt" -le "$MESH_WAIT_RETRIES" ]; do
    if mesh_has_default_route && mesh_can_resolve_host; then
      mesh_log "network ready attempt=${attempt}"
      return 0
    fi
    sleep "$MESH_WAIT_SECONDS"
    attempt=$((attempt + 1))
  done

  mesh_log "network unavailable after wait"
  return 1
}

mesh_is_healthy() {
  [ -f "$MESH_TARGET_DIR/meshagent.msh" ] || return 1
  [ -f "$MESH_TARGET_DIR/meshagent.db" ] || return 1
  systemctl is-active meshagent >/dev/null 2>&1 || return 1
  return 0
}

mesh_log_state() {
  mesh_log "state dir=${MESH_TARGET_DIR} msh=$([ -f "$MESH_TARGET_DIR/meshagent.msh" ] && echo 1 || echo 0) db=$([ -f "$MESH_TARGET_DIR/meshagent.db" ] && echo 1 || echo 0) active=$(systemctl is-active meshagent 2>/dev/null || echo unknown)"
}

mesh_wait_for_health() {
  attempt=1

  while [ "$attempt" -le 12 ]; do
    if mesh_is_healthy; then
      mesh_log "healthy after attempt=${attempt}"
      return 0
    fi
    mesh_log_state
    sleep 5
    attempt=$((attempt + 1))
  done

  mesh_log "health wait timeout"
  return 1
}

mesh_fetch_binary() {
  [ -n "$MESH_URL" ] || return 1
  mkdir -p "$MESH_TARGET_DIR" || return 1
  tmp_bin="$MESH_TARGET_DIR/meshagent.new"

  if ! mesh_log_cmd "wget" wget -O "$tmp_bin" "$MESH_URL"; then
    rm -f "$tmp_bin"
    return 1
  fi

  chmod +x "$tmp_bin" || return 1
  mv -f "$tmp_bin" "$MESH_TARGET_DIR/meshagent" || return 1
  return 0
}

mesh_prepare_msh() {
  source_dir="$1"
  msh_backup="${TMP}/meshagent.msh.backup"
  msh_source=""
  search_path=""

  for search_path in \
    "$source_dir/meshagent.msh" \
    "$MESH_TARGET_DIR/meshagent.msh" \
    "/usr/local/mesh_services/meshagent/meshagent.msh" \
    "/usr/local/meshagent/meshagent.msh" \
    "${TMP}/meshagent.msh.backup"
  do
    if [ -f "$search_path" ]; then
      msh_source="$search_path"
      break
    fi
  done

  if [ -z "$msh_source" ]; then
    for backup_dir in \
      /opt/meshagent.bak-* \
      /usr/local/mesh_services/meshagent.bak-* \
      /usr/local/meshagent.bak-*
    do
      if [ -f "$backup_dir/meshagent.msh" ]; then
        msh_source="$backup_dir/meshagent.msh"
        break
      fi
    done
  fi

  if [ -z "$msh_source" ]; then
    mesh_log "meshagent.msh missing in source, target and backups"
    return 1
  fi

  cp -f "$msh_source" "$msh_backup" || return 1

  mkdir -p "$MESH_TARGET_DIR" || return 1
  cp -f "$msh_backup" "$MESH_TARGET_DIR/meshagent.msh" || return 1
  mesh_log "msh prepared source=${msh_source} target=${MESH_TARGET_DIR}/meshagent.msh"
  return 0
}

mesh_prepare_msh_or_force_new() {
  source_dir="$1"

  if mesh_prepare_msh "$source_dir"; then
    return 0
  fi

  mkdir -p "$MESH_TARGET_DIR" || return 1
  rm -f "$MESH_TARGET_DIR/meshagent.msh" 2>/dev/null || true
  mesh_log "forcing reinstall without msh; a new Mesh identity may be created"
  return 0
}

mesh_uninstall_current() {
  current_dir="$1"

  mesh_log "uninstall current dir=${current_dir}"
  mesh_log_cmd "systemctl stop" systemctl stop meshagent || true

  if [ -x "$current_dir/meshagent" ]; then
    mesh_log_cmd "meshagent uninstall" "$current_dir/meshagent" -uninstall || true
  fi

  mesh_log_cmd "systemctl disable" systemctl disable meshagent || true
  rm -f /etc/systemd/system/meshagent.service 2>/dev/null || true
  rm -f /lib/systemd/system/meshagent.service 2>/dev/null || true
  systemctl daemon-reload >/dev/null 2>&1 || true
}

mesh_cleanup_old_dirs() {
  current_dir="$1"
  for dir_path in \
    /usr/local/mesh_services/meshagent \
    /usr/local/meshagent
  do
    if [ "$dir_path" != "$MESH_TARGET_DIR" ] && [ -d "$dir_path" ]; then
      rm -rf "$dir_path" 2>/dev/null || true
      mesh_log "removed old dir=${dir_path}"
    fi
  done

  if [ -n "$current_dir" ] && [ "$current_dir" != "$MESH_TARGET_DIR" ] && [ -d "$current_dir" ]; then
    rm -rf "$current_dir" 2>/dev/null || true
    mesh_log "removed current dir=${current_dir}"
  fi
}

mesh_install_target() {
  mesh_fetch_binary || return 1
  mesh_log_cmd "meshagent install" "$MESH_TARGET_DIR/meshagent" -install --installPath="$MESH_TARGET_DIR" || return 1
  mesh_log_cmd "systemctl restart" systemctl restart meshagent || true
  mesh_wait_for_health
}

CURRENT_DIR=$(mesh_detect_dir)
mesh_log "start current_dir=${CURRENT_DIR} target_dir=${MESH_TARGET_DIR}"

if mesh_is_healthy; then
  mesh_log "healthy db_present=1"
  exit 0
fi

mesh_log_state
mesh_log "repair needed"

if ! mesh_wait_for_network; then
  exit 1
fi

mesh_prepare_msh_or_force_new "$CURRENT_DIR" || {
  mesh_log "repair failed preparing msh or force-new path"
  exit 1
}

mesh_uninstall_current "$CURRENT_DIR"
mesh_cleanup_old_dirs "$CURRENT_DIR"

if mesh_install_target; then
  mesh_log "repair success"
  exit 0
fi

mesh_log "repair failed"
exit 1
