# FlyKylin Tools

工具目录，包含开发、测试和部署相关的脚本。

## 目录结构

```
tools/
├── developer/              # 开发环境配置工具
├── agent/                  # Agent 工具 (预留)
├── user/                   # 用户工具 (预留)
├── calib_data/             # RKNN 校准数据
├── calibration_npy/        # 校准用 NPY 文件
├── test_data/              # 测试数据
└── accuracy_analysis/      # 精度分析结果
```

## RKNN 相关工具

### 模型转换

| 脚本 | 用途 |
|------|------|
| `convert_nsfw_to_rk3566.py` | ONNX → RKNN 模型转换 (RK3566) |

### 测试工具

| 脚本 | 平台 | 用途 |
|------|------|------|
| `test_rknn_pc_simulator.py` | PC (WSL) | PC端 RKNN 模拟器测试 (推荐) |
| `test_rknn_board_final.py` | RK3566 | 板端 Python 验证脚本 |
| `test_rknn_standalone.cpp` | RK3566 | 板端独立 C++ 测试程序 |
| `test_cpp_preprocess.py` | PC | C++ 预处理逻辑验证 |
| `generate_nwch_npy.py` | PC | 生成 NWCH 格式测试数据 |

### 使用示例

```bash
# PC端 RKNN 模拟器测试 (WSL)
cd /mnt/e/Project/FlyKylin
python3 tools/test_rknn_pc_simulator.py

# 模型转换
python3 tools/convert_nsfw_to_rk3566.py

# 板端测试 (需要先传输到板端)
scp tools/test_rknn_board_final.py kylin@192.168.100.2:/tmp/
ssh kylin@192.168.100.2 "python3 /tmp/test_rknn_board_final.py"
```

## 开发环境工具

详见 `developer/README.md`

```powershell
# Windows: 启动 VS 开发命令提示符
.\tools\developer\start-vsdevcmd.ps1

# 验证环境
.\tools\developer\verify-environment.ps1

# 安装依赖
.\tools\developer\install-onnx-runtime.ps1
.\tools\developer\install-protobuf.ps1
```

## 数据文件

| 文件 | 用途 |
|------|------|
| `calibration_data.txt` | RKNN 量化校准数据列表 |
| `dataset.txt` | 数据集文件列表 |

## 相关文档

- [RKNN 调试总结](../docs/RKNN_DEBUG_SUMMARY.md)
- [部署指南](../docs/DEPLOYMENT_GUIDE.md)
- [开发环境配置](developer/README.md)
