#!/usr/bin/env python3
"""
对比测试不同RKNN模型
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

# 测试多个模型
MODEL_PATHS = [
    ("/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn", "deployed"),
    ("/home/kylin/open_nsfw_clean.rknn", "clean")
]

DATA_PATH = "/home/kylin/test_inputs.npz"

def test_model(model_path, model_name, data):
    """测试单个模型"""
    print(f"\n{'='*50}")
    print(f"Testing: {model_name}")
    print(f"Path: {model_path}")
    print(f"{'='*50}")
    
    if not os.path.exists(model_path):
        print(f"Model not found!")
        return
    
    rknn = RKNNLite(verbose=False)
    
    ret = rknn.load_rknn(model_path)
    if ret != 0:
        print(f"Failed to load model, ret={ret}")
        return
    
    ret = rknn.init_runtime(core_mask=0)
    if ret != 0:
        print(f"Failed to init runtime, ret={ret}")
        rknn.release()
        return
    
    print("Model loaded successfully\n")
    
    # 只测试关键的输入格式
    test_keys = ['porn_float32_bgr_mean', 'porn_uint8_rgb', 'pig_float32_bgr_mean', 'pig_uint8_rgb']
    
    for key in test_keys:
        if key not in data:
            continue
        
        img = data[key]
        if img.ndim == 3:
            img = np.expand_dims(img, axis=0)
        
        try:
            start = time.time()
            outputs = rknn.inference(inputs=[img])
            elapsed = (time.time() - start) * 1000
            
            if outputs and len(outputs) > 0:
                out = outputs[0].flatten()
                if len(out) >= 2:
                    print(f"{key}: SFW={out[0]:.4f}, NSFW={out[1]:.4f} ({elapsed:.1f}ms)")
                else:
                    print(f"{key}: Unexpected output")
            else:
                print(f"{key}: No output")
        except Exception as e:
            print(f"{key}: Error - {e}")
    
    rknn.release()

def main():
    print("=== RKNN Model Comparison ===")
    print(f"Data: {DATA_PATH}")
    
    if not os.path.exists(DATA_PATH):
        print(f"Error: Data not found")
        return
    
    data = np.load(DATA_PATH)
    print(f"Keys: {list(data.keys())}")
    
    print("\n--- Expected Results (ONNX) ---")
    print("porn.jpg: SFW=0.0364, NSFW=0.9636")
    print("pig.jpg:  SFW=0.9974, NSFW=0.0026")
    
    for model_path, model_name in MODEL_PATHS:
        test_model(model_path, model_name, data)
    
    print("\nDone")

if __name__ == "__main__":
    main()
