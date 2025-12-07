#!/usr/bin/env python3
"""分析NHWC模型精度"""
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

def analyze():
    # 准备测试数据 - NHWC格式，BGR，已减均值
    img = np.load(os.path.join(SCRIPT_DIR, "porn_224.npy"))
    bgr = img[:, :, ::-1].astype(np.float32)
    bgr[:, :, 0] -= 104.0
    bgr[:, :, 1] -= 117.0
    bgr[:, :, 2] -= 123.0
    nhwc = np.expand_dims(bgr, axis=0)  # [1, 224, 224, 3]
    
    print(f"Input shape: {nhwc.shape}, range: [{nhwc.min():.1f}, {nhwc.max():.1f}]")
    
    # 保存测试数据
    test_data_path = os.path.join(SCRIPT_DIR, "test_input_nhwc.npy")
    np.save(test_data_path, nhwc)
    
    rknn = RKNN(verbose=True)
    
    print("Configuring...")
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
    
    print("Building RKNN model...")
    ret = rknn.build(do_quantization=False)
    if ret != 0:
        print(f"Build failed: {ret}")
        return False
    
    # 在模拟器上测试
    print("\n=== Simulator Test ===")
    rknn.init_runtime(target=None)
    out = rknn.inference(inputs=[nhwc])
    if out:
        raw = out[0].flatten()
        print(f"Simulator output: {raw}")
        print(f"SFW={raw[0]:.4f}, NSFW={raw[1]:.4f}")
    
    rknn.release()
    return True

if __name__ == "__main__":
    analyze()
