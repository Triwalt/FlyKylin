#!/usr/bin/env python3
"""
使用RKNN的mean_values配置转换模型
尝试不同的配置方式
"""
import os
import sys
import numpy as np

try:
    from rknn.api import RKNN
except ImportError:
    print("Error: rknn-toolkit2 not installed")
    sys.exit(1)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw.onnx")
RKNN_MODEL = os.path.join(PROJECT_ROOT, "model", "rknn", "open_nsfw_rk3566.rknn")

def convert():
    print(f"Input: {ONNX_MODEL}")
    print(f"Output: {RKNN_MODEL}")
    
    rknn = RKNN(verbose=True)
    
    # 尝试设置mean_values
    # open_nsfw期望BGR输入，均值[104, 117, 123]
    # 但输入是RGB，所以均值顺序应该是[123, 117, 104] (R, G, B对应的均值)
    print("Configuring for RK3566 with mean_values...")
    try:
        rknn.config(
            target_platform='rk3566',
            optimization_level=3,
            # 尝试设置均值 - RGB顺序
            mean_values=[[123, 117, 104]],
            std_values=[[1, 1, 1]]
        )
        print("Config with mean_values succeeded!")
    except Exception as e:
        print(f"Config with mean_values failed: {e}")
        print("Trying without mean_values...")
        rknn.config(
            target_platform='rk3566',
            optimization_level=3
        )
    
    print("Loading ONNX model...")
    ret = rknn.load_onnx(
        model=ONNX_MODEL,
        inputs=['input:0'],
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
