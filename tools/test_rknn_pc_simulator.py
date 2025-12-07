#!/usr/bin/env python3
"""
PC端RKNN模拟器测试 - 验证模型准确性
使用rknn-toolkit2的模拟器功能，无需板端即可验证
"""
import os
import sys
import numpy as np

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)

try:
    from rknn.api import RKNN
except ImportError:
    print("Error: rknn-toolkit2 not installed")
    print("Install with: pip install rknn-toolkit2")
    sys.exit(1)

try:
    from PIL import Image
except ImportError:
    print("Error: PIL not installed")
    sys.exit(1)

# 模型路径
RKNN_MODEL = os.path.join(PROJECT_ROOT, "model", "rknn", "open_nsfw_rk3566.rknn")
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw.onnx")

# 测试图片
TEST_IMAGES = [
    ("/mnt/e/Download/porn.jpg", 0.96, "NSFW"),
    ("/mnt/e/Pictures/pig.jpg", 0.0, "SFW"),
]

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
    
    # 输出数组 [W * C * H] 线性存储
    output = np.zeros(width * 3 * height, dtype=np.float32)
    
    for h in range(height):
        for w in range(width):
            r = float(img_np[h, w, 0])
            g = float(img_np[h, w, 1])
            b = float(img_np[h, w, 2])
            
            bVal = b - kMeanB
            gVal = g - kMeanG
            rVal = r - kMeanR
            
            baseIdx = w * 3 * height
            output[baseIdx + 0 * height + h] = bVal
            output[baseIdx + 1 * height + h] = gVal
            output[baseIdx + 2 * height + h] = rVal
    
    return output.reshape(1, width, 3, height)

def test_onnx_reference():
    """ONNX参考测试"""
    print("\n" + "="*60)
    print("ONNX Reference Test (PC)")
    print("="*60)
    
    import onnxruntime as ort
    
    sess = ort.InferenceSession(ONNX_MODEL)
    
    for img_path, expected, label in TEST_IMAGES:
        if not os.path.exists(img_path):
            print(f"Skip: {img_path} not found")
            continue
        
        img = Image.open(img_path).convert('RGB')
        img = img.resize((224, 224), Image.BILINEAR)
        img_np = np.array(img, dtype=np.float32)
        
        # BGR + 均值减法 (ONNX原始格式)
        bgr = img_np[:, :, ::-1]
        bgr[:, :, 0] -= 104.0
        bgr[:, :, 1] -= 117.0
        bgr[:, :, 2] -= 123.0
        
        inp = np.expand_dims(bgr, axis=0)  # NHWC
        out = sess.run(None, {'input:0': inp})
        
        sfw, nsfw = out[0][0]
        status = "OK" if abs(nsfw - expected) < 0.15 else "WRONG"
        print(f"  {label}: SFW={sfw:.4f}, NSFW={nsfw:.4f} (expected ~{expected}) [{status}]")

def test_rknn_simulator():
    """RKNN模拟器测试"""
    print("\n" + "="*60)
    print("RKNN Simulator Test (PC)")
    print("="*60)
    
    if not os.path.exists(RKNN_MODEL):
        print(f"RKNN model not found: {RKNN_MODEL}")
        print("Please run model conversion first")
        return False
    
    rknn = RKNN(verbose=False)
    
    # 方法1: 直接加载RKNN文件（不支持模拟器）
    # 方法2: 重新从ONNX构建并使用模拟器
    print("Loading ONNX and building for simulator...")
    
    rknn.config(target_platform='rk3566')
    # 必须指定inputs和input_size_list，否则会报错
    ret = rknn.load_onnx(
        model=ONNX_MODEL,
        inputs=['input:0'],
        input_size_list=[[1, 224, 224, 3]]  # NHWC格式
    )
    if ret != 0:
        print(f"Load ONNX failed: {ret}")
        return False
    
    ret = rknn.build(do_quantization=False)
    if ret != 0:
        print(f"Build failed: {ret}")
        return False
    
    # 初始化模拟器
    ret = rknn.init_runtime(target=None)  # target=None 使用模拟器
    if ret != 0:
        print(f"Init runtime failed: {ret}")
        return False
    
    print("Simulator initialized")
    
    for img_path, expected, label in TEST_IMAGES:
        if not os.path.exists(img_path):
            print(f"Skip: {img_path} not found")
            continue
        
        # 使用C++风格预处理
        inp = cpp_style_preprocess(img_path)
        print(f"\n  {label} ({os.path.basename(img_path)}):")
        print(f"    Input shape: {inp.shape}")
        
        try:
            out = rknn.inference(inputs=[inp])
            if out is None:
                print(f"    Inference failed")
                continue
            
            raw = out[0].flatten()
            print(f"    Raw output: {raw}")
            
            # 检查是否已经是概率
            if abs(raw.sum() - 1.0) < 0.1:
                sfw, nsfw = raw[0], raw[1]
            else:
                exp_x = np.exp(raw - np.max(raw))
                probs = exp_x / exp_x.sum()
                sfw, nsfw = probs[0], probs[1]
            
            status = "OK" if abs(nsfw - expected) < 0.15 else "WRONG"
            print(f"    SFW={sfw:.4f}, NSFW={nsfw:.4f} (expected ~{expected}) [{status}]")
            
        except Exception as e:
            print(f"    Error: {e}")
    
    rknn.release()
    return True

def main():
    print("="*60)
    print("RKNN PC Simulator Accuracy Test")
    print("="*60)
    print(f"ONNX Model: {ONNX_MODEL}")
    print(f"RKNN Model: {RKNN_MODEL}")
    
    # 1. ONNX参考测试
    test_onnx_reference()
    
    # 2. RKNN模拟器测试
    test_rknn_simulator()
    
    print("\n" + "="*60)
    print("Test Complete")
    print("="*60)

if __name__ == "__main__":
    main()
