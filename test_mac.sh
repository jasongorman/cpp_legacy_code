#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exe="$script_dir/build/cpp_legacy_code"

if [ ! -f "$exe" ]; then
    echo "Executable not found at $exe — build the project first (cmake --build --preset windows)." >&2
    exit 1
fi

for order_id in 1001 1002 1003; do
    "$exe" "$order_id"
done
