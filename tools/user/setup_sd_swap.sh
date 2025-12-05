#!/bin/bash
set -e

# One-shot setup script to create a swapfile on the SD card
# (/media/kylin/EED4-7516/swapfile), configure a systemd service to
# enable it at boot, and immediately enable the swap.
#
# Usage (on the RK3566 board):
#   cd ~/FlyKylinApp
#   sudo ./setup_sd_swap.sh

if [ "$(id -u)" -ne 0 ]; then
  echo "[setup-sd-swap] Please run this script as root (use: sudo ./setup_sd_swap.sh)" >&2
  exit 1
fi

SD_MOUNT="/media/kylin/EED4-7516"
SWAPFILE="$SD_MOUNT/swapfile"
SIZE_GB=2

echo "[setup-sd-swap] SD_MOUNT=$SD_MOUNT"
echo "[setup-sd-swap] SWAPFILE=$SWAPFILE (size ${SIZE_GB}G)"

# Ensure SD mount exists
if [ ! -d "$SD_MOUNT" ]; then
  echo "[setup-sd-swap] ERROR: mountpoint $SD_MOUNT does not exist. Is the SD card mounted?" >&2
  exit 1
fi

# Create swapfile if needed
if [ ! -f "$SWAPFILE" ]; then
  echo "[setup-sd-swap] Creating ${SIZE_GB}G swapfile at $SWAPFILE ..."
  if ! fallocate -l "${SIZE_GB}G" "$SWAPFILE" 2>/dev/null; then
    echo "[setup-sd-swap] fallocate failed, falling back to dd ..."
    dd if=/dev/zero of="$SWAPFILE" bs=1M count=$((SIZE_GB*1024))
  fi
else
  echo "[setup-sd-swap] Swapfile already exists at $SWAPFILE"
fi

# Ensure secure ownership and permissions, and refresh swap signature
chmod 600 "$SWAPFILE"
chown root:root "$SWAPFILE"
mkswap "$SWAPFILE"

# Write helper script used by systemd service to (re)enable swap at boot
mkdir -p /usr/local/sbin
cat > /usr/local/sbin/enable-sd-swap.sh << 'EOF'
#!/bin/bash
set -e

SD_MOUNT="/media/kylin/EED4-7516"
SWAPFILE="$SD_MOUNT/swapfile"
SIZE_GB=2

echo "[sd-swap] using $SWAPFILE"

# wait up to 120 seconds for SD card mount
for i in $(seq 1 120); do
  if [ -d "$SD_MOUNT" ]; then
    echo "[sd-swap] mountpoint $SD_MOUNT is ready"
    break
  fi
  sleep 1
done

if [ ! -d "$SD_MOUNT" ]; then
  echo "[sd-swap] mountpoint $SD_MOUNT not found, skip swap"
  exit 0
fi

if [ ! -f "$SWAPFILE" ]; then
  echo "[sd-swap] creating ${SIZE_GB}G swapfile at $SWAPFILE"
  if ! fallocate -l "${SIZE_GB}G" "$SWAPFILE" 2>/dev/null; then
    echo "[sd-swap] fallocate failed, falling back to dd ..."
    dd if=/dev/zero of="$SWAPFILE" bs=1M count=$((SIZE_GB*1024))
  fi
fi

chmod 600 "$SWAPFILE"
chown root:root "$SWAPFILE"
mkswap "$SWAPFILE"

echo "[sd-swap] enabling swap on $SWAPFILE"
swapon "$SWAPFILE" || true

exit 0
EOF

chmod +x /usr/local/sbin/enable-sd-swap.sh

echo "[setup-sd-swap] Installed /usr/local/sbin/enable-sd-swap.sh"

# Create systemd service unit
cat > /etc/systemd/system/sd-swap.service << 'EOF'
[Unit]
Description=Enable swapfile on SD card /media/kylin/EED4-7516
After=multi-user.target

[Service]
Type=oneshot
ExecStart=/usr/local/sbin/enable-sd-swap.sh

[Install]
WantedBy=multi-user.target
EOF

echo "[setup-sd-swap] Installed /etc/systemd/system/sd-swap.service"

# Enable service and run it now
systemctl daemon-reload
systemctl enable --now sd-swap.service

echo "[setup-sd-swap] sd-swap.service enabled and started"

echo "[setup-sd-swap] Current swap devices:"
swapon --show || true

echo "[setup-sd-swap] Current memory usage:"
free -h || true

echo "[setup-sd-swap] Done. Swap on SD card should now be active and auto-enabled on boot."
