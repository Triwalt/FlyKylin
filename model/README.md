# AI Models Directory

This directory contains AI model files for FlyKylin's intelligent features.

## Directory Structure

```
model/
├── onnx/          # ONNX models (Windows DirectML, cross-platform)
└── rknn/          # RKNN models (RK3566 NPU optimized)
```

## ⚠️ Important Notes

**These model files are excluded from Git repository due to their large size (10MB+)**

Model files must be downloaded separately or generated from source.

## RKNN Models (RK3566 Platform)

### open_nsfw.rknn

**Purpose**: NSFW (Not Safe For Work) content detection  
**Size**: ~12 MB  
**Platform**: RK3566/RK3568 NPU  
**Input**: 224x224 RGB image  
**Output**: NSFW probability score (0.0-1.0)

#### How to Obtain

**Option 1: Download Pre-converted Model** (Recommended)

```bash
# Download from project release
wget https://github.com/YOUR_ORG/FlyKylin/releases/download/models-v1/open_nsfw.rknn
mv open_nsfw.rknn model/rknn/
```

**Option 2: Convert from ONNX**

If you have the ONNX model, convert it using RKNN Toolkit:

```python
from rknn.api import RKNN

# Create RKNN object
rknn = RKNN(verbose=True)

# Config
rknn.config(
    target_platform='rk3566',
    optimization_level=3,
    quantize_input_node=True,
    mean_values=[[104, 117, 123]],
    std_values=[[1, 1, 1]]
)

# Load ONNX model
print('Loading ONNX model')
ret = rknn.load_onnx(model='open_nsfw.onnx')

# Build model
print('Building model')
ret = rknn.build(do_quantization=True, dataset='./dataset.txt')

# Export RKNN model
ret = rknn.export_rknn('open_nsfw.rknn')
```

**Option 3: Train Your Own**

See [NSFW Model Training Guide](../docs/ai/nsfw-model-training.md)

## ONNX Models (Cross-Platform)

### Models to be added

Future models will be placed in `model/onnx/`:
- Translation models (for multilingual support)
- Summarization models (for message digests)
- Semantic search models (for message search)

## Model Versioning

Track model versions in this table:

| Model Name | Version | Date Added | Size | Platform | Purpose |
|------------|---------|------------|------|----------|---------|
| open_nsfw.rknn | 1.0 | 2024-11-18 | 12MB | RK3566 | NSFW Detection |

## Usage in Code

### Loading RKNN Model

```cpp
// src/ai/nsfw/NSFWDetectorRKNN.cpp
#include <rknn_api.h>

rknn_context ctx;
int ret = rknn_init(&ctx, model_path, model_size, 0, nullptr);
if (ret != RKNN_SUCC) {
    qCritical() << "Failed to load RKNN model";
    return false;
}
```

### Loading ONNX Model

```cpp
// src/ai/nsfw/NSFWDetectorONNX.cpp
#include <onnxruntime_cxx_api.h>

Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "FlyKylin");
Ort::SessionOptions session_options;
auto session = Ort::Session(env, model_path, session_options);
```

## Model Download Script

We provide a script to download all required models:

```bash
# Windows
.\tools\developer\download-models.ps1

# Linux
./tools/developer/download-models.sh
```

This script will:
1. Check if models already exist
2. Download from releases or external sources
3. Verify checksums (SHA256)
4. Place models in correct directories

## Model Checksums (SHA256)

Verify downloaded models using these checksums:

```
open_nsfw.rknn: [SHA256 hash will be added when model is finalized]
```

Verify:
```powershell
# Windows
Get-FileHash model\rknn\open_nsfw.rknn -Algorithm SHA256

# Linux
sha256sum model/rknn/open_nsfw.rknn
```

## CI/CD Integration

For automated builds, models should be:
1. **Cached** in CI environment
2. **Downloaded** from artifact storage
3. **Not stored** in Git LFS (to avoid costs)

Example GitHub Actions:

```yaml
- name: Download AI Models
  run: |
    mkdir -p model/rknn
    wget -q https://releases/open_nsfw.rknn -O model/rknn/open_nsfw.rknn
    
- name: Cache models
  uses: actions/cache@v3
  with:
    path: model/
    key: ${{ runner.os }}-models-${{ hashFiles('**/model-versions.txt') }}
```

## Troubleshooting

### Model not found error

```
Error: Failed to load model: model/rknn/open_nsfw.rknn (No such file)
```

**Solution**: Download the model using the script:
```bash
.\tools\developer\download-models.ps1
```

### Model version mismatch

```
Warning: Model version mismatch (expected 1.0, got 0.9)
```

**Solution**: Re-download the latest model version.

### RKNN model load failed on RK3566

```
Error: RKNN init failed with code -1
```

**Solutions**:
1. Ensure RKNN runtime is installed: `sudo apt install rknpu2`
2. Check NPU driver: `lsmod | grep rknpu`
3. Verify model was built for correct platform (rk3566)

## References

- **RKNN Toolkit**: [GitHub](https://github.com/airockchip/rknn-toolkit2)
- **ONNX Runtime**: [Official Docs](https://onnxruntime.ai/)
- **Model Hub**: [Hugging Face](https://huggingface.co/)

## License

Models may have different licenses from the main project. Check individual model licenses:

- open_nsfw: [Yahoo Open-NSFW License](https://github.com/yahoo/open_nsfw)

---

**Last Updated**: 2024-11-18  
**Maintainer**: FlyKylin AI Team
