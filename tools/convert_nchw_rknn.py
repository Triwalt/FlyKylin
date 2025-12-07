#!/usr/bin/env python3
"""
将NCHW格式的ONNX模型转换为RKNN
使用mean_values配置预处理
"""
import os
import sys

try:
    from rknn.api import RKNN
except ImportError:
    print("Error: rknn-toolkit2 not installed")
    sys.exit(1)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw_nchw.onnx")
RKNN_MODEL = os.path.join(PROJECT_ROOT, "model", "rknn", "open_nsfw_rk3566.rknn")

def convert():
    print(f"Input: {ONNX_MODEL}")
    print(f"Output: {RKNN_MODEL}")
    
    rknn = RKNN(verbose=True)
    
    # NCHW格式，输入是BGR，均值[104, 117, 123]
    # 但我们希望输入RGB，所以需要在C++中做RGB->BGR转换
    # 或者这里设置RGB顺序的均值[123, 117, 104]
    print("Configuring for RK3566 with mean_values (BGR)...")
    rknn.config(
        target_platform='rk3566',
        optimization_level=3,
        # BGR均值 [B=104, G=117, R=123]
        mean_values=[[104, 117, 123]],
        std_values=[[1, 1, 1]]
    )
    
    print("Loading ONNX model...")
    ret = rknn.load_onnx(
        model=ONNX_MODEL,
        inputs=['input_nchw'],
        input_size_list=[[1, 3, 224, 224]]
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
