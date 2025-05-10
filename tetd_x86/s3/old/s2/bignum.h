#ifndef BIGNUM_H
#define BIGNUM_H

#include <stdint.h>
#include <stddef.h>

// 3072位 / 64 = 48 limbs
#define NLIMBS      48
// 中间乘积要 2*NLIMBS
#define MAX_LIMBS   (2 * NLIMBS)

// 一个 limb 使用 64 位
typedef uint64_t integer;

// bignum 结构：最多 2*NLIMBS limbs 存储空间。
// “有效” limbs 数量由 size 指定，始终保证 size ≤ NLIMBS（即 <n的位宽）。
typedef struct {
    size_t   size;          // 有效 limbs 数目 (1..NLIMBS)
    integer  tab[MAX_LIMBS];//
} bignum;

// Montgomery 上下文
typedef struct {
    bignum   n;      // 模数
    bignum   r2;     // R^2 mod n
    integer  n0inv;  // -n^{-1} mod 2^64
} mont_ctx;

// ────────────────────────────────────────────────────────────────
// 基本大数运算
void bn_init(bignum *a);
void bn_copy(bignum *dst, const bignum *src);
int  bn_cmp(const bignum *a, const bignum *b);
void bn_add(const bignum *a, const bignum *b, bignum *r);
void bn_sub(const bignum *a, const bignum *b, bignum *r);
void bn_mul(const bignum *a, const bignum *b, bignum *r);

// ────────────────────────────────────────────────────────────────
// Montgomery 乘法和幂
void mont_init(mont_ctx *m, const bignum *n);
void mont_mul(const mont_ctx *m,
              const bignum *a,
              const bignum *b,
              bignum *r);
void mont_exp(const mont_ctx *m,
              const bignum *a,
              const bignum *e,
              bignum *r);

// ────────────────────────────────────────────────────────────────
// Hex / bytes 转换
void bn_from_hex(bignum *a, const char *hex);
void bn_to_bytes(const bignum *a, uint8_t *out, size_t len);

#endif // BIGNUM_H

