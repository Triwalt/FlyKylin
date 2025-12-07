#!/usr/bin/env python3
"""
在ONNX模型前添加预处理层 - 使用Gather替代Split/Concat

原始open_nsfw模型期望:
- 输入: float32 BGR, 已减去均值 [B=104, G=117, R=123]

修改后的模型:
- 输入: float32 RGB [0-255]
- 内部自动转换为BGR并减去均值
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
ONNX_MODEL_PREPROC = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw_preproc_v2.onnx")

def add_preprocessing():
    print(f"Loading original model: {ONNX_MODEL}")
    model = onnx.load(ONNX_MODEL)
    graph = model.graph
    
    orig_input = graph.input[0]
    print(f"Original input: {orig_input.name}")
    
    # 创建新的float32输入
    new_input = helper.make_tensor_value_info(
        'input_rgb',
        TensorProto.FLOAT,
        [1, 224, 224, 3]
    )
    
    # 方法: 使用Gather在最后一个轴上重排通道
    # indices = [2, 1, 0] 将 RGB -> BGR
    indices = numpy_helper.from_array(
        np.array([2, 1, 0], dtype=np.int64),
        name='bgr_indices'
    )
    
    # BGR均值
    mean_values = numpy_helper.from_array(
        np.array([104.0, 117.0, 123.0], dtype=np.float32).reshape(1, 1, 1, 3),
        name='mean_values'
    )
    
    # 节点1: Gather - RGB -> BGR
    # Gather(input_rgb, indices, axis=3)
    gather_node = helper.make_node(
        'Gather',
        inputs=['input_rgb', 'bgr_indices'],
        outputs=['input_bgr'],
        axis=3
    )
    
    # 节点2: Sub - 减去均值
    sub_node = helper.make_node(
        'Sub',
        inputs=['input_bgr', 'mean_values'],
        outputs=[orig_input.name]
    )
    
    # 添加常量
    graph.initializer.extend([indices, mean_values])
    
    # 插入节点
    graph.node.insert(0, gather_node)
    graph.node.insert(1, sub_node)
    
    # 替换输入
    del graph.input[0]
    graph.input.insert(0, new_input)
    
    # 保存
    print(f"Saving: {ONNX_MODEL_PREPROC}")
    onnx.save(model, ONNX_MODEL_PREPROC)
    
    try:
        onnx.checker.check_model(model)
        print("Model validation passed!")
    except Exception as e:
        print(f"Validation warning: {e}")
    
    return True

def test_model():
    print("\n=== Testing ===")
    import onnxruntime as ort
    
    img = np.load(os.path.join(SCRIPT_DIR, "porn_224.npy"))
    img_f32 = img.astype(np.float32)
    img_4d = np.expand_dims(img_f32, axis=0)
    
    sess = ort.InferenceSession(ONNX_MODEL_PREPROC)
    out = sess.run(None, {'input_rgb': img_4d})
    print(f"Output: SFW={out[0][0][0]:.4f}, NSFW={out[0][0][1]:.4f}")

if __name__ == "__main__":
    if add_preprocessing():
        test_model()
