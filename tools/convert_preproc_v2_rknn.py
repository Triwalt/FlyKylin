#!/usr/bin/env python3
"""将v2预处理ONNX模型转换为RKNN"""
import os
import sys

try:
    from rknn.api import RKNN
except ImportError:
    print("Error: rknn-toolkit2 not installed")
    sys.exit(1)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw_preproc_v2.onnx")
RKNN_MODEL = os.path.join(PROJECT_ROOT, "model", "rknn", "open_nsfw_rk3566.rknn")

def convert():
    print(f"Input: {ONNX_MODEL}")
    print(f"Output: {RKNN_MODEL}")
    
    rknn = RKNN(verbose=True)
    
    print("Configuring for RK3566...")
    rknn.config(
        target_platform='rk3566',
        optimization_level=3
    )
    
    print("Loading ONNX model...")
    ret = rknn.load_onnx(
        model=ONNX_MODEL,
        inputs=['input_rgb'],
        input_size_list=[[1, 224, 224, 3]]
    )
    if ret != 0:
        print(f"Load ONNX failed: {ret}")
        return False
    
    print("Building RKNN model (FP16)...")
    ret = rknn.build(do_quantization=False)
    if ret != 0:
        print(f"Build failed: {ret}")
        return False
    
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
