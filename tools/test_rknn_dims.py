#!/usr/bin/env python3
"""测试不同维度排列"""
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
    print(f"\n[{name}] shape={img.shape}")
    sys.stdout.flush()
    try:
        out = rknn.inference(inputs=[img])
        if out is None:
            print("  -> None"); return
        raw = out[0].flatten()
        probs = softmax(raw)
        print(f"  raw: {raw}")
        print(f"  NSFW={probs[1]:.4f}")
    except Exception as e:
        print(f"  Error: {e}")
    sys.stdout.flush()

# 加载porn图片
img = np.load("/home/kylin/Downloads/porn_224.npy")
print(f"Original image: {img.shape}")

# 预处理: BGR + 均值减法
bgr = img[:, :, ::-1].astype(np.float32)
bgr[:, :, 0] -= 104.0
bgr[:, :, 1] -= 117.0
bgr[:, :, 2] -= 123.0

# 原始HWC: [224, 224, 3]
print(f"BGR preprocessed: {bgr.shape}")

# 尝试所有可能的4D排列
# 原始是 [H, W, C] = [224, 224, 3]
# 需要添加N维度并可能重排

# 方案1: 直接添加N -> [1, 224, 224, 3] (NHWC)
nhwc = np.expand_dims(bgr, axis=0)
test(rknn, nhwc, "NHWC [1,H,W,C]")

# 方案2: transpose(0,1,3,2) -> [1, 224, 3, 224]
nhcw = np.transpose(nhwc, (0, 1, 3, 2))
test(rknn, nhcw, "NHCW [1,H,C,W]")

# 方案3: transpose(0,2,1,3) -> [1, 224, 224, 3]
nwhc = np.transpose(nhwc, (0, 2, 1, 3))
test(rknn, nwhc, "NWHC [1,W,H,C]")

# 方案4: transpose(0,3,1,2) -> [1, 3, 224, 224] (NCHW)
nchw = np.transpose(nhwc, (0, 3, 1, 2))
test(rknn, nchw, "NCHW [1,C,H,W]")

# 方案5: transpose(0,3,2,1) -> [1, 3, 224, 224]
ncwh = np.transpose(nhwc, (0, 3, 2, 1))
test(rknn, ncwh, "NCWH [1,C,W,H]")

# 方案6: transpose(0,2,3,1) -> [1, 224, 3, 224]
nwch = np.transpose(nhwc, (0, 2, 3, 1))
test(rknn, nwch, "NWCH [1,W,C,H]")

rknn.release()
print("\nDONE!")
