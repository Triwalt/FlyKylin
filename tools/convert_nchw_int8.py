#!/usr/bin/env python3
"""
将NCHW格式的ONNX模型转换为INT8量化的RKNN
"""
import os
import sys
import numpy as np

try:
    from rknn.api import RKNN
    from PIL import Image
except ImportError as e:
    print(f"Error: {e}")
    sys.exit(1)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw_nchw.onnx")
RKNN_MODEL = os.path.join(PROJECT_ROOT, "model", "rknn", "open_nsfw_rk3566_int8.rknn")

def create_calibration_data():
    """创建校准数据"""
    # 使用测试图片作为校准数据
    test_images = [
        "/mnt/e/Download/porn.jpg",
        "/mnt/e/Pictures/pig.jpg",
    ]
    
    dataset_file = os.path.join(SCRIPT_DIR, "calibration_data.txt")
    npy_dir = os.path.join(SCRIPT_DIR, "calibration_npy")
    os.makedirs(npy_dir, exist_ok=True)
    
    with open(dataset_file, 'w') as f:
        for i, img_path in enumerate(test_images):
            if not os.path.exists(img_path):
                continue
            
            # 加载并预处理图片
            img = Image.open(img_path).convert('RGB')
            img = img.resize((224, 224), Image.BILINEAR)
            img_np = np.array(img, dtype=np.float32)
            
            # RGB -> BGR + 均值减法
            bgr = img_np[:, :, ::-1]
            bgr[:, :, 0] -= 104.0
            bgr[:, :, 1] -= 117.0
            bgr[:, :, 2] -= 123.0
            
            # HWC -> CHW
            nchw = np.transpose(bgr, (2, 0, 1))
            nchw = np.expand_dims(nchw, axis=0)
            
            # 保存为npy
            npy_path = os.path.join(npy_dir, f"calib_{i}.npy")
            np.save(npy_path, nchw)
            f.write(npy_path + '\n')
            print(f"Created calibration data: {npy_path}")
    
    return dataset_file

def convert():
    print(f"Input: {ONNX_MODEL}")
    print(f"Output: {RKNN_MODEL}")
    
    # 创建校准数据
    dataset_file = create_calibration_data()
    
    rknn = RKNN(verbose=True)
    
    print("Configuring for RK3566 (INT8)...")
    rknn.config(
        target_platform='rk3566',
        optimization_level=3,
        quantized_dtype='asymmetric_quantized-8',
        quantized_algorithm='normal'
    )
    
    print("Loading ONNX model...")
    ret = rknn.load_onnx(
        model=ONNX_MODEL,
        inputs=['input_nchw'],
        input_size_list=[[1, 3, 224, 224]]
    )
    if ret != 0:
        print(f"Load ONNX failed: {ret}")
        return False
    
    print("Building RKNN model (INT8)...")
    ret = rknn.build(do_quantization=True, dataset=dataset_file)
    if ret != 0:
        print(f"Build failed: {ret}")
        return False
    
    print(f"Exporting...")
    ret = rknn.export_rknn(RKNN_MODEL)
    if ret != 0:
        print(f"Export failed: {ret}")
        return False
    
    rknn.release()
    print(f"Success! Size: {os.path.getsize(RKNN_MODEL) / 1024 / 1024:.2f} MB")
    return True

if __name__ == "__main__":
    sys.exit(0 if convert() else 1)
