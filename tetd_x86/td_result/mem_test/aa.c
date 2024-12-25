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

void avx512_memcpy(void *dest, const void *src, size_t n) {
    __m512i *d = dest;
    const __m512i *s = src;
    size_t i;

    for (i = 0; i < n / 64; i++) {
        _mm512_storeu_si512(&d[i], _mm512_loadu_si512(&s[i]));
    }

    // 处理剩余字节
    for (i *= 64; i < n; i++) {
        ((char *)dest)[i] = ((const char *)src)[i];
    }
}
unsigned long long aes_encrypt(unsigned char *input, unsigned char *output, unsigned char *key);
int main() {
    char *src;
    char *dest;
    char *dest2;

    // 分配4K对齐的内存
    if (posix_memalign((void**)&src, 4096, 4096) != 0 || 
        posix_memalign((void**)&dest, 4096, 4096) != 0 ||
	posix_memalign((void**)&dest2, 4096, 4096) != 0) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // 初始化源数据
    for (int i = 0; i < 4096; i++) {
        src[i] = (char)(i % 256);
    }
    
    unsigned char key[16] = {
        0x2b, 0x7e, 0x15, 0x16, 
        0x28, 0xae, 0xd2, 0xa6, 
        0xab, 0xf7, 0x32, 0x29, 
        0x1a, 0xc1, 0x30, 0x08
    };
    uint64_t start, end,t_all;
    unsigned long t1,t2,t_aes=0;
    // 开始拷贝时间测量
    //start = read_tsc();
    int i = 0;
    for(i=0;i<32;i++){
        //t1 = read_tsc();
	t1 = aes_encrypt(src+i*128, dest+i*128, key);
	//t2 = read_tsc();
	t_aes = t_aes+t1;
    }
    
    start = read_tsc();
    avx512_memcpy(dest2, src, 4096); // 使用 AVX-512 拷贝
    end = read_tsc();
    t_all=end-start;
    // 计算时间（以纳秒为单位）
    double frequency = 2.4e9; // 2.4 GHz
    //double time_ns = (end - start) / (frequency / 1e9);
    //printf("Copy time (ns): %f\n", time_ns);
    //printf("Copy time (ns): %ld\n", (t_all)*5/12);
    printf("Copy time (ns): %ld\n", (t_aes)*5/12);
    //printf("Copy time (ns): %ld\n", (t_all+t_aes)*5/12);

    // 释放内存
    free(src);
    free(dest);

    return 0;
}
__asm__ (
    ".section .data\n"
    ".comm buf, 16, 16\n"
    
    ".section .text\n"
    
    ".global aes_encrypt\n"
    "aes_encrypt:\n"
    "    movdqu (%rdi), %xmm0\n"  // Load input into %xmm0
    "    movdqu (%rdx), %xmm5\n"  // Load key into %xmm5
    "    pxor   %xmm2, %xmm2\n"   // Clear %xmm2

    "    aeskeygenassist $1, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm6\n"
    "    aeskeygenassist $2, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm7\n"
    "    aeskeygenassist $4, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm8\n"
    "    aeskeygenassist $8, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm9\n"
    "    aeskeygenassist $16, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm10\n"
    "    aeskeygenassist $32, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm11\n"
    "    aeskeygenassist $64, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm12\n"
    "    aeskeygenassist $128, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm13\n"
    "    aeskeygenassist $27, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm14\n"
    "    aeskeygenassist $54, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm15\n"

    "encrypt:\n"
    "rdtsc\n"
    "shl $32,%rdx\n"
    "or %rdx,%rax\n"
    "mov %rax,%r8\n"
    "    pxor       %xmm5,  %xmm0\n"
    "    aesenc     %xmm6,  %xmm0\n"
    "    aesenc     %xmm7,  %xmm0\n"
    "    aesenc     %xmm8,  %xmm0\n"
    "    aesenc     %xmm9,  %xmm0\n"
    "    aesenc     %xmm10, %xmm0\n"
    "    aesenc     %xmm11, %xmm0\n"
    "    aesenc     %xmm12, %xmm0\n"
    "    aesenc     %xmm13, %xmm0\n"
    "    aesenc     %xmm14, %xmm0\n"
    "    aesenclast %xmm15, %xmm0\n"
    "rdtsc\n"
    "shl $32,%rdx\n"
    "or %rdx,%rax\n"
    "sub %r8,%rax\n"
    
    
    "    movdqu %xmm0, (%rsi)\n"  // Store output from %xmm0
    "    ret\n"

    "key_combine:\n"
    "    pshufd $0b11111111, %xmm1, %xmm1\n"
    "    shufps $0b00010000, %xmm0, %xmm2\n"
    "    pxor   %xmm2, %xmm0\n"
    "    shufps $0b10001100, %xmm0, %xmm2\n"
    "    pxor   %xmm2, %xmm0\n"
    "    pxor   %xmm1, %xmm0\n"
    "    ret\n"
);
