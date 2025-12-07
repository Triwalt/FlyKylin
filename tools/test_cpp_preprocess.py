#!/usr/bin/env python3
"""
验证C++预处理逻辑与Python一致
生成测试数据并比较
"""
import numpy as np
from PIL import Image
import os

def cpp_style_preprocess(img_path):
    """
    模拟C++代码的预处理逻辑 (NSFWDetector.cpp)
    输出格式: NWCH [1, W, C, H] = [1, 224, 3, 224]
    """
    img = Image.open(img_path).convert('RGB')
    img = img.resize((224, 224), Image.BILINEAR)
    img_np = np.array(img, dtype=np.uint8)
    
    height, width = img_np.shape[:2]
    
    kMeanB = 104.0
    kMeanG = 117.0
    kMeanR = 123.0
    
    # 线性数组 [W * C * H]
    output = np.zeros(width * 3 * height, dtype=np.float32)
    
    for h in range(height):
        for w in range(width):
            r = float(img_np[h, w, 0])
            g = float(img_np[h, w, 1])
            b = float(img_np[h, w, 2])
            
            bVal = b - kMeanB
            gVal = g - kMeanG
            rVal = r - kMeanR
            
            # C++代码: index = w * (C * H) + c * H + h
            baseIdx = w * 3 * height
            output[baseIdx + 0 * height + h] = bVal  # B
            output[baseIdx + 1 * height + h] = gVal  # G
            output[baseIdx + 2 * height + h] = rVal  # R
    
    return output.reshape(1, width, 3, height)

def test_with_onnx(img_path):
    """使用ONNX验证预处理"""
    import onnxruntime as ort
    
    onnx_path = "/mnt/e/Project/FlyKylin/model/onnx/open_nsfw.onnx"
    sess = ort.InferenceSession(onnx_path)
    
    # 标准ONNX预处理 (NHWC)
    img = Image.open(img_path).convert('RGB')
    img = img.resize((224, 224), Image.BILINEAR)
    img_np = np.array(img, dtype=np.float32)
    
    # BGR + 均值减法
    bgr = img_np[:, :, ::-1]
    bgr[:, :, 0] -= 104.0
    bgr[:, :, 1] -= 117.0
    bgr[:, :, 2] -= 123.0
    
    inp = np.expand_dims(bgr, axis=0)  # NHWC
    out = sess.run(None, {'input:0': inp})
    
    return out[0][0]

def main():
    test_images = [
        ("/mnt/e/Download/porn.jpg", "NSFW"),
        ("/mnt/e/Pictures/pig.jpg", "SFW"),
    ]
    
    print("="*60)
    print("C++ Preprocess Verification")
    print("="*60)
    
    for img_path, label in test_images:
        if not os.path.exists(img_path):
            print(f"Skip: {img_path} not found")
            continue
        
        print(f"\n{label} ({os.path.basename(img_path)}):")
        
        # ONNX参考结果
        onnx_out = test_with_onnx(img_path)
        print(f"  ONNX Reference: SFW={onnx_out[0]:.4f}, NSFW={onnx_out[1]:.4f}")
        
        # C++风格预处理
        cpp_data = cpp_style_preprocess(img_path)
        print(f"  C++ Preprocess shape: {cpp_data.shape}")
        print(f"  C++ Preprocess range: [{cpp_data.min():.2f}, {cpp_data.max():.2f}]")
        
        # 验证数据一致性
        # 将NWCH转回NHWC进行比较
        nwch = cpp_data[0]  # [W, C, H]
        nhwc = np.zeros((224, 224, 3), dtype=np.float32)
        for h in range(224):
            for w in range(224):
                for c in range(3):
                    nhwc[h, w, c] = nwch[w, c, h]
        
        # 与ONNX输入比较
        img = Image.open(img_path).convert('RGB')
        img = img.resize((224, 224), Image.BILINEAR)
        img_np = np.array(img, dtype=np.float32)
        bgr = img_np[:, :, ::-1]
        bgr[:, :, 0] -= 104.0
        bgr[:, :, 1] -= 117.0
        bgr[:, :, 2] -= 123.0
        
        diff = np.abs(nhwc - bgr).max()
        print(f"  Max diff from ONNX input: {diff:.6f}")
        
        if diff < 0.001:
            print(f"  ✅ C++ preprocess matches ONNX input!")
        else:
            print(f"  ❌ Mismatch detected!")

if __name__ == "__main__":
    main()
