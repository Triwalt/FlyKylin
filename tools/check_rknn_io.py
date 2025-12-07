#!/usr/bin/env python3
"""检查RKNN模型的输入输出信息"""
import sys
from rknnlite.api import RKNNLite

rknn = RKNNLite(verbose=True)

print("Load model")
if rknn.load_rknn("/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn") != 0:
    print("FAILED"); sys.exit(1)

print("Init runtime")
if rknn.init_runtime(core_mask=0) != 0:
    rknn.release(); print("FAILED"); sys.exit(1)

# 查看所有属性
print("\n=== RKNN attributes ===")
for attr in dir(rknn):
    if not attr.startswith('_'):
        try:
            val = getattr(rknn, attr)
            if not callable(val):
                print(f"  {attr}: {val}")
        except:
            pass

# 尝试获取SDK版本
try:
    print("\n=== SDK Version ===")
    print(rknn.get_sdk_version())
except Exception as e:
    print(f"Error: {e}")

rknn.release()
print("\nDone!")
