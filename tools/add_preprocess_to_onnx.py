#!/usr/bin/env python3
"""
在ONNX模型前添加预处理层，使其可以直接接收uint8 RGB输入

原始open_nsfw模型期望:
- 输入: float32 BGR, 已减去均值 [B=104, G=117, R=123]

修改后的模型:
- 输入: uint8 RGB [0-255]
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
    print("Install with: pip install onnx")
    sys.exit(1)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw.onnx")
ONNX_MODEL_PREPROC = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw_preproc.onnx")

def add_preprocessing():
    print(f"Loading original model: {ONNX_MODEL}")
    model = onnx.load(ONNX_MODEL)
    graph = model.graph
    
    # 获取原始输入信息
    orig_input = graph.input[0]
    print(f"Original input: {orig_input.name}, shape: {[d.dim_value for d in orig_input.type.tensor_type.shape.dim]}")
    
    # 创建新的float32输入（RKNN不支持uint8输入）
    new_input = helper.make_tensor_value_info(
        'input_rgb',
        TensorProto.FLOAT,
        [1, 224, 224, 3]  # NHWC, RGB, 0-255 range
    )
    
    # 创建预处理常量
    # RGB到BGR的索引: [2, 1, 0]
    rgb_to_bgr_indices = numpy_helper.from_array(
        np.array([2, 1, 0], dtype=np.int64),
        name='rgb_to_bgr_indices'
    )
    
    # BGR均值: [104, 117, 123]
    mean_values = numpy_helper.from_array(
        np.array([104.0, 117.0, 123.0], dtype=np.float32).reshape(1, 1, 1, 3),
        name='mean_values'
    )
    
    # 创建预处理节点
    # 1. RGB -> BGR (gather along last axis)
    # 由于ONNX的Gather操作复杂，我们用Split和Concat来实现
    # Split: [N,H,W,3] -> [N,H,W,1], [N,H,W,1], [N,H,W,1]
    split_node = helper.make_node(
        'Split',
        inputs=['input_rgb'],
        outputs=['r_channel', 'g_channel', 'b_channel'],
        axis=3
    )
    
    # Concat: BGR order
    concat_node = helper.make_node(
        'Concat',
        inputs=['b_channel', 'g_channel', 'r_channel'],
        outputs=['input_bgr'],
        axis=3
    )
    
    # 3. 减去均值
    sub_node = helper.make_node(
        'Sub',
        inputs=['input_bgr', 'mean_values'],
        outputs=[orig_input.name]  # 连接到原始模型的输入
    )
    
    # 构建新图
    # 添加常量到initializer
    graph.initializer.extend([mean_values])
    
    # 在图的开头插入预处理节点（不需要cast，输入已经是float32）
    new_nodes = [split_node, concat_node, sub_node]
    for i, node in enumerate(new_nodes):
        graph.node.insert(i, node)
    
    # 替换输入
    del graph.input[0]
    graph.input.insert(0, new_input)
    
    # 保存新模型
    print(f"Saving preprocessed model: {ONNX_MODEL_PREPROC}")
    onnx.save(model, ONNX_MODEL_PREPROC)
    
    # 验证模型
    try:
        onnx.checker.check_model(model)
        print("Model validation passed!")
    except Exception as e:
        print(f"Model validation warning: {e}")
    
    return True

def test_preprocessed_model():
    """测试预处理后的模型"""
    print("\n=== Testing preprocessed model ===")
    
    try:
        import onnxruntime as ort
        from PIL import Image
    except ImportError:
        print("onnxruntime or PIL not installed, skipping test")
        return
    
    test_image = "/mnt/e/Download/porn.jpg"
    if not os.path.exists(test_image):
        print(f"Test image not found: {test_image}")
        return
    
    # 加载图片 - 简单预处理，转为float32
    img = Image.open(test_image).convert('RGB')
    img = img.resize((224, 224), Image.BILINEAR)
    img_np = np.array(img, dtype=np.float32)  # 0-255 range
    img_np = np.expand_dims(img_np, axis=0)  # [1, 224, 224, 3]
    
    print(f"Input shape: {img_np.shape}, dtype: {img_np.dtype}")
    
    # 运行推理
    session = ort.InferenceSession(ONNX_MODEL_PREPROC)
    input_name = session.get_inputs()[0].name
    print(f"Model input name: {input_name}")
    
    outputs = session.run(None, {input_name: img_np})
    out = outputs[0].flatten()
    print(f"Output: SFW={out[0]:.6f}, NSFW={out[1]:.6f}")
    
    # 对比原始模型
    print("\n--- Comparing with original model ---")
    img_orig = Image.open(test_image).convert('RGB')
    img_orig = img_orig.resize((256, 256), Image.BILINEAR)
    left = (256 - 224) // 2
    top = (256 - 224) // 2
    img_orig = img_orig.crop((left, top, left + 224, top + 224))
    img_orig_np = np.array(img_orig, dtype=np.float32)
    img_orig_np = img_orig_np[:, :, ::-1]  # RGB -> BGR
    img_orig_np[:, :, 0] -= 104.0
    img_orig_np[:, :, 1] -= 117.0
    img_orig_np[:, :, 2] -= 123.0
    img_orig_np = np.expand_dims(img_orig_np, axis=0)
    
    session_orig = ort.InferenceSession(ONNX_MODEL)
    outputs_orig = session_orig.run(None, {'input:0': img_orig_np})
    out_orig = outputs_orig[0].flatten()
    print(f"Original model output: SFW={out_orig[0]:.6f}, NSFW={out_orig[1]:.6f}")

if __name__ == "__main__":
    success = add_preprocessing()
    if success:
        test_preprocessed_model()
    sys.exit(0 if success else 1)
