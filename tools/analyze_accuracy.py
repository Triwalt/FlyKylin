#!/usr/bin/env python3
"""分析RKNN模型精度"""
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
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw_nchw_no_softmax.onnx")
RKNN_MODEL = os.path.join(PROJECT_ROOT, "model", "rknn", "open_nsfw_rk3566.rknn")

def analyze():
    # 准备测试数据
    img = np.load(os.path.join(SCRIPT_DIR, "porn_224.npy"))
    bgr = img[:, :, ::-1].astype(np.float32)
    bgr[:, :, 0] -= 104.0
    bgr[:, :, 1] -= 117.0
    bgr[:, :, 2] -= 123.0
    nchw = np.transpose(bgr, (2, 0, 1))
    nchw = np.expand_dims(nchw, axis=0)
    
    # 保存测试数据
    test_data_path = os.path.join(SCRIPT_DIR, "test_input.npy")
    np.save(test_data_path, nchw)
    
    rknn = RKNN(verbose=True)
    
    print("Configuring...")
    rknn.config(
        target_platform='rk3566',
        optimization_level=3
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
    
    print("Building RKNN model...")
    ret = rknn.build(do_quantization=False)
    if ret != 0:
        print(f"Build failed: {ret}")
        return False
    
    # 精度分析
    print("\n=== Accuracy Analysis ===")
    try:
        rknn.accuracy_analysis(
            inputs=[test_data_path],
            output_dir=os.path.join(SCRIPT_DIR, "accuracy_analysis"),
            target='rk3566'
        )
    except Exception as e:
        print(f"Accuracy analysis failed: {e}")
    
    # 在模拟器上测试
    print("\n=== Simulator Test ===")
    rknn.init_runtime(target=None)  # 使用模拟器
    out = rknn.inference(inputs=[nchw])
    if out:
        print(f"Simulator output: {out[0].flatten()}")
    
    rknn.release()
    return True

if __name__ == "__main__":
    analyze()
