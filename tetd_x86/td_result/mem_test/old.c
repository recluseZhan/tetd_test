#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>
#include <openssl/aes.h>

static inline uint64_t read_tsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

void avx512_memcpy_optimized(void *dest, const void *src, size_t n) {
    size_t i;

    for (i = 0; i < n / 64; i++) {
        _mm_prefetch((const char *)src + (i + 8) * 64, _MM_HINT_T0);
        _mm512_storeu_si512(&((__m512i *)dest)[i], _mm512_loadu_si512(&((const __m512i *)src)[i]));
    }

    for (i *= 64; i < n; i++) {
        ((char *)dest)[i] = ((const char *)src)[i];
    }
}

void aes_encrypt(const unsigned char *input, unsigned char *output, const unsigned char *key) {
    AES_KEY encrypt_key;
    AES_set_encrypt_key(key, 128, &encrypt_key);
    AES_encrypt(input, output, &encrypt_key);
}

void aes_encrypt_data(const unsigned char *input, unsigned char *output, size_t size, const unsigned char *key) {
    for (size_t i = 0; i < size; i += AES_BLOCK_SIZE) {
        aes_encrypt(input + i, output + i, key);
    }
}

int main() {
    size_t sizes[] = {4096, 2 * 1024 * 1024}; // 4K 和 2M
    char *src[2], *dest[2];
    unsigned char key[16] = "0123456789abcdef"; // AES 128-bit key
    unsigned char *encrypted[2];

    // 分配2M对齐的内存
    for (int j = 0; j < 2; j++) {
        size_t alignment = (j == 1) ? 2 * 1024 * 1024 : 4096;
        if (posix_memalign((void**)&src[j], alignment, sizes[j]) != 0 || 
            posix_memalign((void**)&dest[j], alignment, sizes[j]) != 0) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }

        // 初始化源数据
        for (size_t i = 0; i < sizes[j]; i++) {
            src[j][i] = (char)(i % 256);
        }

        // 分配用于存储密文
        encrypted[j] = malloc(sizes[j]);
        if (!encrypted[j]) {
            fprintf(stderr, "Memory allocation for encrypted data failed\n");
            return 1;
        }

        uint64_t start, end;

        // 开始加密时间测量
        start = read_tsc();
        aes_encrypt_data((const unsigned char *)src[j], encrypted[j], sizes[j], key); // 使用 AES 加密
        end = read_tsc();

        double frequency = 2.4e9; // 2.4 GHz
        double encrypt_time_ns = (end - start) / (frequency / 1e9);
        printf("Encryption time for %zu bytes (ns): %f\n", sizes[j], encrypt_time_ns);

        // 开始拷贝时间测量
        start = read_tsc();
        avx512_memcpy_optimized(dest[j], encrypted[j], sizes[j]); // 拷贝密文
        end = read_tsc();

        double copy_time_ns = (end - start) / (frequency / 1e9);
        printf("Copy time for %zu bytes (ns): %f\n", sizes[j], copy_time_ns);

        // 释放内存
        free(encrypted[j]);
        free(src[j]);
        free(dest[j]);
    }

    return 0;
}

