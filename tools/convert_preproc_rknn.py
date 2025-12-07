#!/usr/bin/env python3
"""将带预处理的ONNX模型转换为RKNN"""
import os
import sys

try:
    from rknn.api import RKNN
except ImportError:
    print("Error: rknn-toolkit2 not installed")
    sys.exit(1)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw_preproc.onnx")
RKNN_MODEL = os.path.join(PROJECT_ROOT, "model", "rknn", "open_nsfw_rk3566.rknn")

def convert():
    print(f"Input: {ONNX_MODEL}")
    print(f"Output: {RKNN_MODEL}")
    
    if not os.path.exists(ONNX_MODEL):
        print(f"Error: ONNX model not found")
        return False
    
    rknn = RKNN(verbose=True)
    
    # 配置
    print("Configuring for RK3566...")
    rknn.config(
        target_platform='rk3566',
        optimization_level=3
    )
    
    # 加载ONNX - 输入是float32 RGB [0-255]
    print("Loading ONNX model...")
    ret = rknn.load_onnx(
        model=ONNX_MODEL,
        inputs=['input_rgb'],
        input_size_list=[[1, 224, 224, 3]]
    )
    if ret != 0:
        print(f"Load ONNX failed: {ret}")
        return False
    
    # 构建（不量化，保持FP16）
    print("Building RKNN model (FP16)...")
    ret = rknn.build(do_quantization=False)
    if ret != 0:
        print(f"Build failed: {ret}")
        return False
    
    # 导出
    print(f"Exporting...")
    ret = rknn.export_rknn(RKNN_MODEL)
    if ret != 0:
        print(f"Export failed: {ret}")
        return False
    
    rknn.release()
    print(f"Success! Size: {os.path.getsize(RKNN_MODEL) / 1024 / 1024:.2f} MB")
    return True

if __name__ == "__main__":
    sys.exit(0 if convert() else 1)
