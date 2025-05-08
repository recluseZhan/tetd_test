#include "bignum.h"
#include <string.h>

// 去除高位0
static void bn_trim(bignum *r) {
    while (r->size && r->limbs[r->size - 1] == 0) r->size--;
}

// 从大端字节流 buf[len] 构造 bignum
void bn_from_be(bignum *r, const uint8_t *buf, size_t len) {
    memset(r, 0, sizeof(*r));
    size_t limbs = (len + 3) / 4;
    for (size_t i = 0; i < limbs; i++) {
        uint32_t w = 0;
        size_t chunk = (len >= 4 ? 4 : len);
        for (size_t j = 0; j < chunk; j++) {
            w = (w << 8) | buf[len - chunk + j];
        }
        r->limbs[i] = w;
        if (len >= 4) len -= 4; else len = 0;
    }
    r->size = limbs;
    bn_trim(r);
}

// 输出成大端字节流，长度固定 len，大于实际则左填零
void bn_to_be(const bignum *a, uint8_t *buf, size_t len) {
    memset(buf, 0, len);
    for (size_t i = 0; i < a->size; i++) {
        uint32_t w = a->limbs[i];
        size_t off = len - (i + 1) * 4;
        if (off + 4 <= len) {
            buf[off + 0] = (w >> 24) & 0xFF;
            buf[off + 1] = (w >> 16) & 0xFF;
            buf[off + 2] = (w >> 8) & 0xFF;
            buf[off + 3] = w & 0xFF;
        }
    }
}

// r = a * b mod m
void bn_modmul(const bignum *a, const bignum *b, const bignum *m, bignum *r) {
    // schoolbook multiplication into tmp (up to 2*BN_MAX_LIMBS limbs)
    uint64_t tmp[BN_MAX_LIMBS*2] = {0};
    for (size_t i = 0; i < a->size; i++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < b->size; j++) {
            uint64_t v = tmp[i+j] + (uint64_t)a->limbs[i] * b->limbs[j] + carry;
            tmp[i+j] = v & 0xFFFFFFFF;
            carry = v >> 32;
        }
        tmp[i + b->size] = carry;
    }
    // reduce modulo m by repeated subtract-shift (naive)
    // we implement simple long division remainder: r = tmp mod m
    bignum R = {0}, M = *m;
    // copy tmp into R
    size_t tsize = a->size + b->size;
    for (size_t i = 0; i < tsize; i++) R.limbs[i] = (uint32_t)tmp[i];
    R.size = tsize; bn_trim(&R);
    // division: subtract m<<k while R>=m<<k
    // find bit-length
    size_t mbits = (M.size-1)*32 + 32 - __builtin_clz(M.limbs[M.size-1]);
    size_t Rbits = (R.size-1)*32 + (R.size?32-__builtin_clz(R.limbs[R.size-1]):0);
    while (Rbits >= mbits) {
        size_t shift = Rbits - mbits;
        // compute M<<shift
        bignum T = {0};
        size_t wshift = shift / 32;
        int bshift = shift % 32;
        for (size_t i = 0; i < M.size; i++) {
            uint64_t v = (uint64_t)M.limbs[i] << bshift;
            T.limbs[i + wshift] |= (uint32_t)(v & 0xFFFFFFFF);
            if (i + wshift + 1 < BN_MAX_LIMBS)
                T.limbs[i + wshift + 1] |= (uint32_t)(v >> 32);
        }
        T.size = M.size + wshift + 1; bn_trim(&T);
        // R = R - T
        uint64_t borrow = 0;
        for (size_t i = 0; i < R.size; i++) {
            uint64_t rv = (uint64_t)R.limbs[i];
            uint64_t tv = (i < T.size ? T.limbs[i] : 0) + borrow;
            if (rv < tv) {
                R.limbs[i] = (uint32_t)(rv + ((uint64_t)1<<32) - tv);
                borrow = 1;
            } else {
                R.limbs[i] = (uint32_t)(rv - tv);
                borrow = 0;
            }
        }
        bn_trim(&R);
        Rbits = (R.size-1)*32 + (R.size?32-__builtin_clz(R.limbs[R.size-1]):0);
    }
    *r = R;
}

// r = a^e mod m  (square-and-multiply)
void bn_modexp(const bignum *a, const bignum *e, const bignum *m, bignum *r) {
    bignum base = *a;
    bn_one(r);
    for (size_t i = 0; i < e->size; i++) {
        for (int b = 0; b < 32; b++) {
            // r = r*r mod m
            bn_modmul(r, r, m, r);
            if (e->limbs[i] & (1u << b)) {
                bn_modmul(r, &base, m, r);
            }
        }
    }
}

// r = 1
void bn_one(bignum *r) {
    memset(r, 0, sizeof(*r));
    r->limbs[0] = 1;
    r->size = 1;
}

