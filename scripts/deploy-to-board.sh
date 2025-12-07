#!/bin/bash
#
# FlyKylin 板端部署脚本
# 用于从 WSL 部署应用到 RK3566 板端
#

set -e

# 配置
BOARD_IP="${BOARD_IP:-192.168.100.2}"
BOARD_USER="${BOARD_USER:-kylin}"
BOARD_PASS="${BOARD_PASS:-123456}"
BOARD_APP_DIR="/home/${BOARD_USER}/FlyKylinApp"

# 项目路径 (WSL 中的路径)
PROJECT_ROOT="${PROJECT_ROOT:-/mnt/e/Project/FlyKylin}"
BUILD_DIR="${PROJECT_ROOT}/build/linux-arm64-rk3566-cross"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info() { echo -e "${GREEN}[INFO]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }
log_step() { echo -e "${CYAN}[STEP]${NC} $1"; }

# SSH/SCP 命令封装
ssh_cmd() {
    sshpass -p "$BOARD_PASS" ssh -o StrictHostKeyChecking=no "${BOARD_USER}@${BOARD_IP}" "$@"
}

scp_cmd() {
    sshpass -p "$BOARD_PASS" scp -o StrictHostKeyChecking=no "$@"
}

# 检查依赖
check_deps() {
    log_step "检查依赖..."
    
    if ! command -v sshpass &> /dev/null; then
        log_error "sshpass 未安装，请运行: sudo apt install sshpass"
        exit 1
    fi
    
    if [ ! -f "${BUILD_DIR}/bin/FlyKylin" ]; then
        log_error "可执行文件不存在: ${BUILD_DIR}/bin/FlyKylin"
        log_error "请先运行交叉编译"
        exit 1
    fi
    
    log_info "依赖检查通过"
}

# 检查板端连接
check_connection() {
    log_step "检查板端连接..."
    
    if ! ping -c 1 -W 2 "$BOARD_IP" &> /dev/null; then
        log_error "无法连接到板端: $BOARD_IP"
        exit 1
    fi
    
    if ! ssh_cmd "echo ok" &> /dev/null; then
        log_error "SSH 连接失败"
        exit 1
    fi
    
    log_info "板端连接正常: $BOARD_IP"
}

# 停止旧进程
stop_app() {
    log_step "停止旧进程..."
    
    ssh_cmd "pkill -x FlyKylin 2>/dev/null || true"
    sleep 1
    
    log_info "旧进程已停止"
}

# 部署可执行文件
deploy_binary() {
    log_step "部署可执行文件..."
    
    scp_cmd "${BUILD_DIR}/bin/FlyKylin" "${BOARD_USER}@${BOARD_IP}:${BOARD_APP_DIR}/bin/"
    ssh_cmd "chmod +x ${BOARD_APP_DIR}/bin/FlyKylin"
    
    log_info "可执行文件部署完成"
}

# 部署 RKNN 模型
deploy_model() {
    log_step "部署 RKNN 模型..."
    
    local model_src="${PROJECT_ROOT}/model/rknn/open_nsfw_rk3566.rknn"
    local model_dst="${BOARD_APP_DIR}/bin/models/open_nsfw.rknn"
    
    if [ -f "$model_src" ]; then
        ssh_cmd "mkdir -p ${BOARD_APP_DIR}/bin/models"
        scp_cmd "$model_src" "${BOARD_USER}@${BOARD_IP}:${model_dst}"
        log_info "RKNN 模型部署完成"
    else
        log_warn "RKNN 模型不存在: $model_src"
    fi
}

# 部署 QML 文件
deploy_qml() {
    log_step "部署 QML 文件..."
    
    local qml_src="${PROJECT_ROOT}/src/ui/qml"
    local qml_dst="${BOARD_APP_DIR}/share/qml"
    
    if [ -d "$qml_src" ]; then
        ssh_cmd "mkdir -p ${BOARD_APP_DIR}/share"
        scp_cmd -r "$qml_src" "${BOARD_USER}@${BOARD_IP}:${BOARD_APP_DIR}/share/"
        log_info "QML 文件部署完成"
    else
        log_warn "QML 目录不存在: $qml_src"
    fi
}

# 部署启动脚本
deploy_scripts() {
    log_step "部署启动脚本..."
    
    scp_cmd "${PROJECT_ROOT}/scripts/run-flykylin.sh" "${BOARD_USER}@${BOARD_IP}:${BOARD_APP_DIR}/"
    scp_cmd "${PROJECT_ROOT}/scripts/run-flykylin-optimized.sh" "${BOARD_USER}@${BOARD_IP}:${BOARD_APP_DIR}/"
    
    ssh_cmd "chmod +x ${BOARD_APP_DIR}/run-flykylin.sh ${BOARD_APP_DIR}/run-flykylin-optimized.sh"
    
    log_info "启动脚本部署完成"
}

# 启动应用
start_app() {
    log_step "启动应用..."
    
    ssh_cmd "cd ${BOARD_APP_DIR} && nohup ./run-flykylin-optimized.sh > /tmp/flykylin.log 2>&1 &"
    
    sleep 2
    
    if ssh_cmd "pgrep -x FlyKylin" &> /dev/null; then
        log_info "应用启动成功"
    else
        log_error "应用启动失败，查看日志:"
        ssh_cmd "tail -20 /tmp/flykylin.log"
        exit 1
    fi
}

# 显示帮助
show_help() {
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -a, --all       部署所有内容并启动"
    echo "  -b, --binary    仅部署可执行文件"
    echo "  -m, --model     仅部署 RKNN 模型"
    echo "  -q, --qml       仅部署 QML 文件"
    echo "  -s, --scripts   仅部署启动脚本"
    echo "  -r, --restart   重启应用"
    echo "  -l, --log       查看日志"
    echo "  -h, --help      显示帮助"
    echo ""
    echo "环境变量:"
    echo "  BOARD_IP        板端 IP (默认: 192.168.100.2)"
    echo "  BOARD_USER      板端用户 (默认: kylin)"
    echo "  BOARD_PASS      板端密码 (默认: 123456)"
    echo "  PROJECT_ROOT    项目根目录 (默认: /mnt/e/Project/FlyKylin)"
}

# 主函数
main() {
    case "${1:-}" in
        -a|--all)
            check_deps
            check_connection
            stop_app
            deploy_binary
            deploy_model
            deploy_qml
            deploy_scripts
            start_app
            ;;
        -b|--binary)
            check_deps
            check_connection
            stop_app
            deploy_binary
            start_app
            ;;
        -m|--model)
            check_connection
            deploy_model
            ;;
        -q|--qml)
            check_connection
            deploy_qml
            ;;
        -s|--scripts)
            check_connection
            deploy_scripts
            ;;
        -r|--restart)
            check_connection
            stop_app
            start_app
            ;;
        -l|--log)
            check_connection
            ssh_cmd "tail -50 /tmp/flykylin.log"
            ;;
        -h|--help|"")
            show_help
            ;;
        *)
            log_error "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
}

main "$@"
