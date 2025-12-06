#!/usr/bin/env python3
"""
干净的RKNN模型转换 - 不添加任何预处理
"""
import os
import sys

try:
    from rknn.api import RKNN
except ImportError:
    print("Error: rknn-toolkit2 not installed")
    sys.exit(1)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw.onnx")
RKNN_MODEL = os.path.join(PROJECT_ROOT, "model", "rknn", "open_nsfw_clean.rknn")

def convert():
    print(f"Input: {ONNX_MODEL}")
    print(f"Output: {RKNN_MODEL}")
    
    if not os.path.exists(ONNX_MODEL):
        print(f"Error: ONNX model not found")
        return False
    
    rknn = RKNN(verbose=True)
    
    # 最简单的配置
    print("\nConfiguring for RK3566...")
    rknn.config(
        target_platform='rk3566',
        optimization_level=3
    )
    
    # 加载ONNX - 指定输入
    print("\nLoading ONNX model...")
    ret = rknn.load_onnx(
        model=ONNX_MODEL,
        inputs=['input:0'],
        input_size_list=[[1, 224, 224, 3]]
    )
    if ret != 0:
        print(f"Load failed: {ret}")
        return False
    
    # 构建 - 不量化
    print("\nBuilding RKNN model...")
    ret = rknn.build(do_quantization=False)
    if ret != 0:
        print(f"Build failed: {ret}")
        return False
    
    # 导出
    print(f"\nExporting to {RKNN_MODEL}...")
    ret = rknn.export_rknn(RKNN_MODEL)
    if ret != 0:
        print(f"Export failed: {ret}")
        return False
    
    print(f"\nSuccess! Size: {os.path.getsize(RKNN_MODEL) / 1024 / 1024:.2f} MB")
    
    # 在模拟器上测试
    print("\n=== Testing on simulator ===")
    ret = rknn.init_runtime(target=None)
    if ret == 0:
        import numpy as np
        from PIL import Image
        
        # 测试图片
        test_path = "/mnt/e/Download/porn.jpg"
        if os.path.exists(test_path):
            img = Image.open(test_path).convert('RGB')
            img = img.resize((224, 224), Image.BILINEAR)
            img_np = np.array(img, dtype=np.float32)
            # RGB -> BGR, mean subtraction
            img_np = img_np[:, :, ::-1].copy()
            img_np[:, :, 0] -= 104.0
            img_np[:, :, 1] -= 117.0
            img_np[:, :, 2] -= 123.0
            img_np = np.expand_dims(img_np, axis=0)
            
            print(f"Input shape: {img_np.shape}")
            outputs = rknn.inference(inputs=[img_np], data_format='nhwc')
            if outputs:
                out = outputs[0].flatten()
                print(f"Output: SFW={out[0]:.6f}, NSFW={out[1]:.6f}")
    
    rknn.release()
    return True

if __name__ == "__main__":
    sys.exit(0 if convert() else 1)
