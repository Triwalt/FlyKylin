#!/usr/bin/env bash
# Simple memory probe helper for FlyKylin on RK3566
# Usage:
#   1) ./flykylin-memprobe.sh           # attach to existing FlyKylin if running, otherwise start it
#   2) ./flykylin-memprobe.sh 15        # same as above, but wait 15 seconds after start before sampling
#   3) ./flykylin-memprobe.sh attach    # only attach to existing FlyKylin, do not start new
#
set -euo pipefail

APP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Default wait time after starting a new instance (seconds)
DEFAULT_WAIT_SECONDS=10

wait_seconds="${2:-$DEFAULT_WAIT_SECONDS}"
mode="${1:-auto}"

find_flykylin_pid() {
    # Try pidof first
    if command -v pidof >/dev/null 2>&1; then
        local p
        p="$(pidof -s FlyKylin 2>/dev/null || true)"
        if [[ -n "$p" ]]; then
            echo "$p"
            return 0
        fi
    fi

    # Fallback to ps
    if command -v ps >/dev/null 2>&1; then
        local p
        p="$(ps -C FlyKylin -o pid= 2>/dev/null | head -n 1 | tr -d ' ')"
        if [[ -n "$p" ]]; then
            echo "$p"
            return 0
        fi
    fi

    return 1
}

start_if_needed() {
    local pid
    pid="$(find_flykylin_pid || true)"
    if [[ -n "$pid" ]]; then
        echo "[memprobe] Found existing FlyKylin process: PID=$pid" >&2
        printf '%s\n' "$pid"
        return 0
    fi

    echo "[memprobe] No existing FlyKylin process found, starting via run-flykylin.sh ..." >&2
    if [[ ! -x "$APP_ROOT/run-flykylin.sh" ]]; then
        echo "[memprobe] ERROR: run-flykylin.sh not found or not executable under $APP_ROOT" >&2
        exit 1
    fi

    # Do not override QT_QPA_PLATFORM here; use whatever run-flykylin.sh sets.
    ( cd "$APP_ROOT" && ./run-flykylin.sh ) &
    pid=$!
    echo "[memprobe] Started FlyKylin with PID=$pid, waiting ${wait_seconds}s before sampling ..." >&2
    sleep "$wait_seconds"

    if ! kill -0 "$pid" 2>/dev/null; then
        echo "[memprobe] ERROR: FlyKylin (PID=$pid) exited before sampling" >&2
        exit 1
    fi

    printf '%s\n' "$pid"
}

attach_only() {
    local pid
    pid="$(find_flykylin_pid || true)"
    if [[ -z "$pid" ]]; then
        echo "[memprobe] ERROR: No running FlyKylin process found to attach" >&2
        exit 1
    fi
    echo "[memprobe] Attaching to existing FlyKylin process: PID=$pid" >&2
    printf '%s\n' "$pid"
}

case "$mode" in
    auto)
        target_pid="$(start_if_needed)"
        ;;
    attach)
        target_pid="$(attach_only)"
        ;;
    *)
        # If first argument is a number, treat as wait_seconds in auto mode
        if [[ "$mode" =~ ^[0-9]+$ ]]; then
            wait_seconds="$mode"
            target_pid="$(start_if_needed)"
        else
            echo "Usage: $0 [auto|attach] [wait_seconds]" >&2
            exit 1
        fi
        ;;
esac

if [[ -z "${target_pid:-}" ]]; then
    echo "[memprobe] ERROR: Could not determine FlyKylin PID" >&2
    exit 1
fi

status_file="/proc/${target_pid}/status"

if [[ ! -r "$status_file" ]]; then
    echo "[memprobe] ERROR: Cannot read $status_file (process may have exited)" >&2
    exit 1
fi

echo "[memprobe] ===== /proc/${target_pid}/status (VmSize / VmRSS) ====="
grep -E "^Vm(Size|RSS):" "$status_file" || true

echo
echo "[memprobe] ===== ps output (RSS in KB) ====="
if command -v ps >/dev/null 2>&1; then
    ps -p "$target_pid" -o pid,ppid,rss,vsz,comm
else
    echo "[memprobe] 'ps' not available on this system; only /proc data shown" >&2
fi

echo
echo "[memprobe] NOTE: VmRSS ~= physical memory used by FlyKylin (in KB). Divide by 1024 to get MB."

exit 0
