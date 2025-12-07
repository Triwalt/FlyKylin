# RKNN AI加速调试总结

## 最终状态 (2025-12-07) - NSFW检测和语义搜索均已实现！✅

---

## 一、NSFW图像检测 - 已完成 ✅

### 实际应用验证结果

```
[NSFWDetector] RKNN backend initialized, model = "/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn"
[NSFWDetector] predictNsfwProbability called for "/home/kylin/Downloads/porn.jpg"
[NSFWDetector] Image loaded, size: 6336 x 9504
[NSFWDetector] RKNN raw output: sfw= 0.0422974 nsfw= 0.95752 sum= 0.999817
[FileTransferService] NSFW probability for received image = 0.95752
```

### 验证结果汇总

| 测试环境 | porn.jpg NSFW | pig.jpg NSFW | 状态 |
|----------|---------------|--------------|------|
| PC ONNX参考 | 0.9594 | 0.0003 | ✅ |
| PC RKNN模拟器 | 0.9595 | 0.0003 | ✅ |
| 板端RKNN Python | 0.9590 | 0.0003 | ✅ |
| 板端C++独立测试 | 0.9590 | - | ✅ |
| **板端Qt应用** | **0.9575** | - | **✅** |

## 问题根因分析

### 问题1：RKNN输入维度异常
RKNN在转换NHWC格式的ONNX模型时，将输入维度从 `[N,H,W,C]` 变成 `[N,W,C,H]`（NWCH格式）。

**输入格式测试结果：**
| 输入格式 | Shape | porn NSFW | pig NSFW | 状态 |
|---------|-------|-----------|----------|------|
| NHWC | [1,224,224,3] | 0.27 | 0.27 | ❌ |
| NCHW | [1,3,224,224] | 0.27 | 0.27 | ❌ |
| **NWCH** | **[1,224,3,224]** | **0.96** | **0.0003** | ✅ |

### 问题2：pass_through参数设置错误
| pass_through | data_format | NSFW结果 | 状态 |
|--------------|-------------|----------|------|
| **0 (False)** | **nhwc** | **0.9590** | **✅** |
| 1 (True) | nhwc | 0.0483 | ❌ 导致系统卡死 |
| 0 (False) | nchw | 0.0129 | ❌ |
| 1 (True) | nchw | 0.0352 | ❌ |

**结论：必须使用 pass_through=0 + fmt=NHWC！**

## 正确的配置与流程

### 预处理流程
1. 缩放图片到 224x224
2. RGB → BGR 颜色空间转换
3. 减去均值 [B=104, G=117, R=123]
4. 数据布局从 HWC 转换为 WCH (NWCH格式)
5. 使用 float32 类型

### C++ RKNN API调用参数
```cpp
rknn_input input;
memset(&input, 0, sizeof(input));
input.index = 0;
input.pass_through = 0;  // 重要！必须为0
input.type = RKNN_TENSOR_FLOAT32;
input.fmt = RKNN_TENSOR_NHWC;  // 重要！必须为NHWC
input.size = inputData.size() * sizeof(float);
input.buf = inputData.data();
```

### 预处理代码 (HWC → WCH)
```cpp
for (int h = 0; h < height; ++h) {
    for (int w = 0; w < width; ++w) {
        float bVal = b - 104.0f;
        float gVal = g - 117.0f;
        float rVal = r - 123.0f;
        
        // WCH: index = w * (C * H) + c * H + h
        size_t baseIdx = w * 3 * height;
        inputData[baseIdx + 0 * height + h] = bVal;  // B
        inputData[baseIdx + 1 * height + h] = gVal;  // G
        inputData[baseIdx + 2 * height + h] = rVal;  // R
    }
}
```

---

## 测试环境

