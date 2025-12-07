#!/usr/bin/env python3
"""
从ONNX模型中移除Softmax层
"""
import os
import sys
import numpy as np

try:
    import onnx
    from onnx import helper
except ImportError:
    print("Error: onnx not installed")
    sys.exit(1)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw_nchw.onnx")
ONNX_MODEL_NO_SOFTMAX = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw_nchw_no_softmax.onnx")

def remove_softmax():
    print(f"Loading: {ONNX_MODEL}")
    model = onnx.load(ONNX_MODEL)
    graph = model.graph
    
    # 找到Softmax节点
    softmax_node = None
    softmax_idx = -1
    for i, node in enumerate(graph.node):
        if node.op_type == 'Softmax':
            softmax_node = node
            softmax_idx = i
            break
    
    if softmax_node is None:
        print("No Softmax node found")
        return False
    
    print(f"Found Softmax node at index {softmax_idx}")
    print(f"  inputs: {list(softmax_node.input)}")
    print(f"  outputs: {list(softmax_node.output)}")
    
    # 获取Softmax的输入（BiasAdd的输出）
    softmax_input = softmax_node.input[0]
    softmax_output = softmax_node.output[0]
    
    # 移除Softmax节点
    del graph.node[softmax_idx]
    
    # 修改输出
    for i, output in enumerate(graph.output):
        if output.name == softmax_output:
            # 创建新的输出，使用Softmax的输入
            new_output = helper.make_tensor_value_info(
                softmax_input,
                onnx.TensorProto.FLOAT,
                [1, 2]
            )
            del graph.output[i]
            graph.output.insert(i, new_output)
            break
    
    # 保存
    print(f"Saving: {ONNX_MODEL_NO_SOFTMAX}")
    onnx.save(model, ONNX_MODEL_NO_SOFTMAX)
    
    return True

def test_model():
    print("\n=== Testing model without Softmax ===")
    import onnxruntime as ort
    
    img = np.load(os.path.join(SCRIPT_DIR, "porn_224.npy"))
    
    # BGR + 均值减法
    bgr = img[:, :, ::-1].astype(np.float32)
    bgr[:, :, 0] -= 104.0
    bgr[:, :, 1] -= 117.0
    bgr[:, :, 2] -= 123.0
    
    # HWC -> CHW -> NCHW
    nchw = np.transpose(bgr, (2, 0, 1))
    nchw = np.expand_dims(nchw, axis=0)
    
    sess = ort.InferenceSession(ONNX_MODEL_NO_SOFTMAX)
    out = sess.run(None, {'input_nchw': nchw})
    raw = out[0].flatten()
    print(f"Raw output (logits): {raw}")
    
    # 手动softmax
    exp_x = np.exp(raw - np.max(raw))
    probs = exp_x / exp_x.sum()
    print(f"After softmax: SFW={probs[0]:.4f}, NSFW={probs[1]:.4f}")

if __name__ == "__main__":
    if remove_softmax():
        test_model()
