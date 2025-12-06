#!/usr/bin/env python3
"""
ONNX基准测试 - 建立正确的预期结果
"""
import numpy as np
from PIL import Image
import os
import sys

try:
    import onnxruntime as ort
except ImportError:
    print("Error: onnxruntime not installed")
    sys.exit(1)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw.onnx")

# 测试图片
TEST_IMAGES = [
    "/mnt/e/Download/porn.jpg",
    "/mnt/e/Pictures/pig.jpg"
]

def preprocess_onnx(path):
    """ONNX模型的标准预处理"""
    img = Image.open(path).convert('RGB')
    # 缩放到256x256
    img = img.resize((256, 256), Image.BILINEAR)
    # 中心裁剪224x224
    left = (256 - 224) // 2
    top = (256 - 224) // 2
    img = img.crop((left, top, left + 224, top + 224))
    img_np = np.array(img, dtype=np.float32)
    # RGB -> BGR
    img_np = img_np[:, :, ::-1].copy()
    # Mean subtraction [B=104, G=117, R=123]
    img_np[:, :, 0] -= 104.0  # B
    img_np[:, :, 1] -= 117.0  # G
    img_np[:, :, 2] -= 123.0  # R
    return np.expand_dims(img_np, axis=0)

def main():
    print("=== ONNX Baseline Test ===")
    print(f"Model: {ONNX_MODEL}")
    print()
    
    if not os.path.exists(ONNX_MODEL):
        print(f"Error: Model not found: {ONNX_MODEL}")
        return
    
    session = ort.InferenceSession(ONNX_MODEL)
    input_name = session.get_inputs()[0].name
    print(f"Input name: {input_name}")
    print()
    
    for path in TEST_IMAGES:
        if not os.path.exists(path):
            print(f"{path}: File not found")
            continue
        
        try:
            img = preprocess_onnx(path)
            print(f"Input shape: {img.shape}, dtype: {img.dtype}")
            outputs = session.run(None, {input_name: img})
            out = outputs[0].flatten()
            print(f"{os.path.basename(path)}:")
            print(f"  SFW={out[0]:.6f}, NSFW={out[1]:.6f}")
            print()
        except Exception as e:
            print(f"{path}: Error - {e}")

if __name__ == "__main__":
    main()
