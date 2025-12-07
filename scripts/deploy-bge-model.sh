#!/bin/bash
# 部署BGE文本嵌入模型到RK3566板端
# 用于语义搜索功能

set -e

BOARD_IP="${BOARD_IP:-192.168.100.2}"
BOARD_USER="${BOARD_USER:-kylin}"
BOARD_PASS="${BOARD_PASS:-123456}"
BOARD_APP_DIR="/home/kylin/FlyKylinApp"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "=== Deploying BGE Model to RK3566 Board ==="
echo "Board: ${BOARD_USER}@${BOARD_IP}"
echo "Target: ${BOARD_APP_DIR}/bin/models/"

# Check if model exists
RKNN_MODEL="${PROJECT_ROOT}/model/rknn/bge-small-zh-v1.5.rknn"
VOCAB_FILE="${PROJECT_ROOT}/model/onnx/vocab.txt"

if [ ! -f "$RKNN_MODEL" ]; then
    echo "Error: RKNN model not found: $RKNN_MODEL"
    echo "Please run: python3 tools/convert_bge_to_rk3566.py"
    exit 1
fi

if [ ! -f "$VOCAB_FILE" ]; then
    echo "Error: Vocab file not found: $VOCAB_FILE"
    exit 1
fi

echo ""
echo "Files to deploy:"
echo "  - $RKNN_MODEL ($(du -h "$RKNN_MODEL" | cut -f1))"
echo "  - $VOCAB_FILE"

# Create models directory on board
echo ""
echo "Creating models directory..."
sshpass -p "$BOARD_PASS" ssh "${BOARD_USER}@${BOARD_IP}" \
    "mkdir -p ${BOARD_APP_DIR}/bin/models"

# Deploy RKNN model
echo "Deploying RKNN model..."
sshpass -p "$BOARD_PASS" scp "$RKNN_MODEL" \
    "${BOARD_USER}@${BOARD_IP}:${BOARD_APP_DIR}/bin/models/"

# Deploy vocab file
echo "Deploying vocab file..."
sshpass -p "$BOARD_PASS" scp "$VOCAB_FILE" \
    "${BOARD_USER}@${BOARD_IP}:${BOARD_APP_DIR}/bin/models/"

# Verify deployment
echo ""
echo "Verifying deployment..."
sshpass -p "$BOARD_PASS" ssh "${BOARD_USER}@${BOARD_IP}" \
    "ls -la ${BOARD_APP_DIR}/bin/models/*.rknn ${BOARD_APP_DIR}/bin/models/vocab.txt 2>/dev/null || echo 'Some files missing'"

echo ""
echo "=== BGE Model Deployment Complete ==="
echo ""
echo "Notes:"
echo "  - The BGE model enables semantic search on the board"
echo "  - Model size: ~47MB, may take a few seconds to load"
echo "  - If performance is poor, consider using keyword search as fallback"
