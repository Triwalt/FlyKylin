#!/usr/bin/env python3
"""
将 open_nsfw.onnx 模型转换为 RK3566 兼容的 RKNN 模型
版本2：尝试正确设置预处理参数

open_nsfw模型预处理要求：
1. 输入格式: NHWC [1, 224, 224, 3]
2. 颜色空间: BGR
3. 均值减法: [B=104, G=117, R=123]
4. 无标准化 (std=1)
"""

import os
import sys
import numpy as np

try:
    from rknn.api import RKNN
except ImportError:
    print("Error: rknn-toolkit2 not installed")
    sys.exit(1)

from PIL import Image

# 路径配置
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw.onnx")
RKNN_MODEL_V2 = os.path.join(PROJECT_ROOT, "model", "rknn", "open_nsfw_rk3566_v2.rknn")
TEST_IMAGE = "/mnt/e/Download/porn.jpg"

def preprocess_for_onnx(image_path):
    """ONNX模型的预处理方式（作为参考）"""
    img = Image.open(image_path).convert('RGB')
    img = img.resize((256, 256), Image.BILINEAR)
    # 中心裁剪
    left = (256 - 224) // 2
    top = (256 - 224) // 2
    img = img.crop((left, top, left + 224, top + 224))
    img_np = np.array(img, dtype=np.float32)
    # RGB -> BGR
    img_np = img_np[:, :, ::-1]
    # 减去均值
    img_np[:, :, 0] -= 104.0  # B
    img_np[:, :, 1] -= 117.0  # G
    img_np[:, :, 2] -= 123.0  # R
    return img_np

def preprocess_simple(image_path):
    """简单预处理：直接缩放到224x224，RGB格式"""
    img = Image.open(image_path).convert('RGB')
    img = img.resize((224, 224), Image.BILINEAR)
    return np.array(img, dtype=np.uint8)

def test_onnx_reference():
    """测试ONNX模型作为参考"""
    print("=== ONNX Reference Test ===")
    try:
        import onnxruntime as ort
    except ImportError:
        print("onnxruntime not installed, skipping")
        return None
    
    if not os.path.exists(TEST_IMAGE):
        print(f"Test image not found: {TEST_IMAGE}")
        return None
    
    img = preprocess_for_onnx(TEST_IMAGE)
    img = np.expand_dims(img, axis=0)  # NHWC
    
    session = ort.InferenceSession(ONNX_MODEL)
    input_name = session.get_inputs()[0].name
    outputs = session.run(None, {input_name: img})
    
    out = outputs[0].flatten()
    print(f"ONNX Output: SFW={out[0]:.6f}, NSFW={out[1]:.6f}")
    return out[1]

def convert_and_test():
    """转换模型并测试"""
    print(f"\n=== Converting ONNX to RKNN ===")
    print(f"Input: {ONNX_MODEL}")
    print(f"Output: {RKNN_MODEL_V2}")
    
    if not os.path.exists(ONNX_MODEL):
        print(f"Error: ONNX model not found")
        return False
    
    # 加载测试图片
    if os.path.exists(TEST_IMAGE):
        img_simple = preprocess_simple(TEST_IMAGE)
        img_onnx = preprocess_for_onnx(TEST_IMAGE)
        print(f"Test image loaded: {img_simple.shape}")
    else:
        img_simple = None
        img_onnx = None
        print("No test image, skipping inference test")
    
    # 方案1: 不设置mean_values，在C++代码中手动处理
    print("\n--- Method 1: No preprocessing in RKNN (manual in C++) ---")
    rknn1 = RKNN(verbose=False)
    rknn1.config(target_platform='rk3566', optimization_level=3)
    ret = rknn1.load_onnx(model=ONNX_MODEL, inputs=['input:0'], input_size_list=[[1, 224, 224, 3]])
    if ret != 0:
        print(f"Failed to load ONNX: {ret}")
        return False
    
    ret = rknn1.build(do_quantization=False)
    if ret != 0:
        print(f"Failed to build: {ret}")
        return False
    
    # 在模拟器上测试
    ret = rknn1.init_runtime(target=None)
    if ret == 0 and img_onnx is not None:
        # 使用ONNX预处理后的数据测试
        # 需要转换为RKNN期望的格式 (1, 224, 3, 224)
        img_input = np.transpose(img_onnx, (0, 2, 1))  # (H, W, C) -> (H, C, W)
        img_input = np.expand_dims(img_input, axis=0)
        outputs = rknn1.inference(inputs=[img_input], data_format='nhwc')
        if outputs:
            out = outputs[0].flatten()
            print(f"Method 1 (float32 with mean sub): SFW={out[0]:.6f}, NSFW={out[1]:.6f}")
    
    # 导出模型
    ret = rknn1.export_rknn(RKNN_MODEL_V2)
    if ret != 0:
        print(f"Failed to export: {ret}")
        rknn1.release()
        return False
    
    print(f"\nModel exported to: {RKNN_MODEL_V2}")
    print(f"Size: {os.path.getsize(RKNN_MODEL_V2) / 1024 / 1024:.2f} MB")
    
    rknn1.release()
    return True

if __name__ == "__main__":
    # 先测试ONNX作为参考
    onnx_nsfw = test_onnx_reference()
    
    # 转换并测试
    success = convert_and_test()
    
    if success:
        print("\n=== Summary ===")
        print("Model converted successfully.")
        print("The RKNN model expects float32 BGR input with mean subtraction.")
        print("C++ code needs to:")
        print("  1. Convert RGB to BGR")
        print("  2. Subtract mean values [B=104, G=117, R=123]")
        print("  3. Use float32 input type")
    
    sys.exit(0 if success else 1)
