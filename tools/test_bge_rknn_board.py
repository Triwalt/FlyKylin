#!/usr/bin/env python3
"""
测试板端BGE RKNN模型推理

在板端运行此脚本来验证BGE文本嵌入模型是否正常工作
"""

import os
import sys
import numpy as np

try:
    from rknnlite.api import RKNNLite
except ImportError:
    print("Error: rknnlite not installed")
    print("This script should be run on the RK3566 board")
    sys.exit(1)

# 配置
MODEL_PATH = "/home/kylin/FlyKylinApp/bin/models/bge-small-zh-v1.5.rknn"
VOCAB_PATH = "/home/kylin/FlyKylinApp/bin/models/vocab.txt"
MAX_SEQ_LEN = 128
EMBEDDING_DIM = 512


class SimpleTokenizer:
    """简单的字符级tokenizer"""
    def __init__(self, vocab_path):
        self.token_to_id = {}
        self.cls_id = -1
        self.sep_id = -1
        self.pad_id = -1
        self.unk_id = -1
        
        with open(vocab_path, 'r', encoding='utf-8') as f:
            for idx, line in enumerate(f):
                token = line.strip()
                if not token:
                    continue
                self.token_to_id[token] = idx
                if token == '[CLS]':
                    self.cls_id = idx
                elif token == '[SEP]':
                    self.sep_id = idx
                elif token == '[PAD]':
                    self.pad_id = idx
                elif token == '[UNK]':
                    self.unk_id = idx
        
        print(f"Loaded vocab: {len(self.token_to_id)} tokens")
        print(f"Special tokens: CLS={self.cls_id}, SEP={self.sep_id}, PAD={self.pad_id}, UNK={self.unk_id}")
    
    def encode(self, text, max_length):
        tokens = [self.cls_id]
        
        for char in text:
            if char.isspace():
                continue
            token_id = self.token_to_id.get(char, self.unk_id)
            tokens.append(token_id)
            if len(tokens) >= max_length - 1:
                break
        
        if len(tokens) < max_length:
            tokens.append(self.sep_id)
        
        # Padding
        attention_mask = [1] * len(tokens)
        while len(tokens) < max_length:
            tokens.append(self.pad_id)
            attention_mask.append(0)
        
        return tokens[:max_length], attention_mask[:max_length]


def test_bge_inference():
    print("=== Testing BGE RKNN Model on Board ===")
    
    # Check files
    if not os.path.exists(MODEL_PATH):
        print(f"Error: Model not found: {MODEL_PATH}")
        return False
    
    if not os.path.exists(VOCAB_PATH):
        print(f"Error: Vocab not found: {VOCAB_PATH}")
        return False
    
    print(f"Model: {MODEL_PATH}")
    print(f"Vocab: {VOCAB_PATH}")
    
    # Load tokenizer
    tokenizer = SimpleTokenizer(VOCAB_PATH)
    
    # Initialize RKNN
    print("\nInitializing RKNNLite...")
    rknn = RKNNLite(verbose=False)
    
    ret = rknn.load_rknn(MODEL_PATH)
    if ret != 0:
        print(f"Load RKNN failed: {ret}")
        return False
    
    ret = rknn.init_runtime()
    if ret != 0:
        print(f"Init runtime failed: {ret}")
        rknn.release()
        return False
    
    print("RKNN initialized successfully!")
    
    # Test texts
    test_texts = [
        "你好",
        "今天天气怎么样",
        "我想吃火锅",
        "Hello world",
    ]
    
    print("\n=== Running Inference ===")
    embeddings = []
    
    for text in test_texts:
        print(f"\nText: '{text}'")
        
        # Tokenize
        input_ids, attention_mask = tokenizer.encode(text, MAX_SEQ_LEN)
        token_type_ids = [0] * MAX_SEQ_LEN
        
        # Convert to numpy arrays
        input_ids = np.array([input_ids], dtype=np.int64)
        attention_mask = np.array([attention_mask], dtype=np.int64)
        token_type_ids = np.array([token_type_ids], dtype=np.int64)
        
        print(f"  Input IDs shape: {input_ids.shape}")
        print(f"  First 10 tokens: {input_ids[0][:10]}")
        
        # Run inference
        try:
            outputs = rknn.inference(inputs=[input_ids, attention_mask, token_type_ids])
            
            if outputs and len(outputs) > 0:
                embedding = outputs[0]
                print(f"  Output shape: {embedding.shape}")
                print(f"  Embedding norm: {np.linalg.norm(embedding):.4f}")
                print(f"  First 5 values: {embedding.flatten()[:5]}")
                embeddings.append(embedding.flatten())
            else:
                print("  Error: Empty output")
        except Exception as e:
            print(f"  Error: {e}")
    
    # Calculate similarity
    if len(embeddings) >= 2:
        print("\n=== Similarity Matrix ===")
        for i in range(len(embeddings)):
            for j in range(i + 1, len(embeddings)):
                sim = np.dot(embeddings[i], embeddings[j]) / (
                    np.linalg.norm(embeddings[i]) * np.linalg.norm(embeddings[j])
                )
                print(f"  '{test_texts[i]}' vs '{test_texts[j]}': {sim:.4f}")
    
    rknn.release()
    print("\n=== Test Complete ===")
    return True


if __name__ == "__main__":
    success = test_bge_inference()
    sys.exit(0 if success else 1)
