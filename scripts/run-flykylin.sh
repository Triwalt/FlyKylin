#!/bin/bash

APP_ROOT="$HOME/FlyKylinApp"
LOG_FILE="/tmp/flykylin.log"

# 设置X11显示 (必须在桌面环境下运行)
export DISPLAY=:0

# 应用自带库路径 (优先级最高)
export LD_LIBRARY_PATH="${APP_ROOT}/lib:${APP_ROOT}/3rdparty/onnxruntime-linux-aarch64-1.23.2/lib:$LD_LIBRARY_PATH"

# Qt 插件路径
export QT_PLUGIN_PATH="/usr/lib/aarch64-linux-gnu/qt5/plugins"

# Qt平台插件 (使用X11而非linuxfb)
export QT_QPA_PLATFORM=xcb

# 禁用Qt的高DPI缩放
export QT_AUTO_SCREEN_SCALE_FACTOR=0

echo "=== FlyKylin Startup ===" | tee -a "$LOG_FILE"
echo "Time: $(date)" | tee -a "$LOG_FILE"
echo "APP_ROOT=${APP_ROOT}" | tee -a "$LOG_FILE"
echo "DISPLAY=${DISPLAY}" | tee -a "$LOG_FILE"
echo "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}" | tee -a "$LOG_FILE"

cd "${APP_ROOT}/bin"

# 运行应用，输出到日志文件
exec ./FlyKylin "$@" >> "$LOG_FILE" 2>&1
