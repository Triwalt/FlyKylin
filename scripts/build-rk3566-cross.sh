#!/bin/bash
#
# FlyKylin RK3566 交叉编译脚本
# 在 WSL/Linux 上交叉编译 ARM64 版本
#

set -e

# 配置
PROJECT_ROOT="${PROJECT_ROOT:-$(cd "$(dirname "$0")/.." && pwd)}"
BUILD_DIR="${PROJECT_ROOT}/build/linux-arm64-rk3566-cross"
SYSROOT="${SYSROOT:-/opt/rk3566-sysroot}"

# 交叉编译工具链
CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}"
CC="${CROSS_COMPILE}gcc"
CXX="${CROSS_COMPILE}g++"

# 并行编译数
JOBS="${JOBS:-$(nproc)}"

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

# 显示帮助
show_help() {
    cat << EOF
FlyKylin RK3566 交叉编译脚本

用法: $0 [选项]

选项:
  -c, --configure   仅配置 CMake
  -b, --build       仅构建 (默认)
  -a, --all         配置 + 构建
  -C, --clean       清理构建目录
  -h, --help        显示帮助

环境变量:
  PROJECT_ROOT      项目根目录
  SYSROOT           目标系统 sysroot (默认: /opt/rk3566-sysroot)
  CROSS_COMPILE     交叉编译前缀 (默认: aarch64-linux-gnu-)
  JOBS              并行编译数 (默认: $(nproc))

示例:
  $0 --all
  SYSROOT=/path/to/sysroot $0 --configure
EOF
}

# 检查环境
check_environment() {
    log_step "检查交叉编译环境..."
    
    # 检查交叉编译器
    if ! command -v "${CC}" &> /dev/null; then
        log_error "交叉编译器未找到: ${CC}"
        log_error "请安装: sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu"
        exit 1
    fi
    
    # 检查 CMake
    if ! command -v cmake &> /dev/null; then
        log_error "CMake 未安装"
        exit 1
    fi
    
    # 检查 sysroot
    if [ ! -d "$SYSROOT" ]; then
        log_warn "Sysroot 不存在: $SYSROOT"
        log_warn "部分功能可能无法正常编译"
    else
        log_info "Sysroot: $SYSROOT"
    fi
    
    # 检查 Qt5 在 sysroot 中
    if [ -d "$SYSROOT" ]; then
        QT5_LIB="$SYSROOT/usr/lib/aarch64-linux-gnu/libQt5Core.so"
        if [ -f "$QT5_LIB" ]; then
            log_info "Qt5 库已找到: $QT5_LIB"
        else
            log_warn "Qt5 库未在 sysroot 中找到"
        fi
    fi
    
    log_info "交叉编译器: $(${CC} --version | head -1)"
    log_info "环境检查完成"
}

# 配置项目
do_configure() {
    log_step "配置 CMake 项目..."
    
    mkdir -p "$BUILD_DIR"
    
    cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" \
        -DCMAKE_SYSTEM_NAME=Linux \
        -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
        -DCMAKE_C_COMPILER="${CC}" \
        -DCMAKE_CXX_COMPILER="${CXX}" \
        -DCMAKE_SYSROOT="${SYSROOT}" \
        -DCMAKE_FIND_ROOT_PATH="${SYSROOT}" \
        -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
        -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
        -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
        -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY \
        -DFLYKYLIN_FORCE_QT5=ON \
        -DFLYKYLIN_FORCE_RK3566=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTS=OFF
    
    if [ $? -ne 0 ]; then
        log_error "CMake 配置失败"
        exit 1
    fi
    
    log_info "配置完成"
}

# 构建项目
do_build() {
    log_step "构建项目..."
    
    if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
        log_warn "项目未配置，先运行配置..."
        do_configure
    fi
    
    cmake --build "$BUILD_DIR" --target FlyKylin -j"$JOBS"
    
    if [ $? -ne 0 ]; then
        log_error "构建失败"
        exit 1
    fi
    
    # 显示构建结果
    BINARY="$BUILD_DIR/bin/FlyKylin"
    if [ -f "$BINARY" ]; then
        log_info "构建成功: $BINARY"
        log_info "文件大小: $(ls -lh "$BINARY" | awk '{print $5}')"
        log_info "架构: $(file "$BINARY" | grep -o 'ARM aarch64' || echo 'Unknown')"
    else
        log_error "可执行文件未生成"
        exit 1
    fi
}

# 清理构建
do_clean() {
    log_step "清理构建目录..."
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        log_info "已删除: $BUILD_DIR"
    else
        log_info "构建目录不存在"
    fi
}

# 主函数
main() {
    echo -e "${CYAN}========================================"
    echo -e " FlyKylin RK3566 交叉编译"
    echo -e "========================================${NC}"
    echo ""
    echo "项目目录: $PROJECT_ROOT"
    echo "构建目录: $BUILD_DIR"
    echo "Sysroot:  $SYSROOT"
    echo ""
    
    case "${1:-}" in
        -c|--configure)
            check_environment
            do_configure
            ;;
        -b|--build|"")
            check_environment
            do_build
            ;;
        -a|--all)
            check_environment
            do_configure
            do_build
            ;;
        -C|--clean)
            do_clean
            ;;
        -h|--help)
            show_help
            ;;
        *)
            log_error "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
    
    echo ""
    log_info "操作完成!"
}

main "$@"
