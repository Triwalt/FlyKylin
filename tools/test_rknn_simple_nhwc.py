#!/usr/bin/env python3
"""
测试简单NHWC格式 (不做WCH转换)
"""
import numpy as np
from rknnlite.api import RKNNLite

MODEL_PATH = "/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn"

def main():
    print("="*50)
    print("RKNN Simple NHWC Test")
    print("="*50)
    
    # 加载NWCH格式数据
    nwch_data = np.load("/home/kylin/Downloads/porn_nwch.npy")
    print(f"NWCH data shape: {nwch_data.shape}")  # [1, 224, 3, 224]
    
    # 将NWCH转换回NHWC
    # NWCH: [1, W, C, H] -> NHWC: [1, H, W, C]
    nhwc_data = np.zeros((1, 224, 224, 3), dtype=np.float32)
    for h in range(224):
        for w in range(224):
            for c in range(3):
                nhwc_data[0, h, w, c] = nwch_data[0, w, c, h]
    
    print(f"NHWC data shape: {nhwc_data.shape}")
    print(f"NHWC data range: [{nhwc_data.min():.2f}, {nhwc_data.max():.2f}]")
    
    rknn = RKNNLite(verbose=False)
    ret = rknn.load_rknn(MODEL_PATH)
    if ret != 0:
        print(f"Load RKNN failed: {ret}")
        return
    
    ret = rknn.init_runtime()
    if ret != 0:
        print(f"Init runtime failed: {ret}")
        return
    
    print(f"\nModel loaded: {MODEL_PATH}")
    print()
    
    # 测试1: NHWC格式 (标准格式)
    print("Test 1: NHWC format (standard)")
    try:
        out = rknn.inference(inputs=[nhwc_data], data_format='nhwc')
        if out:
            raw = out[0].flatten()
            print(f"  Raw: {raw}")
            if abs(raw.sum() - 1.0) < 0.1:
                print(f"  SFW={raw[0]:.4f}, NSFW={raw[1]:.4f}")
            else:
                exp_x = np.exp(raw - np.max(raw))
                probs = exp_x / exp_x.sum()
                print(f"  SFW={probs[0]:.4f}, NSFW={probs[1]:.4f}")
    except Exception as e:
        print(f"  Error: {e}")
    print()
    
    # 测试2: NWCH格式 (C++当前使用的格式)
    print("Test 2: NWCH format (C++ current)")
    try:
        out = rknn.inference(inputs=[nwch_data], data_format='nhwc')
        if out:
            raw = out[0].flatten()
            print(f"  Raw: {raw}")
            if abs(raw.sum() - 1.0) < 0.1:
                print(f"  SFW={raw[0]:.4f}, NSFW={raw[1]:.4f}")
            else:
                exp_x = np.exp(raw - np.max(raw))
                probs = exp_x / exp_x.sum()
                print(f"  SFW={probs[0]:.4f}, NSFW={probs[1]:.4f}")
    except Exception as e:
        print(f"  Error: {e}")
    print()
    
    rknn.release()
    print("Test complete!")

if __name__ == "__main__":
    main()
