#!/bin/bash
#
# FlyKylin 优化启动脚本
# 针对RK3566低内存环境优化
#

APP_ROOT="$HOME/FlyKylinApp"
LOG_FILE="/tmp/flykylin.log"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() { echo -e "${GREEN}[INFO]${NC} $1" | tee -a "$LOG_FILE"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1" | tee -a "$LOG_FILE"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1" | tee -a "$LOG_FILE"; }

# === 1. 清理旧日志 ===
[ -f "$LOG_FILE" ] && > "$LOG_FILE"

echo "=== FlyKylin Optimized Startup ===" | tee -a "$LOG_FILE"
echo "Time: $(date)" | tee -a "$LOG_FILE"

# === 2. 内存优化：关闭不必要的后台服务 ===
log_info "Optimizing memory usage..."

# 关闭蓝牙相关（节省约120MB）
pkill -9 blueman-applet 2>/dev/null
pkill -9 blueman-tray 2>/dev/null
pkill -9 bluetooth-sendto 2>/dev/null

# 关闭网络托盘（节省约50MB，nm-applet保留用于网络管理）
pkill -9 nm-tray 2>/dev/null

# 关闭侧边栏（节省约40MB）
pkill -9 ukui-sidebar 2>/dev/null

# 清理内存缓存
sync
echo 1 | sudo tee /proc/sys/vm/drop_caches >/dev/null 2>&1 || true

# 显示优化后的内存状态
MEM_AVAIL=$(free -m | awk '/^Mem:/{print $7}')
log_info "Available memory after optimization: ${MEM_AVAIL}MB"

# === 3. 检查并唤醒NPU ===
log_info "Checking NPU status..."

NPU_STATUS=$(cat /sys/devices/platform/fde40000.npu/power/runtime_status 2>/dev/null)
if [ "$NPU_STATUS" = "suspended" ]; then
    log_warn "NPU is suspended, waking up..."
    echo on | sudo tee /sys/devices/platform/fde40000.npu/power/control >/dev/null 2>&1
    sleep 1
    NPU_STATUS=$(cat /sys/devices/platform/fde40000.npu/power/runtime_status 2>/dev/null)
fi

if [ "$NPU_STATUS" = "active" ]; then
    log_info "NPU status: active ✓"
else
    log_warn "NPU status: $NPU_STATUS (may affect RKNN performance)"
fi

# === 4. 检查RKNN库 ===
if [ -f "/lib/librknnrt.so" ]; then
    RKNN_VER=$(strings /lib/librknnrt.so 2>/dev/null | grep "librknnrt version" | head -1)
    log_info "RKNN library: $RKNN_VER"
else
    log_warn "RKNN library not found at /lib/librknnrt.so"
fi

# === 5. 检查RKNN模型 ===
RKNN_MODEL="${APP_ROOT}/bin/models/open_nsfw.rknn"
if [ -f "$RKNN_MODEL" ]; then
    MODEL_SIZE=$(ls -lh "$RKNN_MODEL" | awk '{print $5}')
    log_info "RKNN model: $RKNN_MODEL ($MODEL_SIZE)"
else
    log_warn "RKNN model not found: $RKNN_MODEL"
fi

# === 6. 设置环境变量 ===
export DISPLAY=:0
export LD_LIBRARY_PATH="${APP_ROOT}/lib:${APP_ROOT}/3rdparty/onnxruntime-linux-aarch64-1.23.2/lib:$LD_LIBRARY_PATH"
export QT_PLUGIN_PATH="/usr/lib/aarch64-linux-gnu/qt5/plugins"
export QT_QPA_PLATFORM=xcb
export QT_AUTO_SCREEN_SCALE_FACTOR=0

# Qt性能优化
export QT_QUICK_BACKEND=software  # 使用软件渲染，减少GPU内存占用
export QSG_RENDER_LOOP=basic      # 使用基本渲染循环，减少内存
export MALLOC_TRIM_THRESHOLD_=65536  # 更积极地释放内存

# === 7. 杀死旧实例 ===
# 只匹配二进制文件，排除脚本本身
OLD_PID=$(pgrep -x "FlyKylin" 2>/dev/null)
if [ -n "$OLD_PID" ]; then
    log_warn "Killing old FlyKylin instance (PID: $OLD_PID)"
    kill -9 $OLD_PID 2>/dev/null
    sleep 1
fi

# === 8. 启动应用 ===
log_info "Starting FlyKylin..."
log_info "APP_ROOT=${APP_ROOT}"
log_info "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"

cd "${APP_ROOT}/bin"

# 使用nice降低优先级，避免卡死整个系统
exec nice -n 5 ./FlyKylin "$@" >> "$LOG_FILE" 2>&1
