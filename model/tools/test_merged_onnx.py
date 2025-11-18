"""Run inference on the merged Open-NSFW + BGE ONNX model."""
from __future__ import annotations

import argparse
from pathlib import Path
from typing import List

import numpy as np
import onnxruntime as ort
from PIL import Image
from transformers import AutoTokenizer

TOKENIZER_ID = "BAAI/bge-small-zh-v1.5"
DEFAULT_MODEL = Path("onnx_models/merged_open_nsfw_bge.onnx")
VGG_MEAN = np.array([104.0, 117.0, 123.0], dtype=np.float32)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Test merged ONNX model outputs for image + text inputs")
    parser.add_argument("--image", type=Path, required=True, help="Image path for NSFW branch input")
    parser.add_argument("--text", action="append", help="Text to encode with BGE branch (repeatable)")
    parser.add_argument("--model", type=Path, default=DEFAULT_MODEL, help="Merged ONNX model path")
    parser.add_argument("--max-length", type=int, default=128, help="Tokenizer max length for BGE input")
    parser.add_argument("--tokenizer", type=str, default=TOKENIZER_ID, help="Tokenizer repo or local path")
    return parser.parse_args()


def load_image(image_path: Path) -> np.ndarray:
    if not image_path.exists():
        raise FileNotFoundError(image_path)
    image = Image.open(image_path).convert("RGB")
    image = image.resize((256, 256), Image.BILINEAR)
    left = (256 - 224) // 2
    top = (256 - 224) // 2
    image = image.crop((left, top, left + 224, top + 224))
    arr = np.asarray(image).astype(np.float32)
    arr = arr[:, :, ::-1]
    arr -= VGG_MEAN
    return np.expand_dims(arr, axis=0)


def run_inference(model_path: Path, image_path: Path, texts: List[str], tokenizer_id: str, max_length: int) -> None:
    if not model_path.exists():
        raise FileNotFoundError(model_path)
    if not texts:
        raise ValueError("请至少提供一条文本 (--text)")

    session = ort.InferenceSession(model_path.as_posix(), providers=["CPUExecutionProvider"])
    tokenizer = AutoTokenizer.from_pretrained(tokenizer_id)

    image_tensor = load_image(image_path)
    encoded = tokenizer(
        texts,
        padding=True,
        truncation=True,
        max_length=max_length,
        return_tensors="np",
    )

    inputs = {
        "nsfw_input:0": image_tensor,
        "bge_input_ids": encoded["input_ids"],
        "bge_attention_mask": encoded["attention_mask"],
        "bge_token_type_ids": encoded.get("token_type_ids", np.zeros_like(encoded["input_ids"])),
    }

    outputs = session.run(None, inputs)
    output_map = {out.name: value for out, value in zip(session.get_outputs(), outputs)}

    nsfw_scores = output_map["nsfw_predictions:0"][0]
    print(f"Image '{image_path.name}' -> SFW={nsfw_scores[0]:.4f}, NSFW={nsfw_scores[1]:.4f}")

    embeddings = output_map["bge_last_hidden_state"]
    print(f"Texts encoded: {len(texts)}, embedding shape: {embeddings.shape}")
    for idx, (text, vector) in enumerate(zip(texts, embeddings)):
        preview = ", ".join(f"{v:.4f}" for v in vector[0][:5])
        print(f"[{idx}] '{text[:30]}...' -> [{preview}, ...]")


def main() -> None:
    args = parse_args()
    run_inference(args.model, args.image, args.text or [], args.tokenizer, args.max_length)


if __name__ == "__main__":
    main()