#!/usr/bin/env python3
"""测试NCHW格式的RKNN模型"""
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
        if len(raw) >= 2:
            if abs(raw[0] + raw[1] - 1.0) > 0.1:
                p = softmax(raw[:2])
                print(f"  softmax: SFW={p[0]:.4f} NSFW={p[1]:.4f}")
            else:
                print(f"  SFW={raw[0]:.4f} NSFW={raw[1]:.4f}")
    except Exception as e:
        print(f"  Error: {e}")
    sys.stdout.flush()

# 测试图片
for name in ['porn', 'pig']:
    img = np.load(f"/home/kylin/Downloads/{name}_224.npy")
    print(f"\nLoaded {name}: {img.shape}")
    
    # 模型期望: NCHW BGR uint8 [0-255] (mean_values会自动减去)
    # 输入图片是RGB，需要转BGR
    bgr = img[:, :, ::-1].copy()
    
    # HWC -> CHW -> NCHW
    nchw = np.transpose(bgr, (2, 0, 1))
    nchw = np.expand_dims(nchw, axis=0)
    
    # uint8输入
    test(rknn, nchw.astype(np.uint8), f"{name}_uint8_NCHW_BGR")
    
    # float32输入
    test(rknn, nchw.astype(np.float32), f"{name}_float32_NCHW_BGR")

rknn.release()
print("\nDONE!")
