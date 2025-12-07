/**
 * 独立的RKNN测试程序
 * 用于精确定位RKNN调用导致系统卡死的问题
 * 
 * 编译命令 (在板端):
 * g++ -o test_rknn_standalone test_rknn_standalone.cpp -I/usr/include -L/usr/lib -lrknnrt -std=c++17
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <fstream>
#include <algorithm>

#include <rknn_api.h>

#define LOG(fmt, ...) printf("[TEST] " fmt "\n", ##__VA_ARGS__); fflush(stdout)

const char* MODEL_PATH = "/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn";
const char* NPY_PATH = "/home/kylin/Downloads/porn_nwch.npy";

// 简单的npy加载器 (只支持float32)
bool load_npy(const char* path, std::vector<float>& data, std::vector<int>& shape) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        LOG("Failed to open npy file: %s", path);
        return false;
    }
    
    // 跳过npy头部 (简化处理，假设头部是128字节)
    char header[128];
    size_t header_len = fread(header, 1, 128, f);
    
    // 找到实际数据开始位置
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    
    // 假设是 [1, 224, 3, 224] float32 = 1*224*3*224*4 = 602112 bytes
    long data_size = 1 * 224 * 3 * 224 * sizeof(float);
    fseek(f, file_size - data_size, SEEK_SET);
    
    data.resize(1 * 224 * 3 * 224);
    size_t read = fread(data.data(), sizeof(float), data.size(), f);
    fclose(f);
    
    if (read != data.size()) {
        LOG("Failed to read npy data, expected %zu, got %zu", data.size(), read);
        return false;
    }
    
    shape = {1, 224, 3, 224};
    LOG("Loaded npy: %zu floats, shape [1, 224, 3, 224]", data.size());
    return true;
}

int main(int argc, char** argv) {
    LOG("=== RKNN Standalone Test ===");
    LOG("Model: %s", MODEL_PATH);
    LOG("Input: %s", NPY_PATH);
    
    // Step 1: 加载模型文件
    LOG("Step 1: Loading model file...");
    FILE* f = fopen(MODEL_PATH, "rb");
    if (!f) {
        LOG("ERROR: Failed to open model file");
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long model_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    std::vector<char> model_data(model_size);
    fread(model_data.data(), 1, model_size, f);
    fclose(f);
    LOG("Model loaded: %ld bytes", model_size);
    
    // Step 2: 初始化RKNN
    LOG("Step 2: Initializing RKNN...");
    rknn_context ctx = 0;
    int ret = rknn_init(&ctx, model_data.data(), model_size, 0, nullptr);
    if (ret != RKNN_SUCC) {
        LOG("ERROR: rknn_init failed: %d", ret);
        return 1;
    }
    LOG("RKNN initialized, ctx=%lu", (unsigned long)ctx);
    
    // Step 3: 查询输入输出信息
    LOG("Step 3: Querying I/O info...");
    rknn_input_output_num io_num;
    memset(&io_num, 0, sizeof(io_num));
    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret != RKNN_SUCC) {
        LOG("ERROR: rknn_query IN_OUT_NUM failed: %d", ret);
        rknn_destroy(ctx);
        return 1;
    }
    LOG("Inputs: %d, Outputs: %d", io_num.n_input, io_num.n_output);
    
    rknn_tensor_attr input_attr;
    memset(&input_attr, 0, sizeof(input_attr));
    input_attr.index = 0;
    ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &input_attr, sizeof(input_attr));
    if (ret != RKNN_SUCC) {
        LOG("ERROR: rknn_query INPUT_ATTR failed: %d", ret);
        rknn_destroy(ctx);
        return 1;
    }
    LOG("Input dims: [%d, %d, %d, %d], fmt=%d, type=%d, n_elems=%d",
        input_attr.dims[0], input_attr.dims[1], input_attr.dims[2], input_attr.dims[3],
        input_attr.fmt, input_attr.type, input_attr.n_elems);
    
    // Step 4: 加载输入数据
    LOG("Step 4: Loading input data...");
    std::vector<float> input_data;
    std::vector<int> input_shape;
    if (!load_npy(NPY_PATH, input_data, input_shape)) {
        LOG("ERROR: Failed to load input data");
        rknn_destroy(ctx);
        return 1;
    }
    LOG("Input data range: [%.2f, %.2f]", 
        *std::min_element(input_data.begin(), input_data.end()),
        *std::max_element(input_data.begin(), input_data.end()));
    
    // Step 5: 设置输入
    LOG("Step 5: Setting input (pass_through=0, fmt=NHWC, type=FLOAT32)...");
    rknn_input input;
    memset(&input, 0, sizeof(input));
    input.index = 0;
    input.pass_through = 0;
    input.type = RKNN_TENSOR_FLOAT32;
    input.fmt = RKNN_TENSOR_NHWC;
    input.size = input_data.size() * sizeof(float);
    input.buf = input_data.data();
    
    LOG("Calling rknn_inputs_set...");
    ret = rknn_inputs_set(ctx, 1, &input);
    if (ret != RKNN_SUCC) {
        LOG("ERROR: rknn_inputs_set failed: %d", ret);
        rknn_destroy(ctx);
        return 1;
    }
    LOG("rknn_inputs_set OK");
    
    // Step 6: 运行推理
    LOG("Step 6: Running inference...");
    LOG("Calling rknn_run...");
    ret = rknn_run(ctx, nullptr);
    if (ret != RKNN_SUCC) {
        LOG("ERROR: rknn_run failed: %d", ret);
        rknn_destroy(ctx);
        return 1;
    }
    LOG("rknn_run OK");
    
    // Step 7: 获取输出
    LOG("Step 7: Getting output...");
    rknn_output output;
    memset(&output, 0, sizeof(output));
    output.want_float = 1;
    output.is_prealloc = 0;
    output.index = 0;
    
    LOG("Calling rknn_outputs_get...");
    ret = rknn_outputs_get(ctx, 1, &output, nullptr);
    if (ret != RKNN_SUCC) {
        LOG("ERROR: rknn_outputs_get failed: %d", ret);
        rknn_destroy(ctx);
        return 1;
    }
    LOG("rknn_outputs_get OK, size=%d", output.size);
    
    // Step 8: 解析输出
    LOG("Step 8: Parsing output...");
    float* out_data = (float*)output.buf;
    if (out_data) {
        float sfw = out_data[0];
        float nsfw = out_data[1];
        LOG("Raw output: sfw=%.4f, nsfw=%.4f, sum=%.4f", sfw, nsfw, sfw + nsfw);
        
        // 如果需要softmax
        if (std::abs(sfw + nsfw - 1.0f) > 0.1f) {
            float max_val = std::max(sfw, nsfw);
            float exp_sfw = std::exp(sfw - max_val);
            float exp_nsfw = std::exp(nsfw - max_val);
            float sum_exp = exp_sfw + exp_nsfw;
            LOG("After softmax: sfw=%.4f, nsfw=%.4f", exp_sfw/sum_exp, exp_nsfw/sum_exp);
        }
    }
    
    // Step 9: 释放资源
    LOG("Step 9: Releasing resources...");
    rknn_outputs_release(ctx, 1, &output);
    rknn_destroy(ctx);
    
    LOG("=== Test Complete ===");
    return 0;
}
