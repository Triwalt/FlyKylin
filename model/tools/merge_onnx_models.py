"""Merge two ONNX graphs into a single multi-output model."""
from __future__ import annotations

import argparse
from pathlib import Path

import onnx
from onnx import compose, version_converter


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Merge two ONNX models via onnx.compose.merge_models")
    parser.add_argument("primary", type=Path, help="Path to the primary ONNX model (e.g. open_nsfw.onnx)")
    parser.add_argument("secondary", type=Path, help="Path to the secondary ONNX model (e.g. BGE encoder)")
    parser.add_argument("output", type=Path, help="Destination path for the merged ONNX graph")
    parser.add_argument(
        "--primary-prefix",
        default="nsfw_",
        help="Prefix applied to nodes/initializers of the primary model to avoid naming collisions.",
    )
    parser.add_argument(
        "--secondary-prefix",
        default="bge_",
        help="Prefix applied to nodes/initializers of the secondary model to avoid naming collisions.",
    )
    return parser.parse_args()


def merge(primary: Path, secondary: Path, output: Path, prefix1: str, prefix2: str) -> None:
    if not primary.exists():
        raise FileNotFoundError(primary)
    if not secondary.exists():
        raise FileNotFoundError(secondary)

    model_a = onnx.load(primary.as_posix())
    model_b = onnx.load(secondary.as_posix())

    target_ir = max(model_a.ir_version or 7, model_b.ir_version or 7)
    model_a.ir_version = target_ir
    model_b.ir_version = target_ir

    target_opset = max(
        [opset.version for opset in model_a.opset_import if opset.domain == ""]
        + [opset.version for opset in model_b.opset_import if opset.domain == ""]
    )

    def ensure_opset(model):
        current = next((op.version for op in model.opset_import if op.domain == ""), None)
        if current is not None and current != target_opset:
            return version_converter.convert_version(model, target_opset)
        return model

    model_a = ensure_opset(model_a)
    model_b = ensure_opset(model_b)

    if prefix1:
        model_a = compose.add_prefix(model_a, prefix1)
    if prefix2:
        model_b = compose.add_prefix(model_b, prefix2)

    merged = compose.merge_models(model_a, model_b, io_map=[])

    output.parent.mkdir(parents=True, exist_ok=True)
    onnx.save(merged, output.as_posix())
    print(f"Merged model saved to {output}")


def main() -> None:
    args = parse_args()
    merge(args.primary, args.secondary, args.output, args.primary_prefix, args.secondary_prefix)


if __name__ == "__main__":
    main()
