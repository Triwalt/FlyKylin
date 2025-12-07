#!/usr/bin/env python3
"""
测试不同的fmt和pass_through组合
模拟C++ rknn_api的行为
"""
import numpy as np
from rknnlite.api import RKNNLite

MODEL_PATH = "/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn"

def test_inference(rknn, data, data_format, pass_through):
    """测试推理"""
    try:
        if pass_through:
            out = rknn.inference(inputs=[data], data_format=data_format, inputs_pass_through=[True])
        else:
            out = rknn.inference(inputs=[data], data_format=data_format)
        
        if out is None:
            return "Failed"
        
        raw = out[0].flatten()
        if abs(raw.sum() - 1.0) < 0.1:
            nsfw = raw[1]
        else:
            exp_x = np.exp(raw - np.max(raw))
            probs = exp_x / exp_x.sum()
            nsfw = probs[1]
        
        status = "OK" if nsfw > 0.9 else "WRONG"
        return f"NSFW={nsfw:.4f} [{status}]"
    except Exception as e:
        return f"Error: {e}"

def main():
    print("="*60)
    print("RKNN Format Options Test")
    print("="*60)
    
    # 加载数据
    nwch_data = np.load("/home/kylin/Downloads/porn_nwch.npy")  # [1, 224, 3, 224]
    
    # 转换为NHWC
    nhwc_data = np.zeros((1, 224, 224, 3), dtype=np.float32)
    for h in range(224):
        for w in range(224):
            for c in range(3):
                nhwc_data[0, h, w, c] = nwch_data[0, w, c, h]
    
    print(f"NWCH shape: {nwch_data.shape}")
    print(f"NHWC shape: {nhwc_data.shape}")
    print()
    
    rknn = RKNNLite(verbose=False)
    ret = rknn.load_rknn(MODEL_PATH)
    if ret != 0:
        print(f"Load RKNN failed: {ret}")
        return
    
    ret = rknn.init_runtime()
    if ret != 0:
        print(f"Init runtime failed: {ret}")
        return
    
    print("Testing all combinations...")
    print()
    
    # 测试所有组合
    data_options = [
        ("NWCH [1,224,3,224]", nwch_data),
        ("NHWC [1,224,224,3]", nhwc_data),
    ]
    
    fmt_options = ['nhwc', 'nchw']
    pt_options = [False, True]
    
    print(f"{'Data':<20} {'Format':<8} {'PassThru':<10} {'Result':<30}")
    print("-" * 70)
    
    for data_name, data in data_options:
        for fmt in fmt_options:
            for pt in pt_options:
                result = test_inference(rknn, data, fmt, pt)
                print(f"{data_name:<20} {fmt:<8} {str(pt):<10} {result:<30}")
    
    rknn.release()
    print()
    print("Expected: NSFW > 0.9 for porn.jpg")

if __name__ == "__main__":
    main()
