"""Batch test harness for the Open_NSFW ONNX model."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import List, Sequence

import numpy as np
import onnxruntime as ort
from PIL import Image

VGG_MEAN = np.array([104.0, 117.0, 123.0], dtype=np.float32)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run open_nsfw.onnx against a folder of images.")
    parser.add_argument(
        "--model-path",
        type=Path,
        default=Path("open_nsfw.onnx"),
        help="Path to the ONNX model file (default: open_nsfw.onnx).",
    )
    parser.add_argument(
        "--input-dir",
        type=Path,
        default=Path("img"),
        help="Directory containing images to classify (default: img).",
    )
    parser.add_argument(
        "--glob",
        type=str,
        default="*",
        help="Glob pattern to select images inside the folder (default: *).",
    )
    parser.add_argument(
        "--save-json",
        type=Path,
        help="Optional path to save classification results as JSON.",
    )
    return parser.parse_args()


def load_image(image_path: Path) -> np.ndarray:
    image = Image.open(image_path).convert("RGB")
    image = image.resize((256, 256), Image.BILINEAR)

    left = (256 - 224) // 2
    top = (256 - 224) // 2
    image = image.crop((left, top, left + 224, top + 224))

    arr = np.asarray(image).astype(np.float32)
    arr = arr[:, :, ::-1]  # RGB -> BGR
    arr -= VGG_MEAN
    return np.expand_dims(arr, axis=0)


def classify(model_path: Path, images: Sequence[Path]) -> List[dict]:
    if not model_path.exists():
        raise FileNotFoundError(f"ONNX model not found: {model_path}")

    session = ort.InferenceSession(model_path.as_posix(), providers=["CPUExecutionProvider"])
    input_name = session.get_inputs()[0].name
    output_name = session.get_outputs()[0].name

    results = []
    for image_path in images:
        sample = load_image(image_path)
        probs = session.run([output_name], {input_name: sample})[0][0]
        record = {
            "image": image_path.as_posix(),
            "sfw_score": float(probs[0]),
            "nsfw_score": float(probs[1]),
            "pred_label": "NSFW" if probs[1] >= probs[0] else "SFW",
        }
        results.append(record)
    return results


def main() -> None:
    args = parse_args()
    image_dir = args.input_dir
    if not image_dir.is_dir():
        raise NotADirectoryError(f"Input directory not found: {image_dir}")

    image_paths = sorted(image_dir.glob(args.glob))
    if not image_paths:
        raise FileNotFoundError(f"No images matched pattern '{args.glob}' in {image_dir}")

    results = classify(args.model_path, image_paths)

    print(f"Tested {len(results)} images using {args.model_path}")
    for item in results:
        print(
            f"- {Path(item['image']).name}: SFW={item['sfw_score']:.4f} | NSFW={item['nsfw_score']:.4f}"
            f" -> {item['pred_label']}"
        )

    if args.save_json:
        args.save_json.parent.mkdir(parents=True, exist_ok=True)
        with args.save_json.open("w", encoding="utf-8") as fp:
            json.dump(results, fp, ensure_ascii=False, indent=2)
        print(f"Results saved to {args.save_json}")


if __name__ == "__main__":
    main()
