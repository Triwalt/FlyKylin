#!/usr/bin/env python3
"""测试RKNN模型 - 使用NHCW维度顺序"""
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
    sys.stdout.flush()
    try:
        out = rknn.inference(inputs=[img])
        if out is None:
            print("  -> None"); return
        raw = out[0].flatten()
        print(f"  raw: {raw}")
        probs = softmax(raw)
        print(f"  softmax: SFW={probs[0]:.4f} NSFW={probs[1]:.4f}")
    except Exception as e:
        print(f"  Error: {e}")
    sys.stdout.flush()

# 测试porn图片
img = np.load("/home/kylin/Downloads/porn_224.npy")
print(f"\nLoaded porn: {img.shape}")

# 预处理: BGR + 均值减法
bgr = img[:, :, ::-1].astype(np.float32)
bgr[:, :, 0] -= 104.0
bgr[:, :, 1] -= 117.0
bgr[:, :, 2] -= 123.0

# 原始NHWC: [1, 224, 224, 3]
nhwc = np.expand_dims(bgr, axis=0)
print(f"NHWC shape: {nhwc.shape}")
test(rknn, nhwc, "NHWC [1,224,224,3]")

# 转换为NHCW: [1, 224, 3, 224]
# 从 [N,H,W,C] 转换为 [N,H,C,W]
# 即交换最后两个维度
nhcw = np.transpose(nhwc, (0, 1, 3, 2))
print(f"NHCW shape: {nhcw.shape}")
test(rknn, nhcw, "NHCW [1,224,3,224]")

# 测试pig图片
img_pig = np.load("/home/kylin/Downloads/pig_224.npy")
print(f"\nLoaded pig: {img_pig.shape}")

bgr_pig = img_pig[:, :, ::-1].astype(np.float32)
bgr_pig[:, :, 0] -= 104.0
bgr_pig[:, :, 1] -= 117.0
bgr_pig[:, :, 2] -= 123.0

nhwc_pig = np.expand_dims(bgr_pig, axis=0)
nhcw_pig = np.transpose(nhwc_pig, (0, 1, 3, 2))
test(rknn, nhcw_pig, "pig NHCW [1,224,3,224]")

rknn.release()
print("\nDONE!")
