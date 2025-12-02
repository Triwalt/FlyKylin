#!/usr/bin/env bash
# FlyKylin watchdog & resource monitor for RK3566
#
# Usage (on board, in ~/FlyKylinApp):
#   ./flykylin-watchdog.sh start [interval_seconds]
#       # start FlyKylin if not running, then sample memory/CPU periodically
#   ./flykylin-watchdog.sh attach [interval_seconds]
#       # attach to an existing FlyKylin process and monitor it
#
set -euo pipefail

APP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIR="${APP_ROOT}/logs"
mkdir -p "$LOG_DIR"

MODE="${1:-start}"
INTERVAL="${2:-10}"   # default: 10 seconds between samples

find_flykylin_pid() {
    if command -v pidof >/dev/null 2>&1; then
        local p
        p="$(pidof -s FlyKylin 2>/dev/null || true)"
        if [[ -n "$p" ]]; then
            echo "$p"
            return 0
        fi
    fi

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

log_sample() {
    local pid="$1"
    local now
    now="$(date '+%F %T')"
    {
        echo "[$now] SAMPLE pid=$pid"
        if [[ -r "/proc/${pid}/status" ]]; then
            grep -E "^Vm(Size|RSS):" "/proc/${pid}/status" || true
        else
            echo "  /proc/${pid}/status not readable (process may have exited)"
        fi
        if command -v ps >/dev/null 2>&1; then
            ps -p "$pid" -o pid,ppid,rss,vsz,comm || true
        fi
        if command -v free >/dev/null 2>&1; then
            free -m || true
        fi
        echo
    } >> "$LOG_FILE"
}

monitor_loop() {
    local pid="$1"
    echo "[watchdog] Monitoring FlyKylin PID=$pid, logging to $LOG_FILE" >&2
    while kill -0 "$pid" 2>/dev/null; do
        log_sample "$pid"
        sleep "$INTERVAL"
    done

    local ts
    ts="$(date '+%F %T')"
    {
        echo "[$ts] NOTICE: FlyKylin PID=$pid is no longer running."
        if command -v dmesg >/dev/null 2>&1; then
            echo "===== dmesg tail ====="
            dmesg | tail -n 40 || true
            echo "===== end dmesg tail ====="
        fi
    } >> "$LOG_FILE"
}

case "$MODE" in
    start)
        LOG_FILE="${LOG_DIR}/watchdog-$(date '+%Y%m%d-%H%M%S').log"
        echo "[watchdog] Log file: $LOG_FILE" >&2
        pid="$(find_flykylin_pid || true)"
        if [[ -z "$pid" ]]; then
            if [[ ! -x "${APP_ROOT}/run-flykylin.sh" ]]; then
                echo "[watchdog] ERROR: run-flykylin.sh not found under $APP_ROOT" >&2
                exit 1
            fi
            echo "[watchdog] No existing FlyKylin, starting via run-flykylin.sh ..." >&2
            ( cd "$APP_ROOT" && ./run-flykylin.sh ) &
            pid=$!
            echo "[watchdog] Started FlyKylin PID=$pid, will monitor every ${INTERVAL}s" >&2
            sleep 5
        else
            echo "[watchdog] Attaching to existing FlyKylin PID=$pid" >&2
        fi
        monitor_loop "$pid"
        ;;
    attach)
        LOG_FILE="${LOG_DIR}/watchdog-attach-$(date '+%Y%m%d-%H%M%S').log"
        echo "[watchdog] Log file: $LOG_FILE" >&2
        pid="$(find_flykylin_pid || true)"
        if [[ -z "$pid" ]]; then
            echo "[watchdog] ERROR: No running FlyKylin process found to attach" >&2
            exit 1
        fi
        monitor_loop "$pid"
        ;;
    *)
        echo "Usage: $0 [start|attach] [interval_seconds]" >&2
        exit 1
        ;;
esac
