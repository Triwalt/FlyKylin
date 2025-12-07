#!/usr/bin/env python3
"""测试float32输入格式"""
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

# BGR均值
MEAN_B, MEAN_G, MEAN_R = 104.0, 117.0, 123.0

def softmax(x):
    e = np.exp(x - np.max(x))
    return e / e.sum()

def test(rknn, img, name):
    print(f"\n[{name}] shape={img.shape} dtype={img.dtype} range=[{img.min():.1f},{img.max():.1f}]")
    sys.stdout.flush()
    img_4d = np.expand_dims(img, axis=0)
    try:
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
    except Exception as e:
        print(f"  Error: {e}")
    sys.stdout.flush()

# 只测试porn图片
img = np.load("/home/kylin/Downloads/porn_224.npy")
print(f"Loaded porn: {img.shape}")

# 1. float32 BGR + 均值减法 (ONNX原始格式)
print("\n=== float32 BGR + mean subtraction ===")
sys.stdout.flush()
bgr = img[:, :, ::-1].astype(np.float32)
bgr[:, :, 0] -= MEAN_B
bgr[:, :, 1] -= MEAN_G
bgr[:, :, 2] -= MEAN_R
test(rknn, bgr, "float32_BGR_mean")

# 2. float32 RGB + 均值减法
print("\n=== float32 RGB + mean subtraction ===")
sys.stdout.flush()
rgb = img.astype(np.float32)
rgb[:, :, 0] -= MEAN_R
rgb[:, :, 1] -= MEAN_G
rgb[:, :, 2] -= MEAN_B
test(rknn, rgb, "float32_RGB_mean")

# 3. float32 BGR 无均值
print("\n=== float32 BGR no mean ===")
sys.stdout.flush()
bgr_raw = img[:, :, ::-1].astype(np.float32)
test(rknn, bgr_raw, "float32_BGR_raw")

rknn.release()
print("\nDONE!")