- **板端**: RK3566, IP: 192.168.100.2, 用户: kylin, 密码: 123456
- **板端项目目录**: `/home/kylin/FlyKylinApp`
- **RKNN库版本**: 2.3.2
- **测试图片**:
  - NSFW图片: `E:\Download\porn.jpg` (PC) / `/mnt/e/Download/porn.jpg` (WSL)
  - 正常图片: `E:\Pictures\pig.jpg` (PC) / `/mnt/e/Pictures/pig.jpg` (WSL)

## 关键文件

### 源代码
- `src/core/ai/NSFWDetector.cpp` - C++ RKNN推理代码（已修复）

### 测试工具
- `tools/test_rknn_pc_simulator.py` - PC端RKNN模拟器测试（推荐）
- `tools/test_rknn_standalone.cpp` - 板端独立C++测试程序
- `tools/test_rknn_board_final.py` - 板端Python验证脚本
- `tools/test_cpp_preprocess.py` - C++预处理逻辑验证
- `tools/generate_nwch_npy.py` - 生成NWCH格式测试数据

### 模型文件
- `model/onnx/open_nsfw.onnx` - 原始ONNX模型
- `model/rknn/open_nsfw_rk3566.rknn` - RKNN模型

## 板端备份

备份位置: `/media/kylin/EED4-7516/FlyKylinApp_backup_20251207_125903.tar.gz`

恢复命令:
```bash
cd /home/kylin
tar -xzvf /media/kylin/EED4-7516/FlyKylinApp_backup_20251207_125903.tar.gz
```

## 常用命令

```bash
# 交叉编译
wsl -d Ubuntu1 -- bash -c "cd /mnt/e/Project/FlyKylin && cmake --build build/linux-arm64-rk3566-cross --target FlyKylin -j8"

# 部署到板端
wsl -d Ubuntu1 -- sshpass -p 123456 scp /mnt/e/Project/FlyKylin/build/linux-arm64-rk3566-cross/bin/FlyKylin kylin@192.168.100.2:/home/kylin/FlyKylinApp/bin/

# 部署RKNN模型
wsl -d Ubuntu1 -- sshpass -p 123456 scp /mnt/e/Project/FlyKylin/model/rknn/open_nsfw_rk3566.rknn kylin@192.168.100.2:/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn

# 启动应用
wsl -d Ubuntu1 -- sshpass -p 123456 ssh kylin@192.168.100.2 "cd /home/kylin/FlyKylinApp && nohup ./run-flykylin.sh > /tmp/flykylin.log 2>&1 &"

# 查看日志
wsl -d Ubuntu1 -- sshpass -p 123456 ssh kylin@192.168.100.2 "tail -50 /tmp/flykylin.log"

# PC端RKNN模拟器测试
wsl -d Ubuntu1 -- bash -c "cd /mnt/e/Project/FlyKylin && python3 tools/test_rknn_pc_simulator.py"
```

## 调试历程总结

1. **初始问题**: 板端RKNN检测结果不准确（NSFW图片检测为0.017，应该是0.96）
2. **发现维度异常**: RKNN将NHWC转换为NWCH格式
3. **测试不同格式**: 确认NWCH格式 + pass_through=0 + fmt=NHWC是正确组合
4. **修复C++代码**: 更新预处理逻辑，添加HWC→WCH转换
5. **独立测试验证**: 创建独立C++测试程序确认RKNN API调用正确
6. **最终验证**: Qt应用成功检测NSFW图片，概率0.9575 ✅


---

## 二、BGE文本嵌入（语义搜索）- 已完成 ✅

### 实现概述

为RK3566板端添加了BGE-small-zh-v1.5文本嵌入模型的RKNN加速支持，用于聊天记录的语义搜索排序。

### 模型信息

| 项目 | 值 |
|------|-----|
| 模型名称 | bge-small-zh-v1.5 |
| 原始格式 | ONNX |
| RKNN格式 | bge-small-zh-v1.5.rknn |
| 模型大小 | ~47MB |
| 输入 | input_ids, attention_mask, token_type_ids [1, 128] |
| 输出 | pooler_output [1, 512], last_hidden_state [1, 128, 512] |
| 嵌入维度 | 512 |
| 最大序列长度 | 128 |

