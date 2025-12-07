#!/usr/bin/env python3
"""检查RKNN模型的输入shape"""
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

def check():
    rknn = RKNN(verbose=False)
    
    print("Configuring...")
    rknn.config(target_platform='rk3566')
    
    print("Loading ONNX model...")
    ret = rknn.load_onnx(
        model=ONNX_MODEL,
        inputs=['input:0'],
        input_size_list=[[1, 224, 224, 3]]
    )
    if ret != 0:
        print(f"Load ONNX failed: {ret}")
        return
    
    print("Building RKNN model...")
    ret = rknn.build(do_quantization=False)
    if ret != 0:
        print(f"Build failed: {ret}")
        return
    
    # 初始化模拟器
    print("Init simulator...")
    rknn.init_runtime(target=None)
    
    # 尝试不同的输入shape
    test_shapes = [
        (1, 224, 224, 3),  # NHWC
        (1, 224, 3, 224),  # N H C W ?
        (1, 3, 224, 224),  # NCHW
    ]
    
    for shape in test_shapes:
        print(f"\nTrying shape {shape}...")
        try:
            dummy = np.zeros(shape, dtype=np.float32)
            out = rknn.inference(inputs=[dummy])
            if out:
                print(f"  Success! Output: {out[0].flatten()}")
        except Exception as e:
            print(f"  Failed: {e}")
    
    rknn.release()

if __name__ == "__main__":
    check()
