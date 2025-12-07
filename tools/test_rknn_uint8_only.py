#!/usr/bin/env python3
"""
RKNN NSFW测试 - 只测试uint8格式，避免float32卡死
"""
import os
import sys
import time
import subprocess
import numpy as np

MODEL_PATH = "/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn"
TEST_IMAGES = [
    "/home/kylin/Downloads/porn.jpg",
    "/home/kylin/Downloads/pig.jpg",
]

def load_rknnlite():
    from rknnlite.api import RKNNLite
    return RKNNLite

def load_image_djpeg(path):
    """使用djpeg解码JPEG"""
    try:
        result = subprocess.run(['djpeg', '-pnm', path], capture_output=True, timeout=10)
        if result.returncode != 0:
            return None
        data = result.stdout
        lines = data.split(b'\n', 3)
        if lines[0] != b'P6':
            return None
        w, h = map(int, lines[1].split())
        pixels = np.frombuffer(lines[3], dtype=np.uint8)
        return pixels.reshape((h, w, 3)), w, h
    except Exception as e:
        print(f"  djpeg error: {e}")
        return None

def resize_nearest(img, src_w, src_h, dst_w=224, dst_h=224):
    """最近邻resize"""
    result = np.zeros((dst_h, dst_w, 3), dtype=np.uint8)
    for y in range(dst_h):
        for x in range(dst_w):
            sy = int(y * src_h / dst_h)
            sx = int(x * src_w / dst_w)
            result[y, x] = img[sy, sx]
    return result

def softmax(x):
    exp_x = np.exp(x - np.max(x))
    return exp_x / exp_x.sum()

def run_test(rknn, img, name):
    print(f"  [{name}] shape={img.shape} dtype={img.dtype} range=[{img.min()},{img.max()}]")
    try:
        t0 = time.time()
        out = rknn.inference(inputs=[img])
        dt = (time.time() - t0) * 1000
        if out is None:
            print(f"    -> No output")
            return None
        raw = out[0].flatten()
        print(f"    -> raw: {raw}, time: {dt:.1f}ms")
        if len(raw) >= 2:
            if abs(raw[0] + raw[1] - 1.0) > 0.1:
                probs = softmax(raw[:2])
                print(f"    -> softmax: SFW={probs[0]:.4f} NSFW={probs[1]:.4f}")
                return probs[1]
            else:
                print(f"    -> SFW={raw[0]:.4f} NSFW={raw[1]:.4f}")
                return raw[1]
        return raw[0]
    except Exception as e:
        print(f"    -> Error: {e}")
        return None

def main():
    print("="*50)
    print("RKNN uint8 Test")
    print("="*50)
    
    RKNNLite = load_rknnlite()
    rknn = RKNNLite(verbose=False)
    
    print(f"Loading {MODEL_PATH}...")
    if rknn.load_rknn(MODEL_PATH) != 0:
        print("Failed to load model")
        return 1
    
    print("Init runtime...")
    if rknn.init_runtime(core_mask=0) != 0:
        print("Failed")
        rknn.release()
        return 1
    print("OK")
    
    for path in TEST_IMAGES:
        print(f"\n--- {os.path.basename(path)} ---")
        if not os.path.exists(path):
            print("  Not found")
            continue
        
        result = load_image_djpeg(path)
        if result is None:
            print("  Failed to load")
            continue
        img, w, h = result
        print(f"  Loaded: {w}x{h}")
        
        img224 = resize_nearest(img, w, h, 224, 224)
        print(f"  Resized: {img224.shape}")
        
        # Test 1: uint8 RGB
        run_test(rknn, img224.copy(), "uint8_RGB")
        
        # Test 2: uint8 BGR
        bgr = img224[:, :, ::-1].copy()
        run_test(rknn, bgr, "uint8_BGR")
    
    rknn.release()
    print("\nDone!")
    return 0

if __name__ == "__main__":
    sys.exit(main())
