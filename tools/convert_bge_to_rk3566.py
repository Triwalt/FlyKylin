#!/usr/bin/env python3
"""
将 bge-small-zh-v1.5.onnx 模型转换为 RK3566 兼容的 RKNN 模型

BGE模型特点：
- 输入: input_ids [1, seq_len], attention_mask [1, seq_len], token_type_ids [1, seq_len]
- 输入类型: int64
- 输出: sentence_embedding [1, 512] (bge-small-zh-v1.5)

注意：RKNN对Transformer模型的支持有限，可能需要：
1. 简化模型结构
2. 使用动态量化
3. 或者在板端使用CPU推理作为fallback
"""

import os
import sys
import numpy as np

try:
    from rknn.api import RKNN
except ImportError:
    print("Error: rknn-toolkit2 not installed")
    print("Install with: pip install rknn-toolkit2")
    sys.exit(1)

# 路径配置
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "bge-small-zh-v1.5.onnx")
RKNN_MODEL = os.path.join(PROJECT_ROOT, "model", "rknn", "bge-small-zh-v1.5.rknn")

# BGE模型参数
MAX_SEQ_LEN = 128  # 最大序列长度
EMBEDDING_DIM = 512  # bge-small-zh-v1.5 输出维度


def analyze_onnx_model():
    """分析ONNX模型结构"""
    try:
        import onnx
    except ImportError:
        print("Warning: onnx not installed, skipping analysis")
        return None
    
    print(f"=== Analyzing ONNX Model ===")
    print(f"Model: {ONNX_MODEL}")
    
    if not os.path.exists(ONNX_MODEL):
        print(f"Error: Model not found")
        return None
    
    # 使用ONNX直接加载模型
    model = onnx.load(ONNX_MODEL)
    print(f"IR Version: {model.ir_version}")
    print(f"Opset: {[op.version for op in model.opset_import]}")
    
    print("\nInputs:")
    inputs_info = []
    for inp in model.graph.input:
        shape = [d.dim_value if d.dim_value else d.dim_param for d in inp.type.tensor_type.shape.dim]
        dtype = inp.type.tensor_type.elem_type
        print(f"  - {inp.name}: type={dtype} shape={shape}")
        inputs_info.append({
            'name': inp.name,
            'type': dtype,
            'shape': shape
        })
    
    print("\nOutputs:")
    for out in model.graph.output:
        shape = [d.dim_value if d.dim_value else d.dim_param for d in out.type.tensor_type.shape.dim]
        dtype = out.type.tensor_type.elem_type
        print(f"  - {out.name}: type={dtype} shape={shape}")
    
    return inputs_info


def test_onnx_inference():
    """测试ONNX推理 - 跳过，因为WSL的onnxruntime版本太旧"""
    print(f"\n=== Skipping ONNX Inference Test ===")
    print("Note: WSL onnxruntime version is too old for this model")
    print("The model works correctly on Windows with newer onnxruntime")


def convert_model_fp16():
    """转换为FP16 RKNN模型"""
    print(f"\n=== Converting BGE to RKNN (FP16) ===")
    print(f"Input: {ONNX_MODEL}")
    print(f"Output: {RKNN_MODEL}")
    
    if not os.path.exists(ONNX_MODEL):
        print(f"Error: ONNX model not found: {ONNX_MODEL}")
        return False
    
    rknn = RKNN(verbose=True)
    
    # 配置
    print("Configuring model for RK3566 (FP16)...")
    rknn.config(
        target_platform='rk3566',
        optimization_level=3
    )
    
    # 加载ONNX - BGE有3个输入
    print("Loading ONNX model...")
    ret = rknn.load_onnx(
        model=ONNX_MODEL,
        inputs=['input_ids', 'attention_mask', 'token_type_ids'],
        input_size_list=[
            [1, MAX_SEQ_LEN],  # input_ids
            [1, MAX_SEQ_LEN],  # attention_mask
            [1, MAX_SEQ_LEN]   # token_type_ids
        ]
    )
    if ret != 0:
        print(f"Load ONNX failed: {ret}")
        rknn.release()
        return False
    
    # 构建（不量化）
    print("Building RKNN model (FP16)...")
    ret = rknn.build(do_quantization=False)
    if ret != 0:
        print(f"Build failed: {ret}")
        print("\nNote: BGE (Transformer) models may not be fully supported by RKNN.")
        print("Consider using CPU inference on the board as fallback.")
        rknn.release()
        return False
    
    # 导出
    print(f"Exporting to {RKNN_MODEL}...")
    os.makedirs(os.path.dirname(RKNN_MODEL), exist_ok=True)
    ret = rknn.export_rknn(RKNN_MODEL)
    if ret != 0:
        print(f"Export failed: {ret}")
        rknn.release()
        return False
    
    rknn.release()
    print(f"Success! Size: {os.path.getsize(RKNN_MODEL) / 1024 / 1024:.2f} MB")
    return True


def test_rknn_inference():
    """测试RKNN推理（PC模拟器）"""
    print(f"\n=== Testing RKNN Inference (PC Simulator) ===")
    
    if not os.path.exists(RKNN_MODEL):
        print(f"Error: RKNN model not found: {RKNN_MODEL}")
        return
    
    rknn = RKNN(verbose=False)
    
    ret = rknn.load_rknn(RKNN_MODEL)
    if ret != 0:
        print(f"Load RKNN failed: {ret}")
        return
    
    ret = rknn.init_runtime(target=None)  # PC模拟器
    if ret != 0:
        print(f"Init runtime failed: {ret}")
        rknn.release()
        return
    
    # 创建测试输入
    input_ids = np.array([[101, 872, 1962, 102] + [0] * (MAX_SEQ_LEN - 4)], dtype=np.int64)
    attention_mask = np.array([[1, 1, 1, 1] + [0] * (MAX_SEQ_LEN - 4)], dtype=np.int64)
    token_type_ids = np.array([[0] * MAX_SEQ_LEN], dtype=np.int64)
    
    outputs = rknn.inference(inputs=[input_ids, attention_mask, token_type_ids])
    
    if outputs and len(outputs) > 0:
        print(f"Output shape: {outputs[0].shape}")
        print(f"Output sample: {outputs[0][0][:5]}...")
        print(f"Output norm: {np.linalg.norm(outputs[0][0]):.4f}")
    else:
        print("Inference failed or empty output")
    
    rknn.release()


if __name__ == "__main__":
    # 1. 分析ONNX模型
    analyze_onnx_model()
    
    # 2. 测试ONNX推理
    test_onnx_inference()
    
    # 3. 尝试转换为RKNN
    success = convert_model_fp16()
    
    if success:
        # 4. 测试RKNN推理
        test_rknn_inference()
        
        print("\n=== Notes ===")
        print("BGE RKNN model created successfully!")
        print("However, Transformer models may have limited RKNN support.")
        print("If performance is poor, consider:")
        print("  1. Using CPU inference on the board")
        print("  2. Using a simpler embedding model")
        print("  3. Pre-computing embeddings offline")
    else:
        print("\n=== Fallback Strategy ===")
        print("RKNN conversion failed. Recommended alternatives:")
        print("  1. Use ONNX Runtime on board (if available)")
        print("  2. Use keyword-based search as fallback")
        print("  3. Pre-compute embeddings on PC and sync to board")
