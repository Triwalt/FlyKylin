#!/usr/bin/env python3
"""
RKNN NSFW模型调试脚本 - 在RK3566板端测试不同输入格式

使用方法:
1. 将此脚本和测试图片传输到板端
2. 运行: python3 test_rknn_debug.py

测试图片:
- /home/kylin/Downloads/porn.jpg (NSFW, 期望结果~0.96)
- /home/kylin/Downloads/pig.jpg (正常, 期望结果~0.0)
"""

import os
import sys
import time
import numpy as np

# 配置
MODEL_PATH = "/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn"
TEST_IMAGES = [
    "/home/kylin/Downloads/porn.jpg",
    "/home/kylin/Downloads/pig.jpg",
]

# BGR均值 (open_nsfw模型要求)
MEAN_B = 104.0
MEAN_G = 117.0
MEAN_R = 123.0

def load_rknnlite():
    """加载RKNNLite"""
    try:
        from rknnlite.api import RKNNLite
        return RKNNLite
    except ImportError:
        print("Error: rknnlite not installed")
        print("Install with: pip3 install rknnlite")
        sys.exit(1)

def load_image_pil(path):
    """使用PIL加载图片"""
    try:
        from PIL import Image
        img = Image.open(path).convert('RGB')
        img = img.resize((224, 224), Image.BILINEAR)
        return np.array(img, dtype=np.uint8)
    except ImportError:
        return None

def load_image_cv2(path):
    """使用OpenCV加载图片"""
    try:
        import cv2
        img = cv2.imread(path)
        if img is None:
            return None
        img = cv2.resize(img, (224, 224))
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        return img
    except ImportError:
        return None

def load_image(path):
    """加载图片"""
    img = load_image_pil(path)
    if img is None:
        img = load_image_cv2(path)
    return img

def preprocess_uint8_rgb(img):
    """预处理: uint8 RGB (当前C++使用的格式)"""
    return img.astype(np.uint8)

def preprocess_uint8_bgr(img):
    """预处理: uint8 BGR"""
    return img[:, :, ::-1].astype(np.uint8)

def preprocess_float32_rgb(img):
    """预处理: float32 RGB (0-255)"""
    return img.astype(np.float32)

def preprocess_float32_bgr(img):
    """预处理: float32 BGR (0-255)"""
    return img[:, :, ::-1].astype(np.float32)

def preprocess_float32_bgr_mean(img):
    """预处理: float32 BGR + 均值减法 (ONNX使用的格式)"""
    img_f = img[:, :, ::-1].astype(np.float32)  # RGB -> BGR
    img_f[:, :, 0] -= MEAN_B
    img_f[:, :, 1] -= MEAN_G
    img_f[:, :, 2] -= MEAN_R
    return img_f

def preprocess_float32_rgb_mean(img):
    """预处理: float32 RGB + 均值减法 (RGB顺序)"""
    img_f = img.astype(np.float32)
    img_f[:, :, 0] -= MEAN_R
    img_f[:, :, 1] -= MEAN_G
    img_f[:, :, 2] -= MEAN_B
    return img_f

def softmax(x):
    """Softmax函数"""
    exp_x = np.exp(x - np.max(x))
    return exp_x / exp_x.sum()

def run_inference(rknn, img_data, name):
    """运行推理并返回结果"""
    print(f"\n  [{name}]")
    print(f"    Input shape: {img_data.shape}, dtype: {img_data.dtype}")
    print(f"    Input range: [{img_data.min():.2f}, {img_data.max():.2f}]")
    
    try:
        start = time.time()
        outputs = rknn.inference(inputs=[img_data])
        elapsed = (time.time() - start) * 1000
        
        if outputs is None or len(outputs) == 0:
            print(f"    Error: No output")
            return None
        
        out = outputs[0].flatten()
        print(f"    Output: {out}")
        print(f"    Time: {elapsed:.2f}ms")
        
        # 检查是否需要softmax
        if len(out) >= 2:
            sfw, nsfw = out[0], out[1]
            if abs(sfw + nsfw - 1.0) > 0.1:
                # 需要softmax
                probs = softmax(out[:2])
                sfw, nsfw = probs[0], probs[1]
                print(f"    After softmax: SFW={sfw:.6f}, NSFW={nsfw:.6f}")
            else:
                print(f"    Result: SFW={sfw:.6f}, NSFW={nsfw:.6f}")
            return nsfw
        else:
            print(f"    Single output: {out[0]:.6f}")
            return out[0]
            
    except Exception as e:
        print(f"    Error: {e}")
        return None

def test_image(rknn, image_path):
    """测试单张图片的不同预处理方式"""
    print(f"\n{'='*60}")
    print(f"Testing: {image_path}")
    print(f"{'='*60}")
    
    if not os.path.exists(image_path):
        print(f"  Error: Image not found")
        return
    
    # 加载原始图片
    img = load_image(image_path)
    if img is None:
        print(f"  Error: Failed to load image")
        return
    
    print(f"  Original image shape: {img.shape}")
    
    # 测试不同预处理方式
    preprocess_methods = [
        ("uint8_rgb", preprocess_uint8_rgb),
        ("uint8_bgr", preprocess_uint8_bgr),
        ("float32_rgb", preprocess_float32_rgb),
        ("float32_bgr", preprocess_float32_bgr),
        ("float32_bgr_mean", preprocess_float32_bgr_mean),
        ("float32_rgb_mean", preprocess_float32_rgb_mean),
    ]
    
    results = {}
    for name, preprocess_fn in preprocess_methods:
        try:
            img_data = preprocess_fn(img)
            result = run_inference(rknn, img_data, name)
            results[name] = result
        except Exception as e:
            print(f"\n  [{name}] Error: {e}")
            results[name] = None
    
    # 总结
    print(f"\n  Summary for {os.path.basename(image_path)}:")
    for name, result in results.items():
        if result is not None:
            print(f"    {name}: NSFW={result:.6f}")
        else:
            print(f"    {name}: FAILED")

def main():
    print("="*60)
    print("RKNN NSFW Model Debug Test")
    print("="*60)
    
    # 检查模型
    if not os.path.exists(MODEL_PATH):
        print(f"Error: Model not found: {MODEL_PATH}")
        return 1
    
    print(f"Model: {MODEL_PATH}")
    
    # 初始化RKNN
    RKNNLite = load_rknnlite()
    rknn = RKNNLite(verbose=False)
    
    print("Loading model...")
    ret = rknn.load_rknn(MODEL_PATH)
    if ret != 0:
        print(f"Error: Failed to load model, ret={ret}")
        return 1
    
    print("Initializing runtime...")
    ret = rknn.init_runtime(core_mask=0)
    if ret != 0:
        print(f"Error: Failed to init runtime, ret={ret}")
        rknn.release()
        return 1
    
    # 获取SDK版本
    try:
        sdk_version = rknn.get_sdk_version()
        print(f"SDK version: {sdk_version}")
    except:
        pass
    
    # 测试每张图片
    for image_path in TEST_IMAGES:
        test_image(rknn, image_path)
    
    # 释放资源
    print("\nReleasing resources...")
    rknn.release()
    print("Done!")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
