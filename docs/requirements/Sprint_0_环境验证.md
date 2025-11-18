# Sprint 0: 环境验证

**周期**: 1周  
**状态**: 进行中 ⏳  
**目标**: 验证Windows开发环境，完成首次编译

## 任务清单

### 1. 环境检查 ✅
- [x] 定位Qt 6.9.3 → `D:\Qt\6.9.3\msvc2022_64`
- [x] 定位NSFW模型 → `E:\Project\tensorflow-open_nsfw\open_nsfw.onnx`
- [x] 定位BGE模型 → `E:\Project\tensorflow-open_nsfw\onnx_models\`
- [x] 确认CMake 3.28.1可用

### 2. 下载依赖 🔄
- [ ] 下载ONNX Runtime C++ API 1.23.2
  - URL: https://github.com/microsoft/onnxruntime/releases/tag/v1.23.2
  - 文件: `onnxruntime-win-x64-1.23.2.zip`
  - 解压到: `e:\Project\FlyKylin\3rdparty\onnxruntime`

- [ ] 安装Protobuf
  - 命令: `choco install protoc`
  - 或使用: vcpkg

### 3. 首次编译 ⏳
- [ ] 在VS Developer Command Prompt中运行配置脚本
- [ ] 创建build目录并配置CMake
- [ ] 编译Debug版本
- [ ] 验证FlyKylin.exe生成

### 4. ONNX Runtime测试 ⏳
- [ ] 创建简单的ONNX加载测试程序
- [ ] 加载NSFW模型
- [ ] 执行一次推理验证
- [ ] 确认DirectML可用（GPU加速）

## 验收标准

- ✅ 所有依赖工具已安装
- ✅ CMake配置成功，无错误
- ✅ 项目编译成功，生成FlyKylin.exe
- ✅ ONNX Runtime能成功加载NSFW模型
- ✅ 能执行一次NSFW检测推理

## 阻塞问题

当前阻塞：
1. ONNX Runtime C++ API未下载
2. Protobuf未安装
3. 未在VS Developer Command Prompt中

## 下一步

完成Sprint 0后，进入Sprint 1开发P2P通信基础。
