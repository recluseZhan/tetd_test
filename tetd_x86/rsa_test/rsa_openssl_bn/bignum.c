// bignum.c
#include "bignum.h"
#include <openssl/bn.h>
#include <stdlib.h>
#include <string.h>

//——— 构造/析构 ——————————————————————————————

bignum bn_new(void) {
    return BN_new();
}

void bn_free(bignum a) {
    BN_free(a);
}

//——— 字节互转 ——————————————————————————————

bignum bn_from_bytes_be(const uint8_t *in, size_t len) {
    return BN_bin2bn(in, len, NULL);
}

int bn_to_bytes_be(const bignum a, uint8_t *out, size_t out_len) {
    int needed = BN_num_bytes(a);
    if ((size_t)needed > out_len) return -1;
    // 写到尾部
    BN_bn2bin(a, out + (out_len - needed));
    // 前面填 0
    memset(out, 0, out_len - needed);
    return needed;
}

//——— 随机 与 素数 ——————————————————————————————

bignum bn_rand_bits(int bits) {
    bignum r = bn_new();
    BN_rand(r, bits, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ANY);
    return r;
}

bignum bn_gen_prime(int bits) {
    bignum p = bn_new();
    BN_generate_prime_ex(p, bits, 0, NULL, NULL, NULL);
    return p;
}

//——— 基本算术 ——————————————————————————————

bignum bn_add(const bignum a, const bignum b) {
    bignum r = bn_new();
    BN_add(r, a, b);
    return r;
}

bignum bn_sub(const bignum a, const bignum b) {
    bignum r = bn_new();
    BN_sub(r, a, b);
    return r;
}

bignum bn_mul(const bignum a, const bignum b) {
    bignum r = bn_new();
    BN_CTX *ctx = BN_CTX_new();
    BN_mul(r, a, b, ctx);
    BN_CTX_free(ctx);
    return r;
}

bignum bn_div(const bignum a, const bignum b) {
    bignum q = bn_new(), rem = bn_new();
    BN_CTX *ctx = BN_CTX_new();
    BN_div(q, rem, a, b, ctx);
    BN_clear_free(rem);
    BN_CTX_free(ctx);
    return q;
}

bignum bn_mod(const bignum a, const bignum m) {
    bignum r = bn_new();
    BN_CTX *ctx = BN_CTX_new();
    BN_mod(r, a, m, ctx);
    BN_CTX_free(ctx);
    return r;
}

bignum bn_modmul(const bignum a, const bignum b, const bignum m) {
    bignum r = bn_new();
    BN_CTX *ctx = BN_CTX_new();
    BN_mod_mul(r, a, b, m, ctx);
    BN_CTX_free(ctx);
    return r;
}

bignum bn_modexp(const bignum base, const bignum exp, const bignum mod) {
    bignum r = bn_new();
    BN_CTX *ctx = BN_CTX_new();
    BN_mod_exp(r, base, exp, mod, ctx);
    BN_CTX_free(ctx);
    return r;
}

bignum bn_modinv(const bignum a, const bignum m) {
    bignum r = bn_new();
    BN_CTX *ctx = BN_CTX_new();
    BN_mod_inverse(r, a, m, ctx);
    BN_CTX_free(ctx);
    return r;
}

int bn_cmp(const bignum a, const bignum b) {
    return BN_cmp(a, b);
}

int bn_is_zero(const bignum a) {
    return BN_is_zero(a);
}

//——— RSA 接口 ——————————————————————————————

void keygen(bignum *n, bignum *e, bignum *d, int bits) {
    // 生成 p, q
    bignum p = bn_gen_prime(bits/2);
    bignum q = bn_gen_prime(bits/2);

    // n = p * q
    *n = bn_mul(p, q);

    // phi = (p-1)*(q-1)
    bignum p1 = bn_sub(p, BN_value_one());
    bignum q1 = bn_sub(q, BN_value_one());
    bignum phi = bn_mul(p1, q1);

    // e = 65537
    *e = bn_new();
    BN_set_word(*e, 65537);

    // d = e^{-1} mod phi
    *d = bn_modinv(*e, phi);

    bn_free(p); bn_free(q);
    bn_free(p1); bn_free(q1); bn_free(phi);
}

bignum RSA_encrypt(const bignum m, const bignum e, const bignum n) {
    return bn_modexp(m, e, n);
}

bignum RSA_decrypt(const bignum c, const bignum d, const bignum n) {
    return bn_modexp(c, d, n);
}

bignum RSA_sign(const bignum m, const bignum d, const bignum n) {
    return bn_modexp(m, d, n);
}

int RSA_verify(const bignum m, const bignum s, const bignum e, const bignum n) {
    bignum m1 = bn_modexp(s, e, n);
    int ok = (BN_cmp(m1, m) == 0);
    bn_free(m1);
    return ok;
}

