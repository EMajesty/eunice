#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE="$SCRIPT_DIR/../../zephyr-workspace"

cd "$WORKSPACE"

west build -p always \
-b promicro_nrf52840/nrf52840/uf2 \
"$SCRIPT_DIR" \
-- \
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
-DDTC_OVERLAY_FILE="$SCRIPT_DIR/promicro_nrf52840.overlay"
