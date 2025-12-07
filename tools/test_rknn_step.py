#!/usr/bin/env python3
"""分步测试RKNN - 修复4维输入"""
import sys
import subprocess
import numpy as np

print("Step 1: Import rknnlite")
sys.stdout.flush()
from rknnlite.api import RKNNLite

print("Step 2: Create instance")
sys.stdout.flush()
rknn = RKNNLite(verbose=False)

print("Step 3: Load model")
sys.stdout.flush()
ret = rknn.load_rknn("/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn")
print(f"  ret={ret}")
sys.stdout.flush()
if ret != 0:
    print("FAILED")
    sys.exit(1)

print("Step 4: Init runtime")
sys.stdout.flush()
ret = rknn.init_runtime(core_mask=0)
print(f"  ret={ret}")
sys.stdout.flush()
if ret != 0:
    rknn.release()
    print("FAILED")
    sys.exit(1)

def softmax(x):
    exp_x = np.exp(x - np.max(x))
    return exp_x / exp_x.sum()

def load_image(path):
    """使用djpeg加载图片"""
    result = subprocess.run(['djpeg', '-pnm', path], capture_output=True, timeout=10)
    if result.returncode != 0:
        return None
    data = result.stdout
    lines = data.split(b'\n', 3)
    if lines[0] != b'P6':
        return None
    w, h = map(int, lines[1].split())
    pixels = np.frombuffer(lines[3], dtype=np.uint8)
    img = pixels.reshape((h, w, 3))
    # 简单resize到224x224
    result = np.zeros((224, 224, 3), dtype=np.uint8)
    for y in range(224):
        for x in range(224):
            sy, sx = int(y * h / 224), int(x * w / 224)
            result[y, x] = img[sy, sx]
    return result

def test_inference(rknn, img, name):
    """测试推理 - 输入需要4维 [1,H,W,C]"""
    print(f"\n  [{name}]")
    print(f"    shape={img.shape} dtype={img.dtype} range=[{img.min()},{img.max()}]")
    sys.stdout.flush()
    
    # 添加batch维度
    img_4d = np.expand_dims(img, axis=0)
    print(f"    4D shape={img_4d.shape}")
    sys.stdout.flush()
    
    try:
        out = rknn.inference(inputs=[img_4d])
        if out is None:
            print("    -> None")
            return
        raw = out[0].flatten()
        print(f"    -> raw: {raw}")
        if len(raw) >= 2:
            if abs(raw[0] + raw[1] - 1.0) > 0.1:
                probs = softmax(raw[:2])
                print(f"    -> softmax: SFW={probs[0]:.4f} NSFW={probs[1]:.4f}")
            else:
                print(f"    -> SFW={raw[0]:.4f} NSFW={raw[1]:.4f}")
    except Exception as e:
        print(f"    -> Error: {e}")
    sys.stdout.flush()

# 测试dummy输入
print("\nStep 5: Test with dummy input")
sys.stdout.flush()
dummy = np.full((224, 224, 3), 128, dtype=np.uint8)
test_inference(rknn, dummy, "dummy_128")

# 测试真实图片
test_images = [
    ("/home/kylin/Downloads/porn.jpg", "porn (expect NSFW~0.96)"),
    ("/home/kylin/Downloads/pig.jpg", "pig (expect NSFW~0.0)"),
]

for path, desc in test_images:
    print(f"\nStep: Test {desc}")
    sys.stdout.flush()
    import os
    if not os.path.exists(path):
        print(f"  File not found: {path}")
        continue
    
    img = load_image(path)
    if img is None:
        print(f"  Failed to load")
        continue
    
    print(f"  Loaded image")
    
    # uint8 RGB
    test_inference(rknn, img.copy(), "uint8_RGB")
    
    # uint8 BGR
    bgr = img[:, :, ::-1].copy()
    test_inference(rknn, bgr, "uint8_BGR")

print("\nStep: Release")
sys.stdout.flush()
rknn.release()
print("ALL DONE!")
