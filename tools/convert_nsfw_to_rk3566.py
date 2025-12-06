#!/usr/bin/env python3
"""
将 open_nsfw.onnx 模型转换为 RK3566 兼容的 RKNN 模型

open_nsfw模型预处理要求：
- 输入: NHWC [1, 224, 224, 3]
- 颜色: BGR
- 均值减法: [B=104, G=117, R=123]

由于RKNN的mean_values在NHWC格式下期望长度为H(224)而不是C(3)，
我们需要使用quantization来嵌入预处理，或者接受在C++中手动处理。

当前方案：使用量化并提供校准数据集来嵌入预处理
"""

import os
import sys
import numpy as np

try:
    from rknn.api import RKNN
except ImportError:
    print("Error: rknn-toolkit2 not installed")
    print("Install with: pip install rknn-toolkit2")
    sys.exit(1)

# 路径配置
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw.onnx")
RKNN_MODEL = os.path.join(PROJECT_ROOT, "model", "rknn", "open_nsfw_rk3566.rknn")

def create_dataset_file():
    """创建校准数据集文件"""
    dataset_path = os.path.join(SCRIPT_DIR, "dataset.txt")
    # 使用测试图片作为校准数据
    test_images = [
        "/mnt/e/Download/porn.jpg",
        "/mnt/e/Download/test.jpg",
    ]
    existing = [p for p in test_images if os.path.exists(p)]
    if not existing:
        print("Warning: No calibration images found, using random data")
        return None
    
    with open(dataset_path, 'w') as f:
        for img in existing:
            f.write(img + '\n')
    return dataset_path

def convert_model_fp16():
    """转换为FP16模型（不量化）"""
    print(f"=== Converting to FP16 RKNN ===")
    print(f"Input: {ONNX_MODEL}")
    print(f"Output: {RKNN_MODEL}")
    
    if not os.path.exists(ONNX_MODEL):
        print(f"Error: ONNX model not found: {ONNX_MODEL}")
        return False
    
    rknn = RKNN(verbose=True)
    
    # 配置 - 不设置mean_values（会在C++中处理）
    print("Configuring model for RK3566 (FP16)...")
    rknn.config(
        target_platform='rk3566',
        optimization_level=3
    )
    
    # 加载ONNX
    print("Loading ONNX model...")
    ret = rknn.load_onnx(
        model=ONNX_MODEL,
        inputs=['input:0'],
        input_size_list=[[1, 224, 224, 3]]
    )
    if ret != 0:
        print(f"Load ONNX failed: {ret}")
        return False
    
    # 构建（不量化）
    print("Building RKNN model (FP16)...")
    ret = rknn.build(do_quantization=False)
    if ret != 0:
        print(f"Build failed: {ret}")
        return False
    
    # 导出
    print(f"Exporting to {RKNN_MODEL}...")
    ret = rknn.export_rknn(RKNN_MODEL)
    if ret != 0:
        print(f"Export failed: {ret}")
        return False
    
    rknn.release()
    print(f"Success! Size: {os.path.getsize(RKNN_MODEL) / 1024 / 1024:.2f} MB")
    return True

def convert_model_int8():
    """转换为INT8量化模型（带预处理）"""
    print(f"\n=== Converting to INT8 RKNN with preprocessing ===")
    
    rknn_int8 = os.path.join(PROJECT_ROOT, "model", "rknn", "open_nsfw_rk3566_int8.rknn")
    
    rknn = RKNN(verbose=True)
    
    # 配置 - 尝试设置量化参数
    print("Configuring model for RK3566 (INT8)...")
    rknn.config(
        target_platform='rk3566',
        optimization_level=3,
        quantized_dtype='asymmetric_quantized-8',
        quantized_algorithm='normal'
    )
    
    # 加载ONNX
    print("Loading ONNX model...")
    ret = rknn.load_onnx(
        model=ONNX_MODEL,
        inputs=['input:0'],
        input_size_list=[[1, 224, 224, 3]]
    )
    if ret != 0:
        print(f"Load ONNX failed: {ret}")
        return False
    
    # 创建校准数据集
    dataset = create_dataset_file()
    
    # 构建（量化）
    print("Building RKNN model (INT8)...")
    if dataset:
        ret = rknn.build(do_quantization=True, dataset=dataset)
    else:
        # 没有数据集，使用随机数据
        ret = rknn.build(do_quantization=True)
    
    if ret != 0:
        print(f"Build failed: {ret}")
        return False
    
    # 导出
    print(f"Exporting to {rknn_int8}...")
    ret = rknn.export_rknn(rknn_int8)
    if ret != 0:
        print(f"Export failed: {ret}")
        return False
    
    rknn.release()
    print(f"Success! Size: {os.path.getsize(rknn_int8) / 1024 / 1024:.2f} MB")
    return True

if __name__ == "__main__":
    # 先转换FP16版本
    success = convert_model_fp16()
    
    # 可选：转换INT8版本
    # convert_model_int8()
    
    if success:
        print("\n=== Notes ===")
        print("The RKNN model expects preprocessed input:")
        print("  - Format: BGR (not RGB)")
        print("  - Mean subtraction: [B=104, G=117, R=123]")
        print("  - Data type: float32")
        print("")
        print("However, float32 input causes system freeze on RK3566.")
        print("Current workaround: use uint8 RGB input with reduced accuracy.")
    
    sys.exit(0 if success else 1)
