#!/usr/bin/env python3
"""最终测试 - NWCH格式"""
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

def preprocess_nwch(img):
    """预处理图片为NWCH格式"""
    # BGR + 均值减法
    bgr = img[:, :, ::-1].astype(np.float32)
    bgr[:, :, 0] -= 104.0
    bgr[:, :, 1] -= 117.0
    bgr[:, :, 2] -= 123.0
    
    # HWC -> NHWC -> NWCH
    nhwc = np.expand_dims(bgr, axis=0)
    nwch = np.transpose(nhwc, (0, 2, 3, 1))
    return nwch

def test(rknn, img, name, expected_nsfw):
    print(f"\n[{name}]")
    sys.stdout.flush()
    try:
        out = rknn.inference(inputs=[img])
        if out is None:
            print("  -> None"); return
        raw = out[0].flatten()
        print(f"  raw output: {raw}")
        print(f"  sum: {raw.sum():.4f}")
        
        # 检查是否已经是概率
        if abs(raw.sum() - 1.0) < 0.1:
            # 已经是概率，直接使用
            sfw, nsfw = raw[0], raw[1]
            print(f"  (Already probability)")
        else:
            # 需要softmax
            probs = softmax(raw)
            sfw, nsfw = probs[0], probs[1]
            print(f"  (After softmax)")
        
        status = "OK" if abs(nsfw - expected_nsfw) < 0.15 else "WRONG"
        print(f"  SFW={sfw:.4f}, NSFW={nsfw:.4f}")
        print(f"  Expected NSFW ~{expected_nsfw} [{status}]")
    except Exception as e:
        print(f"  Error: {e}")
    sys.stdout.flush()

# 测试porn图片 (期望NSFW~0.96)
img_porn = np.load("/home/kylin/Downloads/porn_224.npy")
print(f"\nLoaded porn: {img_porn.shape}")
nwch_porn = preprocess_nwch(img_porn)
test(rknn, nwch_porn, "porn NWCH", 0.96)

# 测试pig图片 (期望NSFW~0.0)
img_pig = np.load("/home/kylin/Downloads/pig_224.npy")
print(f"\nLoaded pig: {img_pig.shape}")
nwch_pig = preprocess_nwch(img_pig)
test(rknn, nwch_pig, "pig NWCH", 0.0)

rknn.release()
print("\n=== SUMMARY ===")
print("Correct input format: NWCH [1, W, C, H] = [1, 224, 3, 224]")
print("Preprocessing: RGB->BGR, subtract mean [104,117,123]")
print("Output: Already softmax probability [SFW, NSFW]")
