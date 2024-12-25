#include <stdio.h>
#include <stdint.h>
#include <string.h>
unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}
void sha256_hash(const uint8_t *data, size_t len, uint8_t *hash) {
    asm volatile (
        "mov $0x101, %%eax\n"          // EAX = 0x101 (SHA instruction set identifier)
        "movq %[data], %%rsi\n"        // RSI = data
        "movq %[len], %%rcx\n"         // RCX = len
        "movq %[hash], %%rdi\n"        // RDI = hash
        "rep sha256rnds2\n"             // 执行 SHA-256 算法
        :
        : [data] "r" (data), [len] "r" (len), [hash] "r" (hash)
        : "rax", "rsi", "rcx", "rdi", "memory"
    );
}

int main() {
    const char *data = "Hello, world!";
    size_t len = strlen(data);
    uint8_t hash[32];  // SHA-256 的哈希值长度为 32 字节

    // 调用 SHA-256 哈希函数
    sha256_hash((const uint8_t *)data, len, hash);

    // 输出哈希值
    printf("SHA-256 hash: ");
    for (int i = 0; i < 32; ++i) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    return 0;
}

