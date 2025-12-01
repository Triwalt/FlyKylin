#include "NSFWDetector.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QImage>

#if defined(FLYKYLIN_ENABLE_ONNXRUNTIME)
#  if __has_include(<onnxruntime_cxx_api.h>)
#    include <onnxruntime_cxx_api.h>
#    include <memory>
#    include <string>
#    include <vector>
#    define FLYKYLIN_ONNXRUNTIME_COMPILED 1
#  else
#    pragma message("onnxruntime_cxx_api.h not found, building NSFWDetector without ONNX backend")
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

NSFWDetector* NSFWDetector::instance()
{
    static NSFWDetector s_instance;
    return &s_instance;
}

NSFWDetector::NSFWDetector() = default;

bool NSFWDetector::isAvailable() const
{
#ifdef FLYKYLIN_ONNXRUNTIME_COMPILED
    return nsfwContext().available;
#else
    return false;
#endif
}

std::optional<float> NSFWDetector::predictNsfwProbability(const QString& imagePath) const
{
#ifdef FLYKYLIN_ONNXRUNTIME_COMPILED
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

        // open_nsfw outputs [SFW, NSFW]
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
