// bignum.h
#ifndef BIGNUM_H
#define BIGNUM_H

#include <stddef.h>
#include <stdint.h>
#include <openssl/bn.h>

// 我们把 BIGNUM* 当成 bignum
typedef BIGNUM* bignum;

// 构造与析构
bignum bn_new(void);
void   bn_free(bignum a);

// 从/到 大端字节数组
bignum bn_from_bytes_be(const uint8_t *in, size_t len);
int    bn_to_bytes_be(const bignum a, uint8_t *out, size_t out_len);

// 随机与素数
bignum bn_rand_bits(int bits);
bignum bn_gen_prime(int bits);

// 基本运算
bignum bn_add(const bignum a, const bignum b);
bignum bn_sub(const bignum a, const bignum b);
bignum bn_mul(const bignum a, const bignum b);
bignum bn_div(const bignum a, const bignum b);     // 整数除
bignum bn_mod(const bignum a, const bignum m);     // 取余
bignum bn_modmul(const bignum a, const bignum b, const bignum m);
bignum bn_modexp(const bignum base, const bignum exp, const bignum mod);
bignum bn_modinv(const bignum a, const bignum m);

// 比较与属性
int    bn_cmp(const bignum a, const bignum b);
int    bn_is_zero(const bignum a);

// RSA 接口
void   keygen(bignum *n, bignum *e, bignum *d, int bits);
bignum RSA_encrypt(const bignum m, const bignum e, const bignum n);
bignum RSA_decrypt(const bignum c, const bignum d, const bignum n);
bignum RSA_sign   (const bignum m, const bignum d, const bignum n);
int    RSA_verify (const bignum m, const bignum s, const bignum e, const bignum n);

#endif // BIGNUM_H

