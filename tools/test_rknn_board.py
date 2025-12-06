#!/usr/bin/env python3
"""
板端RKNN测试脚本 - 测试不同输入格式
在RK3566板端运行，需要rknnlite
"""
import numpy as np
import os
import sys
import time

try:
    from rknnlite.api import RKNNLite
except ImportError:
    print("Error: rknnlite not installed")
    sys.exit(1)

# 尝试不同的图像加载方式
USE_PIL = False
USE_CV2 = False

try:
    from PIL import Image
    USE_PIL = True
except ImportError:
    pass

if not USE_PIL:
    try:
        import cv2
        USE_CV2 = True
    except ImportError:
        pass

if not USE_PIL and not USE_CV2:
    print("Warning: Neither PIL nor cv2 available, using raw numpy")

MODEL_PATH = "/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn"
TEST_IMAGES = [
    "/home/kylin/Downloads/porn.jpg",
    "/home/kylin/Downloads/pig.jpg"
]

def load_image_rgb(path, size=(224, 224)):
    """加载图片为RGB uint8"""
    if USE_PIL:
        img = Image.open(path).convert('RGB')
        img = img.resize(size, Image.BILINEAR)
        return np.array(img, dtype=np.uint8)
    elif USE_CV2:
        img = cv2.imread(path)
        img = cv2.resize(img, size)
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        return img
    else:
        # 使用随机数据作为测试
        print(f"Warning: Cannot load {path}, using random data")
        return np.random.randint(0, 255, (size[1], size[0], 3), dtype=np.uint8)

def preprocess_variants(path):
    """生成不同预处理方式的输入"""
    img_rgb = load_image_rgb(path)
    
    variants = {}
    
    # 1. uint8 RGB (当前C++使用)
    variants['uint8_rgb'] = img_rgb.copy()
    
    # 2. uint8 BGR
    variants['uint8_bgr'] = img_rgb[:, :, ::-1].copy()
    
    # 3. float32 RGB (0-255)
    variants['float32_rgb'] = img_rgb.astype(np.float32)
    
    # 4. float32 BGR (0-255)
    variants['float32_bgr'] = img_rgb[:, :, ::-1].astype(np.float32)
    
    # 5. float32 BGR with mean subtraction (ONNX标准)
    img_bgr_mean = img_rgb[:, :, ::-1].astype(np.float32)
    img_bgr_mean[:, :, 0] -= 104.0  # B
    img_bgr_mean[:, :, 1] -= 117.0  # G
    img_bgr_mean[:, :, 2] -= 123.0  # R
    variants['float32_bgr_mean'] = img_bgr_mean
    
    # 6. float32 RGB with mean subtraction (RGB顺序)
    img_rgb_mean = img_rgb.astype(np.float32)
    img_rgb_mean[:, :, 0] -= 123.0  # R
    img_rgb_mean[:, :, 1] -= 117.0  # G
    img_rgb_mean[:, :, 2] -= 104.0  # B
    variants['float32_rgb_mean'] = img_rgb_mean
    
    return variants

def test_inference(rknn, img, name):
    """运行推理并返回结果"""
    try:
        start = time.time()
        outputs = rknn.inference(inputs=[img])
        elapsed = (time.time() - start) * 1000
        
        if outputs is None or len(outputs) == 0:
            return None, "No output"
        
        out = outputs[0].flatten()
        if len(out) >= 2:
            return (out[0], out[1], elapsed), None
        else:
            return None, f"Unexpected output shape: {out.shape}"
    except Exception as e:
        return None, str(e)

def main():
    print("=== RKNN Board Test ===")
    print(f"Model: {MODEL_PATH}")
    print()
    
    if not os.path.exists(MODEL_PATH):
        print(f"Error: Model not found: {MODEL_PATH}")
        return
    
    # 初始化RKNN
    print("Initializing RKNNLite...")
    rknn = RKNNLite(verbose=False)
    
    ret = rknn.load_rknn(MODEL_PATH)
    if ret != 0:
        print(f"Error: Failed to load model, ret={ret}")
        return
    
    ret = rknn.init_runtime(core_mask=0)
    if ret != 0:
        print(f"Error: Failed to init runtime, ret={ret}")
        rknn.release()
        return
    
    print("Model loaded successfully")
    print()
    
    # 测试每张图片
    for img_path in TEST_IMAGES:
        if not os.path.exists(img_path):
            print(f"{img_path}: File not found")
            continue
        
        print(f"=== Testing: {os.path.basename(img_path)} ===")
        variants = preprocess_variants(img_path)
        
        for name, img in variants.items():
            print(f"\n{name}:")
            print(f"  Shape: {img.shape}, dtype: {img.dtype}")
            
            result, error = test_inference(rknn, img, name)
            if error:
                print(f"  Error: {error}")
            else:
                sfw, nsfw, elapsed = result
                print(f"  SFW={sfw:.6f}, NSFW={nsfw:.6f} ({elapsed:.1f}ms)")
        
        print()
    
    rknn.release()
    print("Done")

if __name__ == "__main__":
    main()
