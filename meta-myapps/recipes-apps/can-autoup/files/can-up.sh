#!/bin/sh
set -e

IFACE=can0
BITRATE=200000

# Nếu kernel/DT chưa tạo can0 thì thôi
ip link show "$IFACE" >/dev/null 2>&1 || exit 0

# Đảm bảo reset trạng thái trước khi set bitrate
ip link set "$IFACE" down >/dev/null 2>&1 || true

# Bring up + cấu hình bitrate
ip link set "$IFACE" up type can bitrate "$BITRATE"
