#!/usr/bin/env python3
"""测试NCHW格式的RKNN模型 - 尝试不同的预处理"""
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
    try:
        out = rknn.inference(inputs=[img])
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
print(f"\nLoaded porn: {img.shape}")

# 方案1: BGR uint8 (RKNN会自动减均值)
bgr = img[:, :, ::-1].copy()
nchw = np.transpose(bgr, (2, 0, 1))
nchw = np.expand_dims(nchw, axis=0)
test(rknn, nchw.astype(np.uint8), "BGR_uint8")

# 方案2: RGB uint8 (看看RKNN是否期望RGB)
rgb = img.copy()
nchw_rgb = np.transpose(rgb, (2, 0, 1))
nchw_rgb = np.expand_dims(nchw_rgb, axis=0)
test(rknn, nchw_rgb.astype(np.uint8), "RGB_uint8")

# 方案3: BGR float32 已减均值 (手动预处理)
bgr_f = img[:, :, ::-1].astype(np.float32)
bgr_f[:, :, 0] -= 104.0
bgr_f[:, :, 1] -= 117.0
bgr_f[:, :, 2] -= 123.0
nchw_f = np.transpose(bgr_f, (2, 0, 1))
nchw_f = np.expand_dims(nchw_f, axis=0)
test(rknn, nchw_f, "BGR_float32_mean_subtracted")

# 方案4: RGB float32 已减均值
rgb_f = img.astype(np.float32)
rgb_f[:, :, 0] -= 123.0  # R
rgb_f[:, :, 1] -= 117.0  # G
rgb_f[:, :, 2] -= 104.0  # B
nchw_rf = np.transpose(rgb_f, (2, 0, 1))
nchw_rf = np.expand_dims(nchw_rf, axis=0)
test(rknn, nchw_rf, "RGB_float32_mean_subtracted")

rknn.release()
print("\nDONE!")
