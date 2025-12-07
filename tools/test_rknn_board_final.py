#!/usr/bin/env python3
"""
板端RKNN最终验证测试
使用预处理好的npy文件测试RKNN模型
"""
import numpy as np
from rknnlite.api import RKNNLite

MODEL_PATH = "/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn"

# 预处理好的npy文件 (NWCH格式 [1, 224, 3, 224])
TEST_FILES = [
    ("/home/kylin/Downloads/porn_nwch.npy", 0.96, "NSFW"),
    ("/home/kylin/Downloads/pig_nwch.npy", 0.0, "SFW"),
]

def main():
    print("="*50)
    print("RKNN Board Final Test")
    print("="*50)
    
    rknn = RKNNLite(verbose=False)
    ret = rknn.load_rknn(MODEL_PATH)
    if ret != 0:
        print(f"Load RKNN failed: {ret}")
        return
    
    ret = rknn.init_runtime()
    if ret != 0:
        print(f"Init runtime failed: {ret}")
        return
    
    print(f"Model loaded: {MODEL_PATH}")
    print()
    
    for npy_path, expected, label in TEST_FILES:
        try:
            data = np.load(npy_path)
            print(f"{label} ({npy_path}):")
            print(f"  Input shape: {data.shape}, dtype: {data.dtype}")
            
            out = rknn.inference(inputs=[data], data_format='nhwc')
            if out is None:
                print(f"  Inference failed!")
                continue
            
            raw = out[0].flatten()
            print(f"  Raw output: {raw}")
            
            # 检查是否需要softmax
            if abs(raw.sum() - 1.0) < 0.1:
                sfw, nsfw = raw[0], raw[1]
            else:
                exp_x = np.exp(raw - np.max(raw))
                probs = exp_x / exp_x.sum()
                sfw, nsfw = probs[0], probs[1]
            
            status = "OK" if abs(nsfw - expected) < 0.15 else "WRONG"
            print(f"  SFW={sfw:.4f}, NSFW={nsfw:.4f} (expected ~{expected}) [{status}]")
            print()
            
        except Exception as e:
            print(f"  Error: {e}")
            print()
    
    rknn.release()
    print("Test complete!")

if __name__ == "__main__":
    main()
