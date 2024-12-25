#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#define PLAIN_TEXT_LENGTH 4096 // 定义明文长度为4096字节

static inline uint64_t read_tsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

void sha256_hash(const uint8_t *data, size_t length, uint8_t *hash) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, length);
    SHA256_Final(hash, &sha256);
}

int main() {
    uint8_t *data = NULL;
    uint8_t hash[SHA256_DIGEST_LENGTH]; // SHA-256 输出长度为 32 字节

    // 分配4KB对齐的内存
    if (posix_memalign((void**)&data, 4096, PLAIN_TEXT_LENGTH) != 0) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // 初始化数据（可以替换为你的输入）
    for (size_t i = 0; i < PLAIN_TEXT_LENGTH; i++) {
        data[i] = (uint8_t)(i % 256);
    }

    // 计算 SHA-256 哈希的时间
    uint64_t start = read_tsc();
    sha256_hash(data, PLAIN_TEXT_LENGTH, hash);
    uint64_t end = read_tsc();

    // 计算时间（以纳秒为单位）
    //double frequency = 2.4e9; // 假设 CPU 频率为 2.4 GHz
    unsigned long time_ns =(end - start)*5/12;

    // 打印哈希值
    printf("SHA-256 Hash: ");
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    // 打印时间
    printf("SHA-256 computation time (ns): %lu\n", time_ns);

    free(data); // 释放分配的内存

    return 0;
}

