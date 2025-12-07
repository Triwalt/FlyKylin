#!/usr/bin/env python3
"""测试无Softmax的RKNN模型"""
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
        print(f"  raw (logits): {raw}")
        
        # 手动softmax
        probs = softmax(raw)
        print(f"  softmax: SFW={probs[0]:.4f} NSFW={probs[1]:.4f}")
    except Exception as e:
        print(f"  Error: {e}")
    sys.stdout.flush()

# 测试图片
for name in ['porn', 'pig']:
    img = np.load(f"/home/kylin/Downloads/{name}_224.npy")
    print(f"\nLoaded {name}: {img.shape}")
    
    # BGR uint8 (RKNN会自动减均值)
    bgr = img[:, :, ::-1].copy()
    nchw = np.transpose(bgr, (2, 0, 1))
    nchw = np.expand_dims(nchw, axis=0)
    test(rknn, nchw.astype(np.uint8), f"{name}_uint8_NCHW_BGR")

rknn.release()
print("\nDONE!")
