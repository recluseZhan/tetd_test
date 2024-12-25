#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>

static inline uint64_t read_tsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

void avx512_memcpy_optimized(void *dest, const void *src, size_t n) {
    size_t i;

    // 处理主要的拷贝
    for (i = 0; i < n / 64; i++) {
        // 预取数据以提高缓存命中率
        _mm_prefetch((const char *)src + (i + 8) * 64, _MM_HINT_T0);
        _mm512_storeu_si512(&((__m512i *)dest)[i], _mm512_loadu_si512(&((const __m512i *)src)[i]));
    }

    // 处理剩余字节
    for (i *= 64; i < n; i++) {
        ((char *)dest)[i] = ((const char *)src)[i];
    }
}

int main() {
    size_t sizes[] = {2*1024*1024}; 
    char *src[1], *dest[1];

    // 分配2M对齐的内存
    for (int j = 0; j < 1; j++) {
        size_t alignment = sizes[0];
        if (posix_memalign((void**)&src[j], alignment, sizes[j]) != 0 || 
            posix_memalign((void**)&dest[j], alignment, sizes[j]) != 0) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }

        // 初始化源数据
        for (size_t i = 0; i < sizes[j]; i++) {
            src[j][i] = (char)(i % 256);
        }

        uint64_t start, end;

        // 开始拷贝时间测量
        start = read_tsc();
        avx512_memcpy_optimized(dest[j], src[j], sizes[j]); // 使用优化的 AVX-512 拷贝
        end = read_tsc();

        // 计算时间（以纳秒为单位）
        //double frequency = 2.4e9; // 2.4 GHz
        //double time_ns = (end - start) / (frequency / 1e9);
        unsigned long time_ns = (end-start)*5/12;
        printf("Copy time for %zu bytes (ns): %ld\n", sizes[j], time_ns);

        // 释放内存
        free(src[j]);
        free(dest[j]);
    }

    return 0;
}

