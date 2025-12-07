#!/usr/bin/env python3
"""
测试不同的pass_through设置
"""
import numpy as np
from rknnlite.api import RKNNLite

MODEL_PATH = "/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn"
NPY_PATH = "/home/kylin/Downloads/porn_nwch.npy"

def test_inference(rknn, data, pass_through, data_format):
    """测试推理"""
    try:
        if pass_through:
            out = rknn.inference(inputs=[data], data_format=data_format, inputs_pass_through=[True])
        else:
            out = rknn.inference(inputs=[data], data_format=data_format)
        
        if out is None:
            return None, None
        
        raw = out[0].flatten()
        if abs(raw.sum() - 1.0) < 0.1:
            return raw[0], raw[1]
        else:
            exp_x = np.exp(raw - np.max(raw))
            probs = exp_x / exp_x.sum()
            return probs[0], probs[1]
    except Exception as e:
        print(f"    Error: {e}")
        return None, None

def main():
    print("="*50)
    print("RKNN Pass-Through Test")
    print("="*50)
    
    rknn = RKNNLite(verbose=False)
    ret = rknn.load_rknn(MODEL_PATH)
    if ret != 0:
        print(f"Load RKNN failed: {ret}")
        return
    
    ret = rknn.init_runtime()
    if ret != 0:
        print(f"Init runtime failed: {ret}")
        return
    
    data = np.load(NPY_PATH)
    print(f"Input: {NPY_PATH}")
    print(f"Shape: {data.shape}, dtype: {data.dtype}")
    print(f"Expected: NSFW ~0.96")
    print()
    
    # 测试不同组合
    tests = [
        (False, 'nhwc', "No pass_through, nhwc"),
        (True, 'nhwc', "pass_through=True, nhwc"),
        (False, 'nchw', "No pass_through, nchw"),
        (True, 'nchw', "pass_through=True, nchw"),
    ]
    
    for pass_through, data_format, desc in tests:
        print(f"Test: {desc}")
        sfw, nsfw = test_inference(rknn, data, pass_through, data_format)
        if nsfw is not None:
            status = "OK" if nsfw > 0.9 else "WRONG"
            print(f"  SFW={sfw:.4f}, NSFW={nsfw:.4f} [{status}]")
        else:
            print(f"  Failed")
        print()
    
    rknn.release()

if __name__ == "__main__":
    main()
