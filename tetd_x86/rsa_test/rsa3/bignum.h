#ifndef BIGNUM_H
#define BIGNUM_H

#include <stddef.h>
#include <stdint.h>

// limbs: 32-bit little endian, 最大可支持 4096-bit => 128 limbs
#define BN_MAX_LIMBS 128

typedef struct {
    uint32_t limbs[BN_MAX_LIMBS];
    size_t   size;   // 有效 limbs 数
} bignum;

// 从大端字节流构造 bignum
void bn_from_be(bignum *r, const uint8_t *buf, size_t len);
// 输出为大端字节流，不足左填零
void bn_to_be(const bignum *a, uint8_t *buf, size_t len);

// bn = 1
void bn_one(bignum *r);
// c = a * b mod m
void bn_modmul(const bignum *a, const bignum *b, const bignum *m, bignum *c);
// c = a^e mod m
void bn_modexp(const bignum *a, const bignum *e, const bignum *m, bignum *c);

#endif // BIGNUM_H

