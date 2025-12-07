#!/usr/bin/env python3
"""
将ONNX模型转换为NCHW格式，然后转换为RKNN
这样可以正确使用mean_values
"""
import os
import sys
import numpy as np

try:
    import onnx
    from onnx import helper, TensorProto, numpy_helper
except ImportError:
    print("Error: onnx not installed")
    sys.exit(1)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw.onnx")
ONNX_MODEL_NCHW = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw_nchw.onnx")

def convert_to_nchw():
    """在模型前添加Transpose层，将NHWC输入转换为NCHW"""
    print(f"Loading: {ONNX_MODEL}")
    model = onnx.load(ONNX_MODEL)
    graph = model.graph
    
    orig_input = graph.input[0]
    print(f"Original input: {orig_input.name}")
    
    # 创建新的NCHW输入 (BGR, 已减均值)
    # 输入shape: [1, 3, 224, 224]
    new_input = helper.make_tensor_value_info(
        'input_nchw',
        TensorProto.FLOAT,
        [1, 3, 224, 224]  # NCHW
    )
    
    # Transpose: NCHW -> NHWC
    # perm = [0, 2, 3, 1] 将 [N,C,H,W] -> [N,H,W,C]
    transpose_node = helper.make_node(
        'Transpose',
        inputs=['input_nchw'],
        outputs=[orig_input.name],
        perm=[0, 2, 3, 1]
    )
    
    # 插入节点
    graph.node.insert(0, transpose_node)
    
    # 替换输入
    del graph.input[0]
    graph.input.insert(0, new_input)
    
    # 保存
    print(f"Saving: {ONNX_MODEL_NCHW}")
    onnx.save(model, ONNX_MODEL_NCHW)
    
    try:
        onnx.checker.check_model(model)
        print("Model validation passed!")
    except Exception as e:
        print(f"Validation warning: {e}")
    
    return True

def test_nchw_model():
    """测试NCHW模型"""
    print("\n=== Testing NCHW model ===")
    import onnxruntime as ort
    
    # 加载测试图片
    img = np.load(os.path.join(SCRIPT_DIR, "porn_224.npy"))
    print(f"Image: {img.shape}")
    
    # 转换为BGR并减均值
    bgr = img[:, :, ::-1].astype(np.float32)
    bgr[:, :, 0] -= 104.0
    bgr[:, :, 1] -= 117.0
    bgr[:, :, 2] -= 123.0
    
    # 转换为NCHW: [H,W,C] -> [C,H,W] -> [1,C,H,W]
    nchw = np.transpose(bgr, (2, 0, 1))
    nchw = np.expand_dims(nchw, axis=0)
    print(f"Input NCHW: {nchw.shape}")
    
    sess = ort.InferenceSession(ONNX_MODEL_NCHW)
    out = sess.run(None, {'input_nchw': nchw})
    print(f"Output: SFW={out[0][0][0]:.4f}, NSFW={out[0][0][1]:.4f}")

if __name__ == "__main__":
    if convert_to_nchw():
        test_nchw_model()
