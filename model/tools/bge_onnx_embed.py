"""Utility to run BGE-small-zh-v1.5 ONNX embeddings with onnxruntime."""
from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import List

import numpy as np
import onnxruntime as ort
from transformers import AutoTokenizer

DEFAULT_MODEL_PATH = Path("onnx_models/bge-small-zh-v1.5/bge-small-zh-v1.5.onnx")
TOKENIZER_ID = "BAAI/bge-small-zh-v1.5"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run text embeddings via the exported BGE-small-zh-v1.5 ONNX model."
    )
    parser.add_argument(
        "--model-path",
        type=Path,
        default=DEFAULT_MODEL_PATH,
        help=f"Path to ONNX file (default: {DEFAULT_MODEL_PATH.as_posix()}).",
    )
    parser.add_argument(
        "--text",
        action="append",
        default=[],
        help="Text snippet to encode. You can repeat this flag multiple times.",
    )
    parser.add_argument(
        "--file",
        type=Path,
        help="Optional UTF-8 file; each non-empty line will be encoded as one sample.",
    )
    parser.add_argument(
        "--max-length",
        type=int,
        default=512,
        help="Maximum sequence length for the tokenizer (default: 512).",
    )
    parser.add_argument(
        "--pooling",
        choices=["cls", "mean"],
        default="cls",
        help="Select how to pool the last hidden states into sentence embeddings.",
    )
    parser.add_argument(
        "--normalize",
        action="store_true",
        help="If set, L2-normalize each embedding vector.",
    )
    return parser.parse_args()


def collect_texts(args: argparse.Namespace) -> List[str]:
    texts: List[str] = [t for t in args.text or [] if t]
    if args.file:
        if not args.file.exists():
            raise FileNotFoundError(f"Input file '{args.file}' does not exist.")
        with args.file.open("r", encoding="utf-8") as f:
            texts.extend(line.strip() for line in f if line.strip())
    if not texts:
        raise ValueError("请通过 --text 或 --file 提供至少一条输入句子。")
    return texts


def run_inference(model_path: Path, sentences: List[str], max_length: int, pooling: str, normalize: bool) -> np.ndarray:
    if not model_path.exists():
        raise FileNotFoundError(f"ONNX 模型 '{model_path}' 不存在，请先运行导出步骤。")

    tokenizer = AutoTokenizer.from_pretrained(TOKENIZER_ID)
    encoded = tokenizer(
        sentences,
        padding=True,
        truncation=True,
        max_length=max_length,
        return_tensors="np",
    )

    session = ort.InferenceSession(model_path.as_posix(), providers=["CPUExecutionProvider"])
    inputs = {name: encoded[name] for name in (inp.name for inp in session.get_inputs())}
    last_hidden_state = session.run(None, inputs)[0]

    if pooling == "cls":
        embeddings = last_hidden_state[:, 0, :]
    else:
        mask = encoded["attention_mask"][..., None]
        masked = last_hidden_state * mask
        denom = np.clip(mask.sum(axis=1, keepdims=False), a_min=1e-9, a_max=None)
        embeddings = masked.sum(axis=1) / denom

    if normalize:
        norms = np.linalg.norm(embeddings, axis=1, keepdims=True)
        embeddings = embeddings / np.clip(norms, a_min=1e-9, a_max=None)

    return embeddings


def main() -> None:
    args = parse_args()
    try:
        sentences = collect_texts(args)
        embeddings = run_inference(args.model_path, sentences, args.max_length, args.pooling, args.normalize)
    except Exception as exc:  # pragma: no cover - CLI convenience
        print(f"[bge_onnx_embed] 出错: {exc}", file=sys.stderr)
        raise SystemExit(1) from exc

    print(f"生成 {len(sentences)} 条句向量，维度 {embeddings.shape[1]}")
    for idx, (text, vector) in enumerate(zip(sentences, embeddings)):
        preview = ", ".join(f"{value:.4f}" for value in vector[:5])
        print(f"[{idx}] '{text[:30]}...' -> [{preview}, ...]")


if __name__ == "__main__":
    main()
