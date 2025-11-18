import os
import sys
import argparse
import importlib

import tensorflow as tf

try:
    tf2onnx = importlib.import_module("tf2onnx")
except ImportError as exc:
    raise ImportError(
        "tf2onnx is required for ONNX export. Install it via 'pip install tf2onnx'."
    ) from exc

sys.path.append((os.path.normpath(
                 os.path.join(os.path.dirname(os.path.realpath(__file__)),
                              '..'))))

from model import OpenNsfwModel, InputType


if hasattr(tf, "compat") and hasattr(tf.compat, "v1"):
    tf.compat.v1.disable_eager_execution()
    tf1 = tf.compat.v1
else:
    tf1 = tf


def convert_to_onnx(target_path, model_weights, input_type, opset, large_model):
    model = OpenNsfwModel()
    input_type_enum = InputType[input_type.upper()]

    with tf1.Session() as sess:
        model.build(weights_path=model_weights, input_type=input_type_enum)

        sess.run(tf1.global_variables_initializer())

        input_name = model.input.name
        output_name = model.predictions.name

        frozen_graph_def = tf1.graph_util.convert_variables_to_constants(
            sess,
            sess.graph.as_graph_def(),
            output_node_names=[model.predictions.op.name]
        )

        tf2onnx.convert.from_graph_def(
            graph_def=frozen_graph_def,
            input_names=[input_name],
            output_names=[output_name],
            output_path=target_path,
            opset=opset,
            large_model=large_model,
        )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Export Open NSFW model to ONNX format")
    parser.add_argument("target", help="Output ONNX filename, e.g. 'open_nsfw.onnx'")
    parser.add_argument("-m", "--model_weights", required=True,
                        help="Path to trained model weights file (open_nsfw-weights.npy)")
    parser.add_argument("-i", "--input_type", default=InputType.TENSOR.name.lower(),
                        choices=[InputType.TENSOR.name.lower(), InputType.BASE64_JPEG.name.lower()],
                        help="Model input type. 'tensor' exports a float tensor input; 'base64_jpeg' may require extra ops")
    parser.add_argument("--opset", type=int, default=13,
                        help="Target ONNX opset version (default: 13)")
    parser.add_argument("--large_model", action="store_true",
                        help="Split large initializers into external data files (ONNX large model format)")

    args = parser.parse_args()

    convert_to_onnx(
        target_path=args.target,
        model_weights=args.model_weights,
        input_type=args.input_type,
        opset=args.opset,
        large_model=args.large_model,
    )
