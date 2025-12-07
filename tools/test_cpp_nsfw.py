#!/usr/bin/env python3
"""
模拟C++代码的预处理逻辑，验证WCH格式转换是否正确
"""
import numpy as np

def cpp_preprocess(img):
    """
    模拟C++代码的预处理:
    1. RGB -> BGR
    2. 减去均值
    3. HWC -> WCH
    """
    height, width = img.shape[:2]
    
    # 均值
    kMeanB = 104.0
    kMeanG = 117.0
    kMeanR = 123.0
    
    # 输出数组 [W, C, H] = [224, 3, 224]
    output = np.zeros((width, 3, height), dtype=np.float32)
    
    for h in range(height):
        for w in range(width):
            r = float(img[h, w, 0])
            g = float(img[h, w, 1])
            b = float(img[h, w, 2])
            
            # BGR + 均值减法
            bVal = b - kMeanB
            gVal = g - kMeanG
            rVal = r - kMeanR
            
            # 存储为 WCH: output[w][c][h]
            output[w, 0, h] = bVal  # B
            output[w, 1, h] = gVal  # G
            output[w, 2, h] = rVal  # R
    
    return output

def python_preprocess(img):
    """
    Python测试脚本的预处理 (已验证正确)
    """
    # BGR + 均值减法
    bgr = img[:, :, ::-1].astype(np.float32)
    bgr[:, :, 0] -= 104.0
    bgr[:, :, 1] -= 117.0
    bgr[:, :, 2] -= 123.0
    
    # HWC -> NHWC -> NWCH
    nhwc = np.expand_dims(bgr, axis=0)
    nwch = np.transpose(nhwc, (0, 2, 3, 1))
    return nwch[0]  # 去掉N维度，返回 [W, C, H]

# 测试
img = np.load("/home/kylin/Downloads/porn_224.npy")
print(f"Input image: {img.shape}")

cpp_out = cpp_preprocess(img)
py_out = python_preprocess(img)

print(f"C++ output shape: {cpp_out.shape}")
print(f"Python output shape: {py_out.shape}")

# 比较
diff = np.abs(cpp_out - py_out)
print(f"Max difference: {diff.max()}")
print(f"Mean difference: {diff.mean()}")

if diff.max() < 1e-5:
    print("✓ C++ and Python preprocessing match!")
else:
    print("✗ Mismatch detected!")
    # 打印一些样本值
    print(f"C++ [0,0,:]: {cpp_out[0, :, 0]}")
    print(f"Python [0,0,:]: {py_out[0, :, 0]}")
