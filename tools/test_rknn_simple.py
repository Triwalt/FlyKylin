#!/usr/bin/env python3
"""
简化的板端RKNN测试脚本 - 使用预处理好的numpy数据
"""
import numpy as np
import os
import sys
import time

try:
    from rknnlite.api import RKNNLite
except ImportError:
    print("Error: rknnlite not installed")
    sys.exit(1)

# 测试两个模型
MODEL_PATHS = [
    "/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn",  # 当前部署的
    "/home/kylin/open_nsfw_original.rknn"  # 原始模型
]
MODEL_PATH = MODEL_PATHS[0]  # 默认
DATA_PATH = "/home/kylin/test_inputs.npz"

def test_inference(rknn, img, name):
    """运行推理并返回结果"""
    try:
        start = time.time()
        outputs = rknn.inference(inputs=[img])
        elapsed = (time.time() - start) * 1000
        
        if outputs is None or len(outputs) == 0:
            return None, "No output"
        
        out = outputs[0].flatten()
        if len(out) >= 2:
            return (out[0], out[1], elapsed), None
        else:
            return None, f"Unexpected output shape: {out.shape}"
    except Exception as e:
        return None, str(e)

def main():
    print("=== RKNN Simple Test ===")
    print(f"Model: {MODEL_PATH}")
    print(f"Data: {DATA_PATH}")
    print()
    
    if not os.path.exists(MODEL_PATH):
        print(f"Error: Model not found: {MODEL_PATH}")
        return
    
    if not os.path.exists(DATA_PATH):
        print(f"Error: Data not found: {DATA_PATH}")
        return
    
    # 加载测试数据
    print("Loading test data...")
    data = np.load(DATA_PATH)
    print(f"Available keys: {list(data.keys())}")
    print()
    
    # 初始化RKNN
    print("Initializing RKNNLite...")
    rknn = RKNNLite(verbose=False)
    
    ret = rknn.load_rknn(MODEL_PATH)
    if ret != 0:
        print(f"Error: Failed to load model, ret={ret}")
        return
    
    ret = rknn.init_runtime(core_mask=0)
    if ret != 0:
        print(f"Error: Failed to init runtime, ret={ret}")
        rknn.release()
        return
    
    print("Model loaded successfully")
    print()
    
    # 测试每种输入
    for key in sorted(data.keys()):
        img = data[key]
        # 添加batch维度 (H,W,C) -> (1,H,W,C)
        if img.ndim == 3:
            img = np.expand_dims(img, axis=0)
        
        print(f"{key}:")
        print(f"  Shape: {img.shape}, dtype: {img.dtype}")
        print(f"  Range: [{img.min():.1f}, {img.max():.1f}]")
        
        result, error = test_inference(rknn, img, key)
        if error:
            print(f"  Error: {error}")
        else:
            sfw, nsfw, elapsed = result
            print(f"  SFW={sfw:.6f}, NSFW={nsfw:.6f} ({elapsed:.1f}ms)")
        print()
    
    rknn.release()
    print("Done")

if __name__ == "__main__":
    main()
