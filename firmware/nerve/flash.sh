#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE="$SCRIPT_DIR/../../zephyr-workspace"

UF2="$WORKSPACE/build/zephyr/zephyr.uf2"

SERIAL="/dev/serial/by-id/usb-Zephyr_Project_CDC_ACM_serial_backend_E9F902BF975161E3-if00"
UF2_DISK="/dev/disk/by-id/usb-Adafruit_nRF_UF2_E9F902BF975161E3-0:0"

if [[ ! -f "$UF2" ]]; then
    echo "No firmware found. Run ./build.sh first."
    exit 1
fi

if [[ ! -e "$SERIAL" ]]; then
    echo "Nerve not found."
    exit 1
fi

echo "Requesting bootloader..."
stty -F "$SERIAL" raw -echo
printf 'b' > "$SERIAL"

echo "Waiting for UF2 bootloader..."
for _ in {1..50}; do
    [[ -e "$UF2_DISK" ]] && break
    sleep 0.1
done

if [[ ! -e "$UF2_DISK" ]]; then
    echo "UF2 bootloader did not appear."
    exit 1
fi

# Don't let Linux's FAT driver interfere with the synthetic UF2 disk.
udisksctl unmount -b "$UF2_DISK" >/dev/null 2>&1 || true

echo "Flashing..."
sudo dd \
    if="$UF2" \
    of="$UF2_DISK" \
    bs=512 \
    conv=fsync \
    status=progress

echo "Flash complete."
