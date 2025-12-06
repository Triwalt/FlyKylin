#!/usr/bin/env python3
"""
在PC端测试RKNN模型准确性（使用rknn-toolkit2模拟器）
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
RKNN_MODEL = os.path.join(PROJECT_ROOT, "model", "rknn", "open_nsfw_rk3566.rknn")
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw.onnx")
TEST_IMAGE = "/mnt/e/Download/porn.jpg"

def preprocess_image(image_path, with_mean_sub=False, to_bgr=False):
    """预处理图片"""
    img = Image.open(image_path)
    img = img.convert('RGB')
    img = img.resize((224, 224), Image.BILINEAR)
    img_np = np.array(img, dtype=np.uint8)
    
    if to_bgr:
        img_np = img_np[:, :, ::-1]  # RGB -> BGR
    
    if with_mean_sub:
        img_np = img_np.astype(np.float32)
        # BGR均值: [B=104, G=117, R=123]
        if to_bgr:
            img_np[:, :, 0] -= 104.0  # B
            img_np[:, :, 1] -= 117.0  # G
            img_np[:, :, 2] -= 123.0  # R
        else:
            # RGB顺序
            img_np[:, :, 0] -= 123.0  # R
            img_np[:, :, 1] -= 117.0  # G
            img_np[:, :, 2] -= 104.0  # B
    
    return img_np

def test_rknn_model():
    """从ONNX构建并在模拟器上测试RKNN模型"""
    print("=== RKNN Model Test (PC Simulator) ===")
    print(f"ONNX Model: {ONNX_MODEL}")
    print(f"Test Image: {TEST_IMAGE}")
    print()
    
    if not os.path.exists(ONNX_MODEL):
        print(f"Error: ONNX model not found: {ONNX_MODEL}")
        return False
    
    if not os.path.exists(TEST_IMAGE):
        print(f"Error: Test image not found: {TEST_IMAGE}")
        return False
    
    # 加载图片 - 使用BGR格式和均值减法（与ONNX一致）
    print("Loading image...")
    img_bgr_mean = preprocess_image(TEST_IMAGE, with_mean_sub=True, to_bgr=True)
    img_rgb = preprocess_image(TEST_IMAGE, with_mean_sub=False, to_bgr=False)
    print(f"Image shape: {img_bgr_mean.shape}, dtype: {img_bgr_mean.dtype}")
    
    # 方案1: 不设置mean_values，使用float32 BGR输入（手动减均值）
    print("\n--- Test 1: float32 BGR input (manual mean sub, like ONNX) ---")
    rknn = RKNN(verbose=False)
    rknn.config(target_platform='rk3566', optimization_level=3)
    ret = rknn.load_onnx(model=ONNX_MODEL, inputs=['input:0'], input_size_list=[[1, 224, 224, 3]])
    if ret != 0:
        print(f"Error: Failed to load ONNX, ret={ret}")
        return False
    ret = rknn.build(do_quantization=False)
    if ret != 0:
        print(f"Error: Failed to build, ret={ret}")
        return False
    ret = rknn.init_runtime(target=None)
    if ret != 0:
        print(f"Error: Failed to init runtime, ret={ret}")
        rknn.release()
        return False
    
    # 添加batch维度，并转换为RKNN期望的格式 (1, 224, 3, 224)
    # 原始: (224, 224, 3) -> 目标: (1, 224, 3, 224)
    # 这意味着 (H, W, C) -> (N, H, C, W)
    img_input = np.transpose(img_bgr_mean, (0, 2, 1))  # (H, W, C) -> (H, C, W)
    img_input = np.expand_dims(img_input, axis=0)  # (1, H, C, W)
    print(f"Input shape for inference: {img_input.shape}")
    outputs = rknn.inference(inputs=[img_input], data_format='nhwc')
    if outputs:
        out = outputs[0].flatten()
        print(f"Output: {out}")
        if len(out) >= 2:
            print(f"SFW: {out[0]:.6f}, NSFW: {out[1]:.6f}")
    rknn.release()
    
    # 方案2: 设置mean_values，使用uint8 BGR输入
    print("\n--- Test 2: uint8 BGR input with mean_values in config ---")
    rknn2 = RKNN(verbose=False)
    # open_nsfw期望BGR格式，均值 [B=104, G=117, R=123]
    rknn2.config(
        target_platform='rk3566',
        optimization_level=3,
        mean_values=[[104, 117, 123]],  # BGR顺序
        std_values=[[1, 1, 1]]
    )
    ret = rknn2.load_onnx(model=ONNX_MODEL, inputs=['input:0'], input_size_list=[[1, 224, 224, 3]])
    if ret != 0:
        print(f"Error: Failed to load ONNX, ret={ret}")
        return False
    ret = rknn2.build(do_quantization=False)
    if ret != 0:
        print(f"Error: Failed to build, ret={ret}")
        return False
    ret = rknn2.init_runtime(target=None)
    if ret != 0:
        print(f"Error: Failed to init runtime, ret={ret}")
        rknn2.release()
        return False
    
    # 使用BGR uint8输入，转换为RKNN期望的格式 (1, 224, 3, 224)
    img_bgr_uint8 = preprocess_image(TEST_IMAGE, with_mean_sub=False, to_bgr=True)
    img_input2 = np.transpose(img_bgr_uint8, (0, 2, 1))  # (H, W, C) -> (H, C, W)
    img_input2 = np.expand_dims(img_input2, axis=0)  # (1, H, C, W)
    print(f"Input shape for inference: {img_input2.shape}, dtype: {img_input2.dtype}")
    outputs = rknn2.inference(inputs=[img_input2], data_format='nhwc')
    if outputs:
        out = outputs[0].flatten()
        print(f"Output: {out}")
        if len(out) >= 2:
            print(f"SFW: {out[0]:.6f}, NSFW: {out[1]:.6f}")
    rknn2.release()
    
    return True

def test_onnx_model():
    """对比测试ONNX模型"""
    print("\n=== ONNX Model Test (Reference) ===")
    
    try:
        import onnxruntime as ort
    except ImportError:
        print("onnxruntime not installed, skipping ONNX test")
        return
    
    if not os.path.exists(ONNX_MODEL):
        print(f"ONNX model not found: {ONNX_MODEL}")
        return
    
    print(f"ONNX Model: {ONNX_MODEL}")
    
    # 加载图片并预处理（与ONNX版本一致）
    img = Image.open(TEST_IMAGE).convert('RGB')
    img = img.resize((256, 256), Image.BILINEAR)
    # 中心裁剪
    left = (256 - 224) // 2
    top = (256 - 224) // 2
    img = img.crop((left, top, left + 224, top + 224))
    img_np = np.array(img, dtype=np.float32)
    
    # BGR格式，减去均值
    img_np = img_np[:, :, ::-1]  # RGB -> BGR
    img_np[:, :, 0] -= 104.0
    img_np[:, :, 1] -= 117.0
    img_np[:, :, 2] -= 123.0
    
    # NHWC格式
    img_np = np.expand_dims(img_np, axis=0)
    print(f"Input shape: {img_np.shape}")
    
    # 运行推理
    session = ort.InferenceSession(ONNX_MODEL)
    input_name = session.get_inputs()[0].name
    outputs = session.run(None, {input_name: img_np})
    
    out = outputs[0].flatten()
    print(f"Output: {out}")
    if len(out) >= 2:
        print(f"SFW: {out[0]:.6f}, NSFW: {out[1]:.6f}")

if __name__ == "__main__":
    test_onnx_model()  # 先测试ONNX作为参考
    print("\n" + "="*50 + "\n")
    test_rknn_model()
