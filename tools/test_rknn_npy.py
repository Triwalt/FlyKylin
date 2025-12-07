#!/usr/bin/env python3
"""使用预处理好的npy文件测试RKNN"""
import sys
import numpy as np

print("Import rknnlite")
from rknnlite.api import RKNNLite

rknn = RKNNLite(verbose=False)

print("Load model")
if rknn.load_rknn("/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn") != 0:
    print("FAILED"); sys.exit(1)

print("Init runtime")
if rknn.init_runtime(core_mask=0) != 0:
    rknn.release(); print("FAILED"); sys.exit(1)

def softmax(x):
    e = np.exp(x - np.max(x))
    return e / e.sum()

def test(rknn, img, name):
    print(f"\n[{name}] shape={img.shape} dtype={img.dtype}")
    img_4d = np.expand_dims(img, axis=0)
    out = rknn.inference(inputs=[img_4d])
    if out is None:
        print("  -> None"); return
    raw = out[0].flatten()
    print(f"  raw: {raw}")
    if len(raw) >= 2:
        if abs(raw[0] + raw[1] - 1.0) > 0.1:
            p = softmax(raw[:2])
            print(f"  softmax: SFW={p[0]:.4f} NSFW={p[1]:.4f}")
        else:
            print(f"  SFW={raw[0]:.4f} NSFW={raw[1]:.4f}")

# 测试预处理好的图片
for name in ['porn', 'pig']:
    path = f"/home/kylin/Downloads/{name}_224.npy"
    try:
        img = np.load(path)
        print(f"\nLoaded {name}: {img.shape}")
        
        # uint8 RGB
        test(rknn, img.copy(), f"{name}_uint8_RGB")
        
        # uint8 BGR
        bgr = img[:, :, ::-1].copy()
        test(rknn, bgr, f"{name}_uint8_BGR")
        
    except Exception as e:
        print(f"Error loading {name}: {e}")

rknn.release()
print("\nDONE!")
