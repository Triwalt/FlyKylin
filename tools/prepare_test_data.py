#!/usr/bin/env python3
"""
在PC端准备测试数据，保存为numpy格式供板端使用
"""
import numpy as np
from PIL import Image
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)

TEST_IMAGES = [
    ("/mnt/e/Download/porn.jpg", "porn"),
    ("/mnt/e/Pictures/pig.jpg", "pig")
]

OUTPUT_DIR = os.path.join(PROJECT_ROOT, "tools", "test_data")

def prepare_variants(path, name):
    """准备不同预处理方式的输入数据"""
    img = Image.open(path).convert('RGB')
    img = img.resize((224, 224), Image.BILINEAR)
    img_rgb = np.array(img, dtype=np.uint8)
    
    variants = {}
    
    # 1. uint8 RGB
    variants[f'{name}_uint8_rgb'] = img_rgb.copy()
    
    # 2. uint8 BGR
    variants[f'{name}_uint8_bgr'] = img_rgb[:, :, ::-1].copy()
    
    # 3. float32 BGR with mean subtraction (ONNX标准)
    img_bgr_mean = img_rgb[:, :, ::-1].astype(np.float32)
    img_bgr_mean[:, :, 0] -= 104.0  # B
    img_bgr_mean[:, :, 1] -= 117.0  # G
    img_bgr_mean[:, :, 2] -= 123.0  # R
    variants[f'{name}_float32_bgr_mean'] = img_bgr_mean
    
    return variants

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    all_data = {}
    for path, name in TEST_IMAGES:
        if not os.path.exists(path):
            print(f"Warning: {path} not found")
            continue
        
        print(f"Processing {name}...")
        variants = prepare_variants(path, name)
        all_data.update(variants)
    
    # 保存为单个npz文件
    output_path = os.path.join(OUTPUT_DIR, "test_inputs.npz")
    np.savez(output_path, **all_data)
    print(f"Saved to {output_path}")
    print(f"Keys: {list(all_data.keys())}")

if __name__ == "__main__":
    main()
