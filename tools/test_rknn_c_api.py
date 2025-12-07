#!/usr/bin/env python3
"""
测试模拟C++ API的调用方式
使用rknnlite但模拟C++ rknn_api的行为
"""
import numpy as np
from rknnlite.api import RKNNLite
from PIL import Image
import os

MODEL_PATH = "/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn"
IMG_PATH = "/home/kylin/Downloads/porn.jpg"

def preprocess_hwc_bgr(img_path):
    """
    简单的HWC BGR预处理 (不做WCH转换)
    """
    img = Image.open(img_path).convert('RGB')
    img = img.resize((224, 224), Image.BILINEAR)
    img_np = np.array(img, dtype=np.float32)
    
    # RGB -> BGR
    bgr = img_np[:, :, ::-1].copy()
    
    # 减去均值
    bgr[:, :, 0] -= 104.0
    bgr[:, :, 1] -= 117.0
    bgr[:, :, 2] -= 123.0
    
    return np.expand_dims(bgr, axis=0)  # [1, 224, 224, 3] NHWC

def preprocess_wch(img_path):
    """
    WCH格式预处理 (模拟C++代码)
    """
    img = Image.open(img_path).convert('RGB')
    img = img.resize((224, 224), Image.BILINEAR)
    img_np = np.array(img, dtype=np.uint8)
    
    height, width = 224, 224
    output = np.zeros((1, width, 3, height), dtype=np.float32)
    
    for h in range(height):
        for w in range(width):
            r = float(img_np[h, w, 0])
            g = float(img_np[h, w, 1])
            b = float(img_np[h, w, 2])
            
            output[0, w, 0, h] = b - 104.0
            output[0, w, 1, h] = g - 117.0
            output[0, w, 2, h] = r - 123.0
    
    return output  # [1, 224, 3, 224] NWCH

def main():
    print("="*50)
    print("RKNN C API Simulation Test")
    print("="*50)
    
    if not os.path.exists(IMG_PATH):
        print(f"Image not found: {IMG_PATH}")
        return
    
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
    print(f"Image: {IMG_PATH}")
    print()
    
    # 测试1: 简单NHWC BGR (不做WCH转换)
    print("Test 1: Simple NHWC BGR (no WCH conversion)")
    data1 = preprocess_hwc_bgr(IMG_PATH)
    print(f"  Shape: {data1.shape}, dtype: {data1.dtype}")
    try:
        out = rknn.inference(inputs=[data1], data_format='nhwc')
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
    
    # 测试2: NWCH格式
    print("Test 2: NWCH format (C++ style)")
    data2 = preprocess_wch(IMG_PATH)
    print(f"  Shape: {data2.shape}, dtype: {data2.dtype}")
    try:
        out = rknn.inference(inputs=[data2], data_format='nhwc')
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
