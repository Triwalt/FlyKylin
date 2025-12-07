#!/usr/bin/env python3
"""
端到端测试：模拟C++代码的完整流程
"""
import numpy as np
from rknnlite.api import RKNNLite

def cpp_preprocess(img):
    """模拟C++代码的预处理"""
    height, width = img.shape[:2]
    
    kMeanB = 104.0
    kMeanG = 117.0
    kMeanR = 123.0
    
    # 输出数组 [W * C * H] 线性存储
    output = np.zeros(width * 3 * height, dtype=np.float32)
    
    for h in range(height):
        for w in range(width):
            r = float(img[h, w, 0])
            g = float(img[h, w, 1])
            b = float(img[h, w, 2])
            
            bVal = b - kMeanB
            gVal = g - kMeanG
            rVal = r - kMeanR
            
            # C++代码: baseIdx = w * 3 * height
            # index = baseIdx + c * height + h
            baseIdx = w * 3 * height
            output[baseIdx + 0 * height + h] = bVal
            output[baseIdx + 1 * height + h] = gVal
            output[baseIdx + 2 * height + h] = rVal
    
    # reshape为 [1, W, C, H] = [1, 224, 3, 224]
    return output.reshape(1, width, 3, height)

print("Loading RKNN model...")
rknn = RKNNLite(verbose=False)
rknn.load_rknn("/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn")
rknn.init_runtime(core_mask=0)

def softmax(x):
    e = np.exp(x - np.max(x))
    return e / e.sum()

def test(name, img_path, expected_nsfw):
    img = np.load(img_path)
    print(f"\n=== {name} ===")
    print(f"Input: {img.shape}")
    
    # C++风格预处理
    preprocessed = cpp_preprocess(img)
    print(f"Preprocessed: {preprocessed.shape}")
    
    # 推理
    out = rknn.inference(inputs=[preprocessed])
    raw = out[0].flatten()
    print(f"Raw output: {raw}")
    
    # 检查是否已经是概率
    if abs(raw.sum() - 1.0) < 0.1:
        sfw, nsfw = raw[0], raw[1]
    else:
        probs = softmax(raw)
        sfw, nsfw = probs[0], probs[1]
    
    status = "OK" if abs(nsfw - expected_nsfw) < 0.15 else "WRONG"
    print(f"SFW={sfw:.4f}, NSFW={nsfw:.4f}")
    print(f"Expected NSFW ~{expected_nsfw} [{status}]")

test("porn", "/home/kylin/Downloads/porn_224.npy", 0.96)
test("pig", "/home/kylin/Downloads/pig_224.npy", 0.0)

rknn.release()
print("\n=== Test Complete ===")
