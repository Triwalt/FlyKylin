import os
import sys
import argparse
import tempfile

import tensorflow as tf

sys.path.append((os.path.normpath(
                 os.path.join(os.path.dirname(os.path.realpath(__file__)),
                              '..'))))

from model import OpenNsfwModel, InputType

"""Exports a tflite version of tensorflow-open_nsfw

Note: The standard TFLite runtime does not support all required ops when using the base64_jpeg input type.
You will have to implement the missing ones by yourself.
"""
try:
    # TF2 defaults to eager mode which is incompatible with the Session-based
    # export logic. Disabling eager ensures graph mode execution across TF1/TF2.
    tf.compat.v1.disable_eager_execution()
except AttributeError:
    # TF1 does not provide tf.compat.v1; nothing to disable there.
    pass

tf1 = tf.compat.v1 if hasattr(tf, "compat") and hasattr(tf.compat, "v1") else tf


def _create_converter(session, inputs, outputs):
    """Return a TFLite converter that works across TF versions."""

    compat_converter = None
    if hasattr(tf, "compat") and hasattr(tf.compat, "v1"):
        compat_lite = getattr(tf.compat.v1, "lite", None)
        if compat_lite is not None:
            compat_converter = getattr(compat_lite, "TFLiteConverter", None)
            if compat_converter is not None and hasattr(compat_converter, "from_session"):
                return compat_converter.from_session(session, inputs, outputs)

    if hasattr(tf, "lite") and hasattr(tf.lite, "TFLiteConverter"):
        converter_cls = tf.lite.TFLiteConverter
        if hasattr(converter_cls, "from_session"):
            return converter_cls.from_session(session, inputs, outputs)

    try:
        import importlib

        tf_contrib_lite = importlib.import_module("tensorflow.contrib.lite")
        converter_cls = tf_contrib_lite.TFLiteConverter
        if hasattr(converter_cls, "from_session"):
            return converter_cls.from_session(session, inputs, outputs)
    except (ImportError, ModuleNotFoundError):
        pass

    raise RuntimeError(
        "Could not locate a TFLite converter supporting from_session. Install "
        "a TensorFlow version with TFLiteConverter.from_session support."
    )


def _convert_via_frozen_graph(session, model):
    input_arrays = [model.input.op.name]
    output_arrays = [model.predictions.op.name]

    frozen_graph_def = tf1.graph_util.convert_variables_to_constants(
        session,
        session.graph.as_graph_def(),
        output_arrays
    )

    with tempfile.NamedTemporaryFile(suffix=".pb", delete=False) as tmp_f:
        tmp_f.write(frozen_graph_def.SerializeToString())
        frozen_graph_path = tmp_f.name

    try:
        converter_cls = None
        if hasattr(tf, "compat") and hasattr(tf.compat, "v1"):
            compat_lite = getattr(tf.compat.v1, "lite", None)
            if compat_lite is not None:
                converter_cls = getattr(compat_lite, "TFLiteConverter", None)

        if converter_cls is None and hasattr(tf, "lite") and hasattr(tf.lite, "TFLiteConverter"):
            converter_cls = tf.lite.TFLiteConverter

        if converter_cls is None:
            raise RuntimeError("Unable to locate a TFLite converter for frozen graph conversion")

        converter = converter_cls.from_frozen_graph(
            frozen_graph_path,
            input_arrays=input_arrays,
            output_arrays=output_arrays,
            input_shapes={input_arrays[0]: model.input.shape.as_list()}
        )
        return converter.convert()
    finally:
        try:
            os.remove(frozen_graph_path)
        except OSError:
            pass


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("target", help="output filename, e.g. 'open_nsfw.tflite'")

    parser.add_argument("-i", "--input_type", required=True,
                        default=InputType.TENSOR.name.lower(),
                        help="Input type. Warning: base64_jpeg does not work with the standard TFLite runtime since a lot of operations are not supported",
                        choices=[InputType.TENSOR.name.lower(),
                                 InputType.BASE64_JPEG.name.lower()])

    parser.add_argument("-m", "--model_weights", required=True,
                        help="Path to trained model weights file")

    args = parser.parse_args()

    model = OpenNsfwModel()

    export_path = args.target
    input_type = InputType[args.input_type.upper()]

    with tf1.Session() as sess:
        model.build(weights_path=args.model_weights,
                    input_type=input_type)

        sess.run(tf1.global_variables_initializer())

        try:
            converter = _create_converter(sess, [model.input], [model.predictions])
            tflite_model = converter.convert()
        except Exception as converter_error:
            sys.stderr.write(
                "Standard TFLite conversion via from_session failed ({}). "
                "Falling back to frozen-graph conversion...\n".format(converter_error)
            )
            tflite_model = _convert_via_frozen_graph(sess, model)

        with open(export_path, "wb") as f:
            f.write(tflite_model)
