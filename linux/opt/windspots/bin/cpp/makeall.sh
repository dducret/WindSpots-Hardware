#!/usr/bin/env bash
set -uo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  cat <<'USAGE'
Usage: makeall.sh [make args...]

Run `make` in every C++ subfolder that contains a Makefile under this script directory.
Any arguments are passed through to each make command.
Examples:
  ./makeall.sh
  ./makeall.sh clean
  ./makeall.sh -j4
USAGE
  exit 0
fi

mapfile -t MAKE_DIRS < <(find "$ROOT_DIR" -type f -name 'Makefile' -printf '%h\n' | sort)

if [[ "${#MAKE_DIRS[@]}" -eq 0 ]]; then
  echo "No Makefile found under $ROOT_DIR"
  exit 0
fi

failures=0

for dir in "${MAKE_DIRS[@]}"; do
  echo "==== make in $dir ===="
  if ! make -C "$dir" "$@"; then
    ((failures+=1))
    echo "FAILED: $dir"
  fi
done

if [[ "$failures" -gt 0 ]]; then
  echo "Completed with $failures failure(s)."
  exit 1
fi

echo "All make commands completed successfully."

echo "copy w3rpi and initwsdb to /opt/windspots/bin"
killall w3rpi
chown -R windspots:windspots /opt/windspots/bin/cpp
chmod o+x /opt/windspots/bin/cpp/WS200/getalim /opt/windspots/bin/cpp/WS200/getanemo /opt/windspots/bin/cpp/WS200/getbaro /opt/windspots/bin/cpp/WS200/gettemp
chmod +x /opt/windspots/bin/cpp/WS200/getalim/getalim /opt/windspots/bin/cpp/WS200/getanemo/getanemo /opt/windspots/bin/cpp/WS200/getbaro/getbaro /opt/windspots/bin/cpp/WS200/gettemp/gettemp
cp /opt/windspots/bin/cpp/w3rpi/w3rpi /opt/windspots/bin/.
cp /opt/windspots/bin/cpp/initwsdb/initwsdb /opt/windspots/bin/.
chmod 755 /opt/windspots/bin/w3rpi /opt/windspots/bin/initwsdb
