# RKNN NSFW检测调试总结

## 最终状态 (2025-12-07) - 问题已完全解决！✅

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

## 已修改文件
- `src/core/ai/NSFWDetector.cpp` - RKNN推理代码（已修复）

## 验证结果
端到端测试通过：
- porn.jpg: NSFW=0.9575 (期望~0.96) ✅
- pig.jpg: NSFW=0.0003 (期望~0.0) ✅

### C++预处理代码关键逻辑
```cpp
// 从 HWC [h][w][c] 转换为 WCH [w][c][h]
for (int h = 0; h < height; ++h) {
    for (int w = 0; w < width; ++w) {
        // RGB -> BGR + 均值减法
        float bVal = b - 104.0f;
        float gVal = g - 117.0f;
        float rVal = r - 123.0f;
        
        // 存储为 WCH: index = w * (C * H) + c * H + h
        size_t baseIdx = w * 3 * height;
        inputData[baseIdx + 0 * height + h] = bVal;  // B
        inputData[baseIdx + 1 * height + h] = gVal;  // G
        inputData[baseIdx + 2 * height + h] = rVal;  // R
    }
}
```

---

## 问题背景

在RK3566板端整合RKNN加速NSFW检测时，检测结果严重不准确。

## 测试环境

- **板端**: RK3566, IP: 192.168.100.2, 用户: kylin, 密码: 123456
- **板端项目目录**: `/home/kylin/FlyKylinApp`
- **RKNN库版本**: 2.3.2
- **测试图片**:
  - NSFW图片: `E:\Download\porn.jpg` (PC) / `/mnt/e/Download/porn.jpg` (WSL)
  - 正常图片: `E:\Pictures\pig.jpg` (PC) / `/mnt/e/Pictures/pig.jpg` (WSL)

## 参考结果 (PC端ONNX)

| 图片 | SFW | NSFW |
|------|-----|------|
| porn.jpg | 0.036 | **0.964** |
| pig.jpg | 应该接近1.0 | 应该接近0.0 |

## 当前问题

板端RKNN检测 porn.jpg 结果: NSFW=0.017 (严重错误，应该是0.96)

## 问题根因分析

### 1. 预处理不一致
open_nsfw模型期望:
- 输入格式: NHWC [1, 224, 224, 3]
- 颜色空间: **BGR** (不是RGB)
- 预处理: 减去均值 [B=104, G=117, R=123]
- 数据类型: float32

### 2. RKNN mean_values配置失败
```python
# 尝试设置mean_values报错
rknn.config(mean_values=[[104, 117, 123]])  # 报错: expect 224
```
RKNN对NHWC格式的mean_values期望长度为H(224)而不是C(3)

### 3. float32输入导致系统卡死
在C++中使用float32输入会导致板端系统级卡死

### 4. 模型转换后维度异常
转换后输入维度变成 `[1, 224, 3, 224]` 而不是 `[1, 224, 224, 3]`

## 已尝试的方案

| 方案 | 结果 |
|------|------|
| uint8 RGB输入 | 不卡死，但结果不准确 |
| float32 BGR + 均值减法 | 系统卡死 |
| 在ONNX中添加预处理层再转换 | 维度错乱，结果仍不准确 |
| 设置mean_values | 配置报错 |

## 待测试方案

### 方案A: 板端Python直接测试 (优先)
在板端用Python测试不同输入格式，排除C++代码问题:
1. uint8 RGB
2. uint8 BGR  
3. float32 BGR + 均值减法

### 方案B: C++中使用uint8 BGR + 近似均值处理
避免float32，使用整数运算近似均值减法

### 方案C: INT8量化模型
使用校准数据集进行量化，让量化过程学习预处理

## 关键文件

- `src/core/ai/NSFWDetector.cpp` - C++ RKNN推理代码（已修复）
- `tools/test_rknn_pc_simulator.py` - PC端RKNN模拟器测试（推荐）
- `tools/test_rknn_board_final.py` - 板端最终验证脚本
- `tools/test_cpp_preprocess.py` - C++预处理逻辑验证
- `tools/generate_nwch_npy.py` - 生成NWCH格式测试数据
- `tools/convert_nsfw_to_rk3566.py` - 模型转换脚本
- `model/onnx/open_nsfw.onnx` - 原始ONNX模型
- `model/rknn/open_nsfw_rk3566.rknn` - 当前RKNN模型

## 常用命令

```bash
# 交叉编译
wsl -d Ubuntu1 -- bash -c "cd /mnt/e/Project/FlyKylin && cmake --build build/linux-arm64-rk3566-cross --target FlyKylin -j8"

# 部署到板端
wsl -d Ubuntu1 -- sshpass -p 123456 scp /mnt/e/Project/FlyKylin/build/linux-arm64-rk3566-cross/bin/FlyKylin kylin@192.168.100.2:/home/kylin/FlyKylinApp/bin/

# 部署RKNN模型
wsl -d Ubuntu1 -- sshpass -p 123456 scp /mnt/e/Project/FlyKylin/model/rknn/open_nsfw_rk3566.rknn kylin@192.168.100.2:/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn

# 启动应用
wsl -d Ubuntu1 -- sshpass -p 123456 ssh kylin@192.168.100.2 "cd /home/kylin/FlyKylinApp && ./run-flykylin.sh"

# 查看日志
wsl -d Ubuntu1 -- sshpass -p 123456 ssh kylin@192.168.100.2 "tail -50 /tmp/flykylin.log"

# PC端测试ONNX
wsl -d Ubuntu1 -- bash -c "cd /mnt/e/Project/FlyKylin && python3 tools/test_rknn_pc.py"
```

## 下一步行动

1. 创建板端Python测试脚本，测试不同输入格式
2. 将测试图片传输到板端
3. 在板端运行测试，记录每种输入格式的输出
4. 根据测试结果确定正确的输入格式
5. 修改C++代码或重新转换模型