### 板端测试结果

```
=== Testing BGE RKNN Model on Board ===
Model: /home/kylin/FlyKylinApp/bin/models/bge-small-zh-v1.5.rknn
Vocab: /home/kylin/FlyKylinApp/bin/models/vocab.txt
Loaded vocab: 21127 tokens
RKNN initialized successfully!

Text: '你好'
  Output shape: (1, 128, 512)
  Embedding norm: 100.5251

Text: '今天天气怎么样'
  Output shape: (1, 128, 512)
  Embedding norm: 109.9130

=== Similarity Matrix ===
  '你好' vs '今天天气怎么样': 0.4668
  '你好' vs '我想吃火锅': 0.4341
  '今天天气怎么样' vs '我想吃火锅': 0.4085
```

### 关键文件

| 文件 | 说明 |
|------|------|
| `src/core/ai/TextEmbeddingEngine.cpp` | 文本嵌入引擎，支持ONNX和RKNN后端 |
| `src/core/services/ChatSearchService.cpp` | 聊天搜索服务，调用TextEmbeddingEngine |
| `tools/convert_bge_to_rk3566.py` | BGE模型RKNN转换脚本 |
| `tools/test_bge_rknn_board.py` | 板端BGE RKNN测试脚本 |
| `scripts/deploy-bge-model.sh` | BGE模型部署脚本 |
| `model/rknn/bge-small-zh-v1.5.rknn` | RKNN格式BGE模型 |
| `model/onnx/vocab.txt` | BGE词表文件 |

### RKNN API调用参数

```cpp
// 输入设置
rknn_input inputs[3];
inputs[0].index = 0;  // input_ids
inputs[0].pass_through = 0;
inputs[0].type = RKNN_TENSOR_INT64;
inputs[0].fmt = RKNN_TENSOR_UNDEFINED;

inputs[1].index = 1;  // attention_mask
inputs[2].index = 2;  // token_type_ids

// 输出获取
rknn_output outputs[2];
outputs[0].index = 0;  // pooler_output [1, 512]
outputs[1].index = 1;  // last_hidden_state [1, 128, 512]
outputs[0].want_float = 1;
outputs[1].want_float = 1;
```

### 注意事项

1. **模型输出**: BGE RKNN模型有两个输出，优先使用pooler_output（index=0），如果不可用则使用last_hidden_state的第一个token（CLS）
2. **词表文件**: 需要部署vocab.txt到板端models目录
3. **内存占用**: BGE模型较大（~47MB），加载时需要足够内存
4. **性能**: Transformer模型在NPU上的加速效果可能有限，如果性能不佳可考虑使用关键字搜索作为fallback

### 部署命令

```bash
# 转换模型（在WSL中运行）
python3 tools/convert_bge_to_rk3566.py

# 部署模型到板端
sshpass -p 123456 scp model/rknn/bge-small-zh-v1.5.rknn kylin@192.168.100.2:/home/kylin/FlyKylinApp/bin/models/
sshpass -p 123456 scp model/onnx/vocab.txt kylin@192.168.100.2:/home/kylin/FlyKylinApp/bin/models/

# 测试板端推理
sshpass -p 123456 ssh kylin@192.168.100.2 'python3 /home/kylin/test_bge_rknn_board.py'
```

---

## 三、编译配置

### CMake宏定义

| 宏 | 说明 |
|----|------|
| `RK3566_PLATFORM` | RK3566平台标识 |
| `FLYKYLIN_ENABLE_RKNN` | 启用RKNN支持 |
| `FLYKYLIN_ENABLE_ONNXRUNTIME` | 启用ONNX Runtime支持 |

### 编译后的符号

```bash
# 检查RKNN符号
nm build/linux-arm64-rk3566-cross/bin/FlyKylin | grep -i rknn

# 应该看到:
# RknnNsfwContext - NSFW检测
# RknnEmbeddingContext - 文本嵌入
# RknnBgeTokenizer - BGE分词器
```
