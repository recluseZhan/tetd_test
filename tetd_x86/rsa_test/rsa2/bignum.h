#ifndef BIGNUM_H
#define BIGNUM_H

#include <stdint.h>
#include <stddef.h>

#define BN_LIMB_BITS 32
#define BN_MAX_LIMBS 128

typedef struct {
    uint32_t limbs[BN_MAX_LIMBS]; // 小端表示：limbs[0] 是最低位
    size_t   size; // 实际使用的 limb 数
    int      sign; // +1 或 -1
} bignum;

void   bn_zero(bignum *r);
void   bn_from_u32(bignum *r, uint32_t x);
void   bn_copy(bignum *dst, const bignum *src);
void   bn_from_bytes_be(bignum *r, const uint8_t *buf, size_t len);
void   bn_to_bytes_be(const bignum *a, uint8_t *buf, size_t len);

int    bn_cmp(const bignum *a, const bignum *b);
int    bn_is_zero(const bignum *a);
int    bn_is_one(const bignum *a);

void   bn_add(const bignum *a, const bignum *b, bignum *r);
void   bn_sub(const bignum *a, const bignum *b, bignum *r);
void   bn_mul(const bignum *a, const bignum *b, bignum *r);
void   bn_divmod(const bignum *a, const bignum *b, bignum *q, bignum *r);
void   bn_mod(const bignum *a, const bignum *m, bignum *r);
void   bn_modmul(const bignum *a, const bignum *b, const bignum *m, bignum *r);
void   bn_modexp(const bignum *base, const bignum *exp, const bignum *mod, bignum *r);
int    bn_modinv(const bignum *a, const bignum *m, bignum *r);

void   bn_rand_bits(bignum *r, int bits);
int    bn_is_probable_prime(const bignum *n, int rounds);

void   rsa_keygen(bignum *n, bignum *e, bignum *d, int bits);
void   rsa_encrypt(const bignum *m, const bignum *e, const bignum *n, bignum *r);
void   rsa_decrypt(const bignum *c, const bignum *d, const bignum *n, bignum *r);
void   rsa_sign   (const bignum *m, const bignum *d, const bignum *n, bignum *r);
int    rsa_verify (const bignum *m, const bignum *s, const bignum *e, const bignum *n);

#endif

