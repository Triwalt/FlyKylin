#!/usr/bin/env python3
"""
RKNN NSFW模型测试脚本
用于在RK3566板端直接测试RKNN推理，排查卡死问题
"""

import os
import sys
import time
import numpy as np

try:
    from rknnlite.api import RKNNLite
except ImportError:
    print("Error: rknnlite not installed")
    print("This script should run on RK3566 board with rknnlite installed")
    sys.exit(1)

try:
    from PIL import Image
except ImportError:
    print("Error: PIL not installed, trying cv2...")
    try:
        import cv2
        USE_CV2 = True
    except ImportError:
        print("Error: Neither PIL nor cv2 available")
        sys.exit(1)
else:
    USE_CV2 = False

MODEL_PATH = "/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn"
TEST_IMAGE = "/home/kylin/Downloads/test.jpg"  # 需要一张测试图片

def load_image(path, size=(224, 224)):
    """加载并预处理图片"""
    if USE_CV2:
        img = cv2.imread(path)
        if img is None:
            return None
        img = cv2.resize(img, size)
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    else:
        img = Image.open(path)
        img = img.resize(size, Image.BILINEAR)
        img = np.array(img)
    
    return img

def test_rknn_inference():
    print(f"=== RKNN NSFW Test ===")
    print(f"Model: {MODEL_PATH}")
    print(f"Test image: {TEST_IMAGE}")
    print()
    
    # 检查文件
    if not os.path.exists(MODEL_PATH):
        print(f"Error: Model not found: {MODEL_PATH}")
        return False
    
    if not os.path.exists(TEST_IMAGE):
        print(f"Warning: Test image not found: {TEST_IMAGE}")
        print("Creating a dummy test image...")
        # 创建一个随机测试图片
        dummy_img = np.random.randint(0, 255, (224, 224, 3), dtype=np.uint8)
        if USE_CV2:
            cv2.imwrite(TEST_IMAGE, dummy_img)
        else:
            Image.fromarray(dummy_img).save(TEST_IMAGE)
    
    # 初始化RKNN
    print("1. Initializing RKNNLite...")
    rknn = RKNNLite(verbose=False)
    
    print("2. Loading RKNN model...")
    ret = rknn.load_rknn(MODEL_PATH)
    if ret != 0:
        print(f"Error: Failed to load model, ret={ret}")
        return False
    print("   Model loaded successfully")
    
    print("3. Initializing runtime...")
    # 尝试不同的core_mask设置
    # core_mask: 0=auto, 1=NPU0, 2=NPU1, 3=NPU0+NPU1
    ret = rknn.init_runtime(core_mask=0)
    if ret != 0:
        print(f"Error: Failed to init runtime, ret={ret}")
        rknn.release()
        return False
    print("   Runtime initialized")
    
    # 查询输入输出信息
    print("4. Querying model info...")
    sdk_version = rknn.get_sdk_version()
    print(f"   SDK version: {sdk_version}")
    
    # 加载测试图片
    print("5. Loading test image...")
    img = load_image(TEST_IMAGE)
    if img is None:
        print("Error: Failed to load image")
        rknn.release()
        return False
    print(f"   Image shape: {img.shape}, dtype: {img.dtype}")
    
    # 运行推理
    print("6. Running inference...")
    print("   (If system freezes here, it's an NPU driver issue)")
    sys.stdout.flush()
    
    start_time = time.time()
    try:
        outputs = rknn.inference(inputs=[img])
        elapsed = time.time() - start_time
        print(f"   Inference completed in {elapsed*1000:.2f}ms")
    except Exception as e:
        print(f"Error during inference: {e}")
        rknn.release()
        return False
    
    # 解析输出
    print("7. Parsing output...")
    if outputs is None or len(outputs) == 0:
        print("Error: No output from inference")
        rknn.release()
        return False
    
    output = outputs[0]
    print(f"   Output shape: {output.shape}, dtype: {output.dtype}")
    print(f"   Output values: {output.flatten()[:10]}")
    
    if output.size >= 2:
        sfw_prob = output.flatten()[0]
        nsfw_prob = output.flatten()[1]
        print(f"   SFW probability: {sfw_prob:.6f}")
        print(f"   NSFW probability: {nsfw_prob:.6f}")
    
    # 多次推理测试稳定性
    print("8. Running stability test (10 iterations)...")
    for i in range(10):
        try:
            outputs = rknn.inference(inputs=[img])
            print(f"   Iteration {i+1}: OK")
        except Exception as e:
            print(f"   Iteration {i+1}: FAILED - {e}")
            break
    
    # 释放资源
    print("9. Releasing resources...")
    rknn.release()
    print("   Done")
    
    return True

if __name__ == "__main__":
    success = test_rknn_inference()
    sys.exit(0 if success else 1)
