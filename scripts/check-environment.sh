#!/bin/bash

# FlyKylin Linux环境检查脚本

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

ERROR_COUNT=0
WARNING_COUNT=0

echo -e "${CYAN}========================================"
echo -e " FlyKylin 开发环境检查 - Linux"
echo -e "========================================${NC}"
echo ""

check_command() {
    local name=$1
    local command=$2
    local min_version=$3
    local required=${4:-true}
    
    echo -n "检查 $name..."
    
    if command -v ${command%% *} &> /dev/null; then
        version=$($command 2>&1 | head -n 1)
        echo -e " ${GREEN}✓${NC}"
        if [ -n "$version" ]; then
            echo -e "  ${NC}版本: $version${NC}"
        fi
        return 0
    else
        if [ "$required" = true ]; then
            echo -e " ${RED}✗ 未安装${NC}"
            ((ERROR_COUNT++))
        else
            echo -e " ${YELLOW}⚠ 未安装 (可选)${NC}"
            ((WARNING_COUNT++))
        fi
        return 1
    fi
}

check_path() {
    local name=$1
    local path=$2
    local required=${3:-true}
    
    echo -n "检查 $name..."
    
    if [ -e "$path" ]; then
        echo -e " ${GREEN}✓${NC}"
        echo -e "  ${NC}路径: $path${NC}"
        return 0
    else
        if [ "$required" = true ]; then
            echo -e " ${RED}✗ 路径不存在${NC}"
            ((ERROR_COUNT++))
        else
            echo -e " ${YELLOW}⚠ 路径不存在 (可选)${NC}"
            ((WARNING_COUNT++))
        fi
        return 1
    fi
}

echo -e "${CYAN}1. 核心编译工具${NC}"
echo "----------------------------------------"
check_command "GCC" "gcc --version"
check_command "G++" "g++ --version"
check_command "CMake" "cmake --version"
check_command "Make" "make --version"
check_command "Git" "git --version"
echo ""

echo -e "${CYAN}2. Qt框架${NC}"
echo "----------------------------------------"
if ! check_command "qmake" "qmake --version" "" false; then
    check_command "qmake6" "qmake6 --version" "" false
fi

if [ -n "$Qt6_DIR" ]; then
    echo -e "  当前Qt6_DIR: $Qt6_DIR"
fi
echo ""

echo -e "${CYAN}3. 第三方库${NC}"
echo "----------------------------------------"
check_command "Protobuf编译器" "protoc --version" "" true
check_path "ONNX Runtime" "3rdparty/onnxruntime/include" false

# 检查RK3566 NPU工具（仅ARM64）
if [ "$(uname -m)" = "aarch64" ]; then
    check_path "RKNPU Toolkit" "3rdparty/rknn_api/include" false
fi
echo ""

echo -e "${CYAN}4. 开发库${NC}"
echo "----------------------------------------"
check_command "SQLite3" "sqlite3 --version" "" true
check_command "pkg-config" "pkg-config --version" "" true

# 检查Qt6库
if command -v pkg-config &> /dev/null; then
    echo -n "检查 Qt6 Core..."
    if pkg-config --exists Qt6Core; then
        echo -e " ${GREEN}✓${NC}"
    else
        echo -e " ${RED}✗${NC}"
        ((ERROR_COUNT++))
    fi
fi
echo ""

echo -e "${CYAN}5. 代码质量工具${NC}"
echo "----------------------------------------"
check_command "clang-format" "clang-format --version" "" false
check_command "clang-tidy" "clang-tidy --version" "" false
check_command "cppcheck" "cppcheck --version" "" false
check_command "valgrind" "valgrind --version" "" false
echo ""

echo -e "${CYAN}6. 性能分析工具${NC}"
echo "----------------------------------------"
check_command "perf" "perf --version" "" false
check_command "gprof" "gprof --version" "" false
echo ""

echo -e "${CYAN}7. Python (用于脚本和工具)${NC}"
echo "----------------------------------------"
if check_command "Python3" "python3 --version" "" false; then
    check_command "pip3" "pip3 --version" "" false
    
    # 检查RKNPU工具包（仅ARM64）
    if [ "$(uname -m)" = "aarch64" ]; then
        echo -n "检查 rknn-toolkit2..."
        if python3 -c "import rknnlite" 2>/dev/null; then
            echo -e " ${GREEN}✓${NC}"
        else
            echo -e " ${YELLOW}⚠ 未安装 (RK3566需要)${NC}"
            ((WARNING_COUNT++))
        fi
    fi
fi
echo ""

echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}检查完成${NC}"
echo -e "${CYAN}========================================${NC}"
echo ""

if [ $ERROR_COUNT -eq 0 ]; then
    echo -e "${GREEN}✓ 所有必需组件已安装!${NC}"
else
    echo -e "${RED}✗ 发现 $ERROR_COUNT 个必需组件未安装${NC}"
    echo -e "${YELLOW}请参考 docs/环境配置指南.md 进行安装${NC}"
fi

if [ $WARNING_COUNT -gt 0 ]; then
    echo -e "${YELLOW}⚠ $WARNING_COUNT 个可选组件未安装${NC}"
fi

echo ""
echo -e "${CYAN}详细配置指南: docs/环境配置指南.md${NC}"
echo ""

# 返回退出码
if [ $ERROR_COUNT -gt 0 ]; then
    exit 1
else
    exit 0
fi
