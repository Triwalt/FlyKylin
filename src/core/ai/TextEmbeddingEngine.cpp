#include "TextEmbeddingEngine.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>

#if defined(FLYKYLIN_ENABLE_ONNXRUNTIME)
#  if __has_include(<onnxruntime_cxx_api.h>)
#    include <onnxruntime_cxx_api.h>
#    include <memory>
#    include <string>
#    include <unordered_map>
#    include <vector>
#    define FLYKYLIN_ONNXRUNTIME_COMPILED 1
#  else
#    pragma message("onnxruntime_cxx_api.h not found, building without ONNX backend")
#  endif
#endif

namespace flykylin {
namespace ai {

namespace {

#ifdef FLYKYLIN_ONNXRUNTIME_COMPILED
class BgeTokenizer {
public:
    BgeTokenizer()
        : m_loaded(false)
        , m_clsId(-1)
        , m_sepId(-1)
        , m_padId(-1)
        , m_unkId(-1)
    {
        QString appDir = QCoreApplication::applicationDirPath();
        QDir dir(appDir);
        QString vocabPath = dir.filePath("models/text-embedding-vocab.txt");

        QFile file(vocabPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "[TextEmbeddingEngine] BGE vocab not found at" << vocabPath;
            return;
        }

        QTextStream in(&file);
        int index = 0;
        while (!in.atEnd()) {
            const QString line = in.readLine().trimmed();
            if (line.isEmpty()) {
                continue;
            }

            m_tokenToId.insert(line, index);
            if (line == QLatin1String("[CLS]")) {
                m_clsId = index;
            } else if (line == QLatin1String("[SEP]")) {
                m_sepId = index;
            } else if (line == QLatin1String("[PAD]")) {
                m_padId = index;
            } else if (line == QLatin1String("[UNK]")) {
                m_unkId = index;
            }

            ++index;
        }

        if (m_clsId < 0 || m_sepId < 0 || m_padId < 0 || m_unkId < 0) {
            qWarning() << "[TextEmbeddingEngine] BGE vocab missing required special tokens";
            return;
        }

        m_loaded = true;
    }

    bool isLoaded() const
    {
        return m_loaded;
    }

    void encode(const QString& text,
                std::vector<int64_t>& inputIds,
                std::vector<int64_t>& attentionMask,
                int maxLength) const
    {
        inputIds.clear();
        attentionMask.clear();

        if (!m_loaded || maxLength <= 0) {
            return;
        }

        std::vector<int64_t> tokens;
        tokens.reserve(static_cast<std::size_t>(maxLength));

        tokens.push_back(m_clsId);

        const int textLen = text.size();
        for (int i = 0; i < textLen; ++i) {
            const QChar ch = text.at(i);
            if (ch.isSpace()) {
                continue;
            }

            const QString tokenStr(ch);
            const int id = m_tokenToId.value(tokenStr, m_unkId);
            tokens.push_back(id);

            if (static_cast<int>(tokens.size()) >= maxLength - 1) {
                break;
            }
        }

        if (static_cast<int>(tokens.size()) < maxLength) {
            tokens.push_back(m_sepId);
        }

        if (static_cast<int>(tokens.size()) > maxLength) {
            tokens.resize(static_cast<std::size_t>(maxLength));
        }

        inputIds.assign(tokens.begin(), tokens.end());
        attentionMask.assign(inputIds.size(), 1);

        if (static_cast<int>(inputIds.size()) < maxLength) {
            const int padStart = static_cast<int>(inputIds.size());
            inputIds.resize(static_cast<std::size_t>(maxLength), m_padId);
            attentionMask.resize(static_cast<std::size_t>(maxLength), 0);
            Q_UNUSED(padStart);
        }
    }

private:
    bool m_loaded;
    int m_clsId;
    int m_sepId;
    int m_padId;
    int m_unkId;
    QHash<QString, int> m_tokenToId;
};

BgeTokenizer& bgeTokenizer()
{
    static BgeTokenizer tokenizer;
    return tokenizer;
}

class OnnxEmbeddingContext {
public:
    OnnxEmbeddingContext()
        : available(false)
        , dim(0)
        , inputSize(0)
    {
        QString appDir = QCoreApplication::applicationDirPath();
        QDir dir(appDir);
        QString modelPath = dir.filePath("models/text-embedding.onnx");

        if (modelPath.isEmpty()) {
            return;
        }

        try {
            env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "FlyKylinTextEmbed");
            Ort::SessionOptions options;

            const ORTCHAR_T* ortPath = nullptr;
#ifdef _WIN32
            std::wstring wpath = modelPath.toStdWString();
            ortPath = wpath.c_str();
#else
            QByteArray utf8 = modelPath.toUtf8();
            std::string path =
                std::string(utf8.constData(), static_cast<std::size_t>(utf8.size()));
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

            Ort::TypeInfo inputTypeInfo = session->GetInputTypeInfo(0);
            auto inputTensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();
            std::vector<int64_t> inputShape = inputTensorInfo.GetShape();
            if (inputShape.empty()) {
                return;
            }

            std::size_t totalInput = 1;
            for (int64_t d : inputShape) {
                int64_t v = d > 0 ? d : 1;
                totalInput *= static_cast<std::size_t>(v);
            }

            inputSize = totalInput;

            Ort::AllocatedStringPtr outputNamePtr = session->GetOutputNameAllocated(0, allocator);
            if (outputNamePtr) {
                outputName = std::string(outputNamePtr.get());
            }

            Ort::TypeInfo outputTypeInfo = session->GetOutputTypeInfo(0);
            auto outputTensorInfo = outputTypeInfo.GetTensorTypeAndShapeInfo();
            std::vector<int64_t> outputShape = outputTensorInfo.GetShape();
            if (outputShape.empty()) {
                return;
            }

            std::size_t totalOutput = 1;
            for (int64_t d : outputShape) {
                int64_t v = d > 0 ? d : 1;
                totalOutput *= static_cast<std::size_t>(v);
            }

            dim = static_cast<int>(totalOutput);

            if (inputSize > 0 && dim > 0 && !inputName.empty() && !outputName.empty()) {
                available = true;
                qInfo() << "[TextEmbeddingEngine] ONNX backend initialized, dim=" << dim;
            }
        } catch (const Ort::Exception& ex) {
            qWarning() << "[TextEmbeddingEngine] Failed to initialize ONNX backend:" << ex.what();
            env.reset();
            session.reset();
            available = false;
            dim = 0;
            inputSize = 0;
            inputName.clear();
            outputName.clear();
        }
    }

