#!/usr/bin/env python3
"""
RKNN NSFW模型调试脚本 - 简化版，不依赖PIL/OpenCV
使用numpy直接读取JPEG文件的原始字节，然后手动解码
"""

import os
import sys
import time
import struct
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
        sys.exit(1)

def decode_jpeg_simple(path):
    """尝试使用系统工具解码JPEG"""
    import subprocess
    import tempfile
    
    # 尝试使用djpeg (libjpeg-turbo)
    try:
        result = subprocess.run(
            ['djpeg', '-pnm', path],
            capture_output=True,
            timeout=10
        )
        if result.returncode == 0:
            # 解析PPM格式
            data = result.stdout
            # PPM格式: P6\n<width> <height>\n<maxval>\n<binary data>
            lines = data.split(b'\n', 3)
            if lines[0] == b'P6':
                w, h = map(int, lines[1].split())
                maxval = int(lines[2])
                pixels = np.frombuffer(lines[3], dtype=np.uint8)
                img = pixels.reshape((h, w, 3))
                return img
    except Exception as e:
        print(f"  djpeg failed: {e}")
    
    # 尝试使用ImageMagick convert
    try:
        with tempfile.NamedTemporaryFile(suffix='.ppm', delete=False) as f:
            tmp_path = f.name
        result = subprocess.run(
            ['convert', path, '-resize', '224x224!', 'ppm:' + tmp_path],
            capture_output=True,
            timeout=30
        )
        if result.returncode == 0:
            with open(tmp_path, 'rb') as f:
                data = f.read()
            os.unlink(tmp_path)
            # 解析PPM
            lines = data.split(b'\n', 3)
            if lines[0] == b'P6':
                w, h = map(int, lines[1].split())
                pixels = np.frombuffer(lines[3], dtype=np.uint8)
                img = pixels.reshape((h, w, 3))
                return img
    except Exception as e:
        print(f"  ImageMagick failed: {e}")
    
    return None

def load_image(path):
    """加载并resize图片到224x224"""
    # 先尝试PIL
    try:
        from PIL import Image
        img = Image.open(path).convert('RGB')
        img = img.resize((224, 224), Image.BILINEAR)
        return np.array(img, dtype=np.uint8)
    except ImportError:
        pass
    except Exception as e:
        print(f"  PIL error: {e}")
    
    # 尝试OpenCV
    try:
        import cv2
        img = cv2.imread(path)
        if img is not None:
            img = cv2.resize(img, (224, 224))
            img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
            return img
    except ImportError:
        pass
    except Exception as e:
        print(f"  OpenCV error: {e}")
    
    # 尝试系统工具
    img = decode_jpeg_simple(path)
    if img is not None:
        # 简单resize (最近邻)
        if img.shape[0] != 224 or img.shape[1] != 224:
            h, w = img.shape[:2]
            new_img = np.zeros((224, 224, 3), dtype=np.uint8)
            for y in range(224):
                for x in range(224):
                    src_y = int(y * h / 224)
                    src_x = int(x * w / 224)
                    new_img[y, x] = img[src_y, src_x]
            img = new_img
        return img
    
    return None

def softmax(x):
    """Softmax函数"""
    exp_x = np.exp(x - np.max(x))
    return exp_x / exp_x.sum()

def run_inference(rknn, img_data, name):
    """运行推理"""
    print(f"\n  [{name}]")
    print(f"    Shape: {img_data.shape}, dtype: {img_data.dtype}")
    print(f"    Range: [{img_data.min():.2f}, {img_data.max():.2f}]")
    
    try:
        start = time.time()
        outputs = rknn.inference(inputs=[img_data])
        elapsed = (time.time() - start) * 1000
        
        if outputs is None or len(outputs) == 0:
            print(f"    Error: No output")
            return None
        
        out = outputs[0].flatten()
        print(f"    Raw output: {out}")
        print(f"    Time: {elapsed:.2f}ms")
        
        if len(out) >= 2:
            sfw, nsfw = out[0], out[1]
            if abs(sfw + nsfw - 1.0) > 0.1:
                probs = softmax(out[:2])
                sfw, nsfw = probs[0], probs[1]
                print(f"    After softmax: SFW={sfw:.4f}, NSFW={nsfw:.4f}")
            else:
                print(f"    Result: SFW={sfw:.4f}, NSFW={nsfw:.4f}")
            return nsfw
        return out[0]
    except Exception as e:
        print(f"    Error: {e}")
        return None

def test_image(rknn, image_path):
    """测试单张图片"""
    print(f"\n{'='*50}")
    print(f"Testing: {os.path.basename(image_path)}")
    print(f"{'='*50}")
    
    if not os.path.exists(image_path):
        print(f"  File not found!")
        return
    
    img = load_image(image_path)
    if img is None:
        print(f"  Failed to load image")
        return
    
    print(f"  Loaded: {img.shape}")
    
    # 测试不同预处理
    results = {}
    
    # 1. uint8 RGB (当前C++使用)
    data = img.astype(np.uint8)
    results['uint8_rgb'] = run_inference(rknn, data, 'uint8_rgb')
    
    # 2. uint8 BGR
    data = img[:, :, ::-1].astype(np.uint8)
    results['uint8_bgr'] = run_inference(rknn, data, 'uint8_bgr')
    
    # 3. float32 BGR + 均值减法 (ONNX原始格式)
    data = img[:, :, ::-1].astype(np.float32)
    data[:, :, 0] -= MEAN_B
    data[:, :, 1] -= MEAN_G
    data[:, :, 2] -= MEAN_R
    results['float32_bgr_mean'] = run_inference(rknn, data, 'float32_bgr_mean')
    
    # 4. float32 RGB + 均值减法
    data = img.astype(np.float32)
    data[:, :, 0] -= MEAN_R
    data[:, :, 1] -= MEAN_G
    data[:, :, 2] -= MEAN_B
    results['float32_rgb_mean'] = run_inference(rknn, data, 'float32_rgb_mean')
    
    print(f"\n  Summary:")
    for name, val in results.items():
        if val is not None:
            print(f"    {name}: NSFW={val:.4f}")

def main():
    print("="*50)
    print("RKNN NSFW Debug Test (Simple)")
    print("="*50)
    
    if not os.path.exists(MODEL_PATH):
        print(f"Model not found: {MODEL_PATH}")
        return 1
    
    RKNNLite = load_rknnlite()
    rknn = RKNNLite(verbose=False)
    
    print("Loading model...")
    if rknn.load_rknn(MODEL_PATH) != 0:
        print("Failed to load model")
        return 1
    
    print("Init runtime...")
    if rknn.init_runtime(core_mask=0) != 0:
        print("Failed to init runtime")
        rknn.release()
        return 1
    
    for path in TEST_IMAGES:
        test_image(rknn, path)
    
    rknn.release()
    print("\nDone!")
    return 0

if __name__ == "__main__":
    sys.exit(main())
