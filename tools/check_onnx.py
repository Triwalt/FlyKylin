#!/usr/bin/env python3
"""检查ONNX模型结构"""
import onnx
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ONNX_MODEL = os.path.join(PROJECT_ROOT, "model", "onnx", "open_nsfw.onnx")

model = onnx.load(ONNX_MODEL)

print("=== ONNX Model Info ===")
print(f"IR version: {model.ir_version}")
print(f"Producer: {model.producer_name} {model.producer_version}")
print(f"Opset: {[op.version for op in model.opset_import]}")

print("\nInputs:")
for inp in model.graph.input:
    shape = [d.dim_value if d.dim_value else d.dim_param for d in inp.type.tensor_type.shape.dim]
    print(f"  {inp.name}: {shape}, dtype={inp.type.tensor_type.elem_type}")

print("\nOutputs:")
for out in model.graph.output:
    shape = [d.dim_value if d.dim_value else d.dim_param for d in out.type.tensor_type.shape.dim]
    print(f"  {out.name}: {shape}, dtype={out.type.tensor_type.elem_type}")

print(f"\nTotal nodes: {len(model.graph.node)}")

# 检查前几个节点
print("\nFirst 5 nodes:")
for i, node in enumerate(model.graph.node[:5]):
    print(f"  {i}: {node.op_type} - {node.name}")
    print(f"      inputs: {list(node.input)}")
    print(f"      outputs: {list(node.output)}")