    bool available;
    int dim;
    std::size_t inputSize;
    std::unique_ptr<Ort::Env> env;
    std::unique_ptr<Ort::Session> session;
    std::string inputName;
    std::string outputName;
};

OnnxEmbeddingContext& onnxContext()
{
    static OnnxEmbeddingContext ctx;
    return ctx;
}
#endif

} // namespace

TextEmbeddingEngine* TextEmbeddingEngine::instance()
{
    static TextEmbeddingEngine s_instance;
    return &s_instance;
}

TextEmbeddingEngine::TextEmbeddingEngine()
{
#ifdef FLYKYLIN_ONNXRUNTIME_COMPILED
    OnnxEmbeddingContext& ctx = onnxContext();
    if (!ctx.available) {
        qInfo() << "[TextEmbeddingEngine] ONNX backend not available, semantic search disabled";
    }
#else
    qInfo() << "[TextEmbeddingEngine] Initialized skeleton (no backend configured)";
#endif
}

bool TextEmbeddingEngine::isAvailable() const
{
#ifdef FLYKYLIN_ONNXRUNTIME_COMPILED
    return onnxContext().available;
#else
    return false;
#endif
}

std::vector<float> TextEmbeddingEngine::computeEmbedding(const QString& text) const
{
#ifdef FLYKYLIN_ONNXRUNTIME_COMPILED
    OnnxEmbeddingContext& ctx = onnxContext();
    if (!ctx.available || !ctx.session || !ctx.env) {
        Q_UNUSED(text);
        return {};
    }

    if (text.isEmpty()) {
        return {};
    }

    try {
        Ort::TypeInfo inputTypeInfo = ctx.session->GetInputTypeInfo(0);
        auto inputTensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();
        ONNXTensorElementDataType elementType = inputTensorInfo.GetElementType();

        if (elementType == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
            std::vector<float> inputData(ctx.inputSize, 0.0f);
            QByteArray utf8 = text.toUtf8();
            int len = utf8.size();
            int maxLen = static_cast<int>(ctx.inputSize);
            int copyLen = len < maxLen ? len : maxLen;
            for (int i = 0; i < copyLen; ++i) {
                unsigned char ch = static_cast<unsigned char>(utf8.at(i));
                inputData[static_cast<std::size_t>(i)] = static_cast<float>(ch) / 255.0f;
            }

            std::vector<int64_t> inputShape{1, static_cast<int64_t>(ctx.inputSize)};

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
                return {};
            }

            float* outData = outputs[0].GetTensorMutableData<float>();
            std::size_t outCount = static_cast<std::size_t>(ctx.dim);
            std::vector<float> result(outCount);
            for (std::size_t i = 0; i < outCount; ++i) {
                result[i] = outData[i];
            }

            return result;
        }

        if (elementType == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64) {
            BgeTokenizer& tokenizer = bgeTokenizer();
            if (!tokenizer.isLoaded()) {
                qWarning() << "[TextEmbeddingEngine] BGE tokenizer not loaded; semantic embeddings disabled";
                return {};
            }

            int maxLen = 128;
            std::vector<int64_t> inputShape = inputTensorInfo.GetShape();
            if (!inputShape.empty()) {
                if (inputShape.size() >= 2 && inputShape[1] > 0 && inputShape[1] <= 4096) {
                    maxLen = static_cast<int>(inputShape[1]);
                } else if (inputShape[0] > 0 && inputShape[0] <= 4096) {
                    maxLen = static_cast<int>(inputShape[0]);
                }
            }

            std::vector<int64_t> inputIds;
            std::vector<int64_t> attentionMask;
            tokenizer.encode(text, inputIds, attentionMask, maxLen);
            if (inputIds.empty() || attentionMask.empty()) {
                return {};
            }

            std::vector<int64_t> tokenTypeIds(inputIds.size(), 0);

            std::vector<int64_t> tensorShape{1, static_cast<int64_t>(inputIds.size())};

            Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

            Ort::Value idsTensor = Ort::Value::CreateTensor<int64_t>(memInfo,
                                                                      inputIds.data(),
                                                                      inputIds.size(),
                                                                      tensorShape.data(),
                                                                      tensorShape.size());
            Ort::Value maskTensor = Ort::Value::CreateTensor<int64_t>(memInfo,
                                                                       attentionMask.data(),
                                                                       attentionMask.size(),
                                                                       tensorShape.data(),
                                                                       tensorShape.size());
            Ort::Value typeIdsTensor = Ort::Value::CreateTensor<int64_t>(memInfo,
                                                                          tokenTypeIds.data(),
                                                                          tokenTypeIds.size(),
                                                                          tensorShape.data(),
                                                                          tensorShape.size());

            size_t inputCount = ctx.session->GetInputCount();
            std::vector<std::string> inputNameStorage(inputCount);
            std::vector<const char*> inputNames(inputCount);
            std::vector<Ort::Value> inputTensors(inputCount);

            Ort::AllocatorWithDefaultOptions allocator;
            for (size_t i = 0; i < inputCount; ++i) {
                Ort::AllocatedStringPtr namePtr = ctx.session->GetInputNameAllocated(i, allocator);
                if (namePtr) {
                    inputNameStorage[i] = std::string(namePtr.get());
                }
            }

            for (size_t i = 0; i < inputCount; ++i) {
                inputNames[i] = inputNameStorage[i].c_str();
                const std::string& name = inputNameStorage[i];

                if (name.find("input_ids") != std::string::npos) {
                    inputTensors[i] = std::move(idsTensor);
                } else if (name.find("attention_mask") != std::string::npos) {
                    inputTensors[i] = std::move(maskTensor);
                } else if (name.find("token_type_ids") != std::string::npos) {
                    inputTensors[i] = std::move(typeIdsTensor);
                }
            }

            const char* outputNames[] = {ctx.outputName.c_str()};

            std::vector<Ort::Value> outputs = ctx.session->Run(Ort::RunOptions{nullptr},
                                                                inputNames.data(),
                                                                inputTensors.data(),
                                                                inputCount,
                                                                outputNames,
                                                                1);

            if (outputs.empty() || !outputs[0].IsTensor()) {
                return {};
            }

            float* outData = outputs[0].GetTensorMutableData<float>();
            std::size_t outCount = static_cast<std::size_t>(ctx.dim);
            std::vector<float> result(outCount);
            for (std::size_t i = 0; i < outCount; ++i) {
                result[i] = outData[i];
            }

            return result;
        }

        qWarning() << "[TextEmbeddingEngine] Unsupported ONNX input element type" << static_cast<int>(elementType);
        return {};
    } catch (const Ort::Exception& ex) {
        qWarning() << "[TextEmbeddingEngine] ONNX Run failed:" << ex.what();
        return {};
    } catch (const std::exception& ex) {
        qWarning() << "[TextEmbeddingEngine] ONNX backend std::exception:" << ex.what();
        return {};
    } catch (...) {
        qWarning() << "[TextEmbeddingEngine] ONNX backend unknown exception during computeEmbedding";
        return {};
    }
#else
    Q_UNUSED(text);
    return {};
#endif
}

int TextEmbeddingEngine::embeddingDim() const
{
#ifdef FLYKYLIN_ONNXRUNTIME_COMPILED
    return onnxContext().dim;
#else
    return 0;
#endif
}

QString TextEmbeddingEngine::backendName() const
{
#ifdef FLYKYLIN_ONNXRUNTIME_COMPILED
    return QStringLiteral("onnxruntime");
#else
    return QStringLiteral("unavailable");
#endif
}

} // namespace ai
} // namespace flykylin
