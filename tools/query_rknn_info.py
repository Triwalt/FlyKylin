#!/usr/bin/env python3
"""查询RKNN模型信息"""
from rknnlite.api import RKNNLite

rknn = RKNNLite()
rknn.load_rknn('/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn')
rknn.init_runtime()

print("SDK:", rknn.get_sdk_version())

# 尝试不同的查询方法
attrs = ['input_info', 'output_info', 'inputs', 'outputs']
for attr in attrs:
    if hasattr(rknn, attr):
        print(f"{attr}: {getattr(rknn, attr)}")

# 查看rknn对象的所有属性
print("\nRKNN attributes:")
for attr in dir(rknn):
    if not attr.startswith('_'):
        print(f"  {attr}")

rknn.release()
