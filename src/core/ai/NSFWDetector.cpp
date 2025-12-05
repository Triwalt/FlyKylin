#include "NSFWDetector.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QStringList>

#include <cstring>
#include <memory>
#include <string>
#include <vector>

#if defined(FLYKYLIN_ENABLE_ONNXRUNTIME)
#  if __has_include(<onnxruntime_cxx_api.h>)
#    include <onnxruntime_cxx_api.h>
#    define FLYKYLIN_ONNXRUNTIME_COMPILED 1
#  else
#    pragma message("onnxruntime_cxx_api.h not found, building NSFWDetector without ONNX backend")
#  endif
#endif

#if defined(RK3566_PLATFORM) && defined(FLYKYLIN_ENABLE_RKNN)
#  if __has_include(<rknn_api.h>)
#    include <rknn_api.h>
#    define FLYKYLIN_RKNN_COMPILED 1
#  else
#    pragma message("rknn_api.h not found, building NSFWDetector without RKNN backend")
#  endif
#endif

namespace flykylin {
namespace ai {

#ifdef FLYKYLIN_ONNXRUNTIME_COMPILED
namespace {

class OnnxNsfwContext {
public:
    OnnxNsfwContext()
        : available(false)
    {
        QString appDir = QCoreApplication::applicationDirPath();
        QDir dir(appDir);
        QString modelPath = dir.filePath("models/open_nsfw.onnx");

        if (modelPath.isEmpty()) {
            return;
        }

        try {
            env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "FlyKylinNSFW");
            Ort::SessionOptions options;

            const ORTCHAR_T* ortPath = nullptr;
#ifdef _WIN32
            std::wstring wpath = modelPath.toStdWString();
            ortPath = wpath.c_str();
#else
            QByteArray utf8 = modelPath.toUtf8();
            std::string path(utf8.constData(), static_cast<std::size_t>(utf8.size()));
            if (path.empty()) {
                return;
            }
            ortPath = path.c_str();
#endif

            session = std::make_unique<Ort::Session>(*env, ortPath, options);

            Ort::AllocatorWithDefaultOptions allocator;

            Ort::AllocatedStringPtr inputNamePtr = session->GetInputNameAllocated(0, allocator);
            if (inputNamePtr) {
                inputName = std::string(inputNamePtr.get());
            }

            Ort::AllocatedStringPtr outputNamePtr = session->GetOutputNameAllocated(0, allocator);
            if (outputNamePtr) {
                outputName = std::string(outputNamePtr.get());
            }

            if (!inputName.empty() && !outputName.empty()) {
                available = true;
                qInfo() << "[NSFWDetector] ONNX backend initialized";
            }
        } catch (const Ort::Exception& ex) {
            qWarning() << "[NSFWDetector] Failed to initialize ONNX backend:" << ex.what();
            env.reset();
            session.reset();
            inputName.clear();
            outputName.clear();
            available = false;
        }
    }

    bool available;
    std::unique_ptr<Ort::Env> env;
    std::unique_ptr<Ort::Session> session;
    std::string inputName;
    std::string outputName;
};

OnnxNsfwContext& nsfwContext()
{
    static OnnxNsfwContext ctx;
    return ctx;
}

} // namespace
#endif

#ifdef FLYKYLIN_RKNN_COMPILED
namespace {

class RknnNsfwContext {
public:
    RknnNsfwContext()
        : available(false)
        , ctx(0)
        , width(0)
        , height(0)
        , channels(0)
    {
        QString appDir = QCoreApplication::applicationDirPath();
        QDir dir(appDir);

        QStringList candidates;
        candidates << dir.filePath("models/open_nsfw.rknn")
                   << dir.filePath("model/rknn/open_nsfw.rknn")
                   << QStringLiteral("/home/kylin/benchmark/models/open_nsfw.rknn")
                   << QStringLiteral("/userdata/rootfs_overlay/home/kylin/benchmark/models/open_nsfw.rknn");

        QString modelPath;
        for (const QString& c : candidates) {
            if (QFile::exists(c)) {
                modelPath = c;
                break;
            }
        }

        if (modelPath.isEmpty()) {
            qWarning() << "[NSFWDetector] RKNN model not found in candidates" << candidates;
            return;
        }

        QFile f(modelPath);
        if (!f.open(QIODevice::ReadOnly)) {
            qWarning() << "[NSFWDetector] Failed to open RKNN model" << modelPath;
            return;
        }

        QByteArray data = f.readAll();
        if (data.isEmpty()) {
            qWarning() << "[NSFWDetector] Empty RKNN model file" << modelPath;
            return;
        }

        rknn_context localCtx = 0;
        int ret = rknn_init(&localCtx, data.data(), static_cast<uint32_t>(data.size()), 0, nullptr);
        if (ret != RKNN_SUCC || !localCtx) {
            qWarning() << "[NSFWDetector] rknn_init failed" << ret;
            return;
        }

        rknn_input_output_num ioNum;
        std::memset(&ioNum, 0, sizeof(ioNum));
        ret = rknn_query(localCtx, RKNN_QUERY_IN_OUT_NUM, &ioNum, sizeof(ioNum));
        if (ret != RKNN_SUCC || ioNum.n_input < 1 || ioNum.n_output < 1) {
            qWarning() << "[NSFWDetector] rknn_query in/out num failed" << ret;
            rknn_destroy(localCtx);
            return;
        }

        std::memset(&inputAttr, 0, sizeof(inputAttr));
        inputAttr.index = 0;
        ret = rknn_query(localCtx, RKNN_QUERY_INPUT_ATTR, &inputAttr, sizeof(inputAttr));
        if (ret != RKNN_SUCC) {
            qWarning() << "[NSFWDetector] rknn_query input attr failed" << ret;
            rknn_destroy(localCtx);
            return;
        }

        std::memset(&outputAttr, 0, sizeof(outputAttr));
        outputAttr.index = 0;
        ret = rknn_query(localCtx, RKNN_QUERY_OUTPUT_ATTR, &outputAttr, sizeof(outputAttr));
        if (ret != RKNN_SUCC) {
            qWarning() << "[NSFWDetector] rknn_query output attr failed" << ret;
            rknn_destroy(localCtx);
            return;
        }

        if (inputAttr.n_dims == 4) {
            if (inputAttr.fmt == RKNN_TENSOR_NHWC) {
                height = inputAttr.dims[1];
                width = inputAttr.dims[2];
                channels = inputAttr.dims[3];
            } else if (inputAttr.fmt == RKNN_TENSOR_NCHW) {
                height = inputAttr.dims[2];
                width = inputAttr.dims[3];
                channels = inputAttr.dims[1];
            }
        }

        if (width <= 0 || height <= 0 || channels != 3) {
            qWarning() << "[NSFWDetector] Unexpected RKNN input shape" << width << height << channels;
            rknn_destroy(localCtx);
            return;
        }

        ctx = localCtx;
        available = true;
        qInfo() << "[NSFWDetector] RKNN backend initialized, model =" << modelPath
                << "input size =" << width << "x" << height;
    }

