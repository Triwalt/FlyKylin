#!/usr/bin/env python3
"""测试新模型 - float32 RGB [0-255]输入"""
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

# 测试porn图片
img = np.load("/home/kylin/Downloads/porn_224.npy")
print(f"Loaded porn: {img.shape}")

# 新模型期望: float32 RGB [0-255]
print("\n=== float32 RGB [0-255] (新模型期望格式) ===")
sys.stdout.flush()
rgb_f32 = img.astype(np.float32)
test(rknn, rgb_f32, "float32_RGB_0_255")

# 测试pig图片
img_pig = np.load("/home/kylin/Downloads/pig_224.npy")
print(f"\nLoaded pig: {img_pig.shape}")
rgb_f32_pig = img_pig.astype(np.float32)
test(rknn, rgb_f32_pig, "pig_float32_RGB_0_255")

rknn.release()
print("\nDONE!")
