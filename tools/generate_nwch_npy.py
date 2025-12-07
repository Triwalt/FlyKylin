#!/usr/bin/env python3
"""
生成NWCH格式的预处理npy文件用于板端测试
"""
import numpy as np
from PIL import Image
import os

def cpp_style_preprocess(img_path):
    """
    模拟C++代码的预处理逻辑
    输出格式: NWCH [1, W, C, H] = [1, 224, 3, 224]
    """
    img = Image.open(img_path).convert('RGB')
    img = img.resize((224, 224), Image.BILINEAR)
    img_np = np.array(img, dtype=np.uint8)
    
    height, width = img_np.shape[:2]
    
    kMeanB = 104.0
    kMeanG = 117.0
    kMeanR = 123.0
    
    # 输出数组 [1, W, C, H] = [1, 224, 3, 224]
    output = np.zeros((1, width, 3, height), dtype=np.float32)
    
    for h in range(height):
        for w in range(width):
            r = float(img_np[h, w, 0])
            g = float(img_np[h, w, 1])
            b = float(img_np[h, w, 2])
            
            bVal = b - kMeanB
            gVal = g - kMeanG
            rVal = r - kMeanR
            
            # NWCH: [n, w, c, h]
            output[0, w, 0, h] = bVal  # B
            output[0, w, 1, h] = gVal  # G
            output[0, w, 2, h] = rVal  # R
    
    return output

def main():
    test_images = [
        ("/mnt/e/Download/porn.jpg", "porn_nwch.npy"),
        ("/mnt/e/Pictures/pig.jpg", "pig_nwch.npy"),
    ]
    
    output_dir = "/mnt/e/Project/FlyKylin/tools"
    
    for img_path, out_name in test_images:
        if not os.path.exists(img_path):
            print(f"Skip: {img_path} not found")
            continue
        
        data = cpp_style_preprocess(img_path)
        out_path = os.path.join(output_dir, out_name)
        np.save(out_path, data)
        print(f"Saved: {out_path}")
        print(f"  Shape: {data.shape}, dtype: {data.dtype}")
        print(f"  Range: [{data.min():.2f}, {data.max():.2f}]")

if __name__ == "__main__":
    main()