    ~RknnNsfwContext()
    {
        if (ctx) {
            rknn_destroy(ctx);
            ctx = 0;
        }
    }

    bool available;
    rknn_context ctx;
    int width;
    int height;
    int channels;
    rknn_tensor_attr inputAttr;
    rknn_tensor_attr outputAttr;
};

RknnNsfwContext& rknnContext()
{
    static RknnNsfwContext ctx;
    return ctx;
}

} // namespace
#endif

NSFWDetector* NSFWDetector::instance()
{
    static NSFWDetector s_instance;
    return &s_instance;
}

NSFWDetector::NSFWDetector() = default;

bool NSFWDetector::isAvailable() const
{
#if defined(FLYKYLIN_RKNN_COMPILED)
    return rknnContext().available;
#elif defined(FLYKYLIN_ONNXRUNTIME_COMPILED)
    return nsfwContext().available;
#else
    return false;
#endif
}

std::optional<float> NSFWDetector::predictNsfwProbability(const QString& imagePath) const
{
#if defined(FLYKYLIN_RKNN_COMPILED)
    RknnNsfwContext& ctx = rknnContext();
    if (!ctx.available || !ctx.ctx) {
        Q_UNUSED(imagePath);
        return std::nullopt;
    }

    QImage image(imagePath);
    if (image.isNull()) {
        qWarning() << "[NSFWDetector] Failed to load image" << imagePath;
        return std::nullopt;
    }

    QImage resized = image.scaled(ctx.width, ctx.height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if (resized.isNull()) {
        qWarning() << "[NSFWDetector] Failed to resize image" << imagePath;
        return std::nullopt;
    }

    if (resized.format() != QImage::Format_RGB888) {
        resized = resized.convertToFormat(QImage::Format_RGB888);
    }

    const int width = resized.width();
    const int height = resized.height();
    if (width != ctx.width || height != ctx.height || ctx.channels != 3) {
        qWarning() << "[NSFWDetector] Unexpected RKNN input size" << width << height << ctx.channels;
        return std::nullopt;
    }

    std::vector<uint8_t> inputData(static_cast<std::size_t>(width * height * 3));

    for (int y = 0; y < height; ++y) {
        const uchar* line = resized.constScanLine(y);
        for (int x = 0; x < width; ++x) {
            const int idx = (y * width + x) * 3;
            const uchar r = line[3 * x + 0];
            const uchar g = line[3 * x + 1];
            const uchar b = line[3 * x + 2];

            inputData[static_cast<std::size_t>(idx + 0)] = b;
            inputData[static_cast<std::size_t>(idx + 1)] = g;
            inputData[static_cast<std::size_t>(idx + 2)] = r;
        }
    }

    rknn_input input;
    std::memset(&input, 0, sizeof(input));
    input.index = 0;
    input.pass_through = 0;
    input.type = RKNN_TENSOR_UINT8;
    input.fmt = ctx.inputAttr.fmt;
    input.size = static_cast<uint32_t>(inputData.size());
    input.buf = inputData.data();

    int ret = rknn_inputs_set(ctx.ctx, 1, &input);
    if (ret != RKNN_SUCC) {
        qWarning() << "[NSFWDetector] rknn_inputs_set failed" << ret;
        return std::nullopt;
    }

    ret = rknn_run(ctx.ctx, nullptr);
    if (ret != RKNN_SUCC) {
        qWarning() << "[NSFWDetector] rknn_run failed" << ret;
        return std::nullopt;
    }

    rknn_output output;
    std::memset(&output, 0, sizeof(output));
    output.want_float = 1;
    output.is_prealloc = 0;
    output.index = 0;

    ret = rknn_outputs_get(ctx.ctx, 1, &output, nullptr);
    if (ret != RKNN_SUCC || !output.buf) {
        qWarning() << "[NSFWDetector] rknn_outputs_get failed" << ret;
        return std::nullopt;
    }

    float nsfwProb = 0.0f;
    float* outData = static_cast<float*>(output.buf);
    if (!outData) {
        qWarning() << "[NSFWDetector] RKNN output data pointer is null";
        rknn_outputs_release(ctx.ctx, 1, &output);
        return std::nullopt;
    }

    if (ctx.outputAttr.n_elems >= 2) {
        nsfwProb = outData[1];
    } else if (ctx.outputAttr.n_elems >= 1) {
        nsfwProb = outData[0];
    } else {
        qWarning() << "[NSFWDetector] RKNN output tensor has no elements";
        rknn_outputs_release(ctx.ctx, 1, &output);
        return std::nullopt;
    }

    rknn_outputs_release(ctx.ctx, 1, &output);
    return nsfwProb;
#elif defined(FLYKYLIN_ONNXRUNTIME_COMPILED)
    OnnxNsfwContext& ctx = nsfwContext();
    if (!ctx.available || !ctx.session || !ctx.env) {
        Q_UNUSED(imagePath);
        return std::nullopt;
    }

    QImage image(imagePath);
    if (image.isNull()) {
        qWarning() << "[NSFWDetector] Failed to load image" << imagePath;
        return std::nullopt;
    }

    QImage resized = image.scaled(256, 256, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    const int cropSize = 224;
    const int left = (256 - cropSize) / 2;
    const int top = (256 - cropSize) / 2;
    QImage cropped = resized.copy(left, top, cropSize, cropSize);
    if (cropped.isNull()) {
        qWarning() << "[NSFWDetector] Failed to crop image" << imagePath;
        return std::nullopt;
    }

    if (cropped.format() != QImage::Format_RGB888) {
        cropped = cropped.convertToFormat(QImage::Format_RGB888);
    }

    const int width = cropped.width();
    const int height = cropped.height();
    if (width != cropSize || height != cropSize) {
        qWarning() << "[NSFWDetector] Unexpected cropped size" << width << height;
        return std::nullopt;
    }

    constexpr float kMeanB = 104.0f;
    constexpr float kMeanG = 117.0f;
    constexpr float kMeanR = 123.0f;

    std::vector<float> inputData(static_cast<std::size_t>(width * height * 3));

    for (int y = 0; y < height; ++y) {
        const uchar* line = cropped.constScanLine(y);
        for (int x = 0; x < width; ++x) {
            const int idx = (y * width + x) * 3;
            const uchar r = line[3 * x + 0];
            const uchar g = line[3 * x + 1];
            const uchar b = line[3 * x + 2];

            inputData[static_cast<std::size_t>(idx + 0)] = static_cast<float>(b) - kMeanB;
            inputData[static_cast<std::size_t>(idx + 1)] = static_cast<float>(g) - kMeanG;
            inputData[static_cast<std::size_t>(idx + 2)] = static_cast<float>(r) - kMeanR;
        }
    }

    std::vector<int64_t> inputShape{1, height, width, 3};

    try {
        Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(memInfo,
                                                                 inputData.data(),
                                                                 inputData.size(),
                                                                 inputShape.data(),
                                                                 inputShape.size());

        const char* inputNames[] = {ctx.inputName.c_str()};
        const char* outputNames[] = {ctx.outputName.c_str()};

        std::vector<Ort::Value> outputs =
            ctx.session->Run(Ort::RunOptions{nullptr}, inputNames, &inputTensor, 1, outputNames, 1);

        if (outputs.empty() || !outputs[0].IsTensor()) {
            qWarning() << "[NSFWDetector] Empty or non-tensor output";
            return std::nullopt;
        }

        float* outData = outputs[0].GetTensorMutableData<float>();
        if (!outData) {
            qWarning() << "[NSFWDetector] Output data pointer is null";
            return std::nullopt;
        }

        const float nsfwProb = outData[1];
        return nsfwProb;
    } catch (const Ort::Exception& ex) {
        qWarning() << "[NSFWDetector] ONNX Run failed:" << ex.what();
        return std::nullopt;
    } catch (const std::exception& ex) {
        qWarning() << "[NSFWDetector] std::exception:" << ex.what();
        return std::nullopt;
    } catch (...) {
        qWarning() << "[NSFWDetector] Unknown exception during predictNsfwProbability";
        return std::nullopt;
    }
#else
    Q_UNUSED(imagePath);
    return std::nullopt;
#endif
}

} // namespace ai
} // namespace flykylin
