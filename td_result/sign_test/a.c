#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/sha.h>

#define PLAIN_TEXT_LENGTH 512 // 定义明文长度为4096字节

static inline uint64_t read_tsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

// 使用 SHA-256 计算哈希
void sha256(const uint8_t *data, size_t length, uint8_t *hash) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, length);
    SHA256_Final(hash, &ctx);
}

// 生成 RSA 密钥对
RSA *generate_rsa_keypair(int bits) {
    RSA *rsa = RSA_new();
    BIGNUM *bn = BN_new();
    BN_set_word(bn, RSA_F4);
    RSA_generate_key_ex(rsa, bits, bn, NULL);
    BN_free(bn);
    return rsa;
}

// RSA 签名
int rsa_sign(RSA *rsa, const uint8_t *hash, size_t hash_len, uint8_t **signature, unsigned int *sig_len) {
    *signature = (uint8_t *)malloc(RSA_size(rsa));
    return RSA_sign(NID_sha256, hash, hash_len, *signature, sig_len, rsa);
}

// RSA 验证签名
int rsa_verify(RSA *rsa, const uint8_t *hash, size_t hash_len, uint8_t *signature, unsigned int sig_len) {
    return RSA_verify(NID_sha256, hash, hash_len, signature, sig_len, rsa);
}

int main() {
    uint8_t *data;
    uint8_t hash[SHA256_DIGEST_LENGTH];
    uint8_t *signature = NULL;
    unsigned int sig_len;

    // 生成 3072 位的 RSA 密钥对
    RSA *rsa = generate_rsa_keypair(3072);

    // 分配4KB对齐的内存
    if (posix_memalign((void**)&data, 4096, PLAIN_TEXT_LENGTH) != 0) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // 初始化数据
    for (size_t i = 0; i < PLAIN_TEXT_LENGTH; i++) {
        data[i] = (uint8_t)(i % 256);
    }

    // SHA-256 计算
    uint64_t start = read_tsc();
    sha256(data, PLAIN_TEXT_LENGTH, hash);
    uint64_t end = read_tsc();
    unsigned long sha_time_ns = (end - start) * 5 / 12;

    // RSA 签名
    start = read_tsc();
    rsa_sign(rsa, hash, SHA256_DIGEST_LENGTH, &signature, &sig_len);
    end = read_tsc();
    unsigned long sign_time_ns = (end - start) * 5 / 12;

    // RSA 验证签名
    start = read_tsc();
    int verify_result = rsa_verify(rsa, hash, SHA256_DIGEST_LENGTH, signature, sig_len);
    end = read_tsc();
    unsigned long verify_time_ns = (end - start) * 5 / 12;

    if (verify_result == 1) {
        printf("Signature verification succeeded for %d bytes.\n", PLAIN_TEXT_LENGTH);
    } else {
        printf("Signature verification failed for %d bytes.\n", PLAIN_TEXT_LENGTH);
    }

    // 输出时间到文件（格式适合 Excel）
    FILE *file = fopen("time.txt", "a");
    if (file != NULL) {
        fprintf(file, "%lu\t%lu\t%lu\n", sha_time_ns, sign_time_ns, verify_time_ns);
        fclose(file);
    } else {
        fprintf(stderr, "Error opening file for writing\n");
    }

    // 释放资源
    free(signature);
    free(data);
    RSA_free(rsa);

    return 0;
}

