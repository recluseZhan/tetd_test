#include "bignum.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define WORD_BITS 64

// ----------------- 基础运算 -----------------

void bn_init(bignum *a) {
    a->size = 1;
    a->tab[0] = 0;
    // 清空剩余 storage
    for (size_t i = 1; i < MAX_LIMBS; i++) a->tab[i] = 0;
}

void bn_copy(bignum *dst, const bignum *src) {
    dst->size = src->size;
    memcpy(dst->tab, src->tab, sizeof(integer)*src->size);
}

int bn_cmp(const bignum *a, const bignum *b) {
    if (a->size != b->size) return a->size < b->size ? -1 : 1;
    for (int i = (int)a->size - 1; i >= 0; i--) {
        if (a->tab[i] < b->tab[i]) return -1;
        if (a->tab[i] > b->tab[i]) return 1;
    }
    return 0;
}

void bn_add(const bignum *a, const bignum *b, bignum *r) {
    __uint128_t carry = 0;
    size_t n = a->size > b->size ? a->size : b->size;
    for (size_t i = 0; i < n; i++) {
        __uint128_t av = i < a->size ? a->tab[i] : 0;
        __uint128_t bv = i < b->size ? b->tab[i] : 0;
        __uint128_t sum = av + bv + carry;
        r->tab[i] = (integer)sum;
        carry = sum >> WORD_BITS;
    }
    r->size = n + (carry ? 1 : 0);
    if (carry) r->tab[n] = (integer)carry;
    while (r->size > 1 && r->tab[r->size - 1] == 0) r->size--;
}

void bn_sub(const bignum *a, const bignum *b, bignum *r) {
    __int128_t carry = 0;
    for (size_t i = 0; i < a->size; i++) {
        __int128_t av = a->tab[i];
        __int128_t bv = i < b->size ? b->tab[i] : 0;
        __int128_t diff = av - bv + carry;
        if (diff < 0) {
            diff += ( (__int128_t)1 << WORD_BITS );
            carry = -1;
        } else {
            carry = 0;
        }
        r->tab[i] = (integer)diff;
    }
    r->size = a->size;
    while (r->size > 1 && r->tab[r->size - 1] == 0) r->size--;
}

// schoolbook multiplication into r->tab[0..(as+bs-1)]
static void bn_mul_school(const bignum *a, const bignum *b, bignum *r) {
    size_t as = a->size, bs = b->size;
    assert(as + bs <= MAX_LIMBS);
    for (size_t i = 0; i < as + bs; i++) r->tab[i] = 0;
    for (size_t i = 0; i < as; i++) {
        __uint128_t carry = 0;
        for (size_t j = 0; j < bs; j++) {
            __uint128_t v = (__uint128_t)a->tab[i] * b->tab[j]
                           + r->tab[i+j] + carry;
            r->tab[i+j] = (integer)v;
            carry = v >> WORD_BITS;
        }
        r->tab[i+bs] += (integer)carry;
    }
    r->size = as + bs;
    while (r->size > 1 && r->tab[r->size - 1] == 0) r->size--;
}

void bn_mul(const bignum *a, const bignum *b, bignum *r) {
    bn_mul_school(a, b, r);
}

// ----------------- Montgomery -----------------

static integer mont_inv64(integer n0) {
    integer x = 1;
    for (int i = 0; i < 6; i++) {
        x = x * (2 - n0 * x);
    }
    return (integer)(-x);
}

void mont_init(mont_ctx *m, const bignum *n) {
    // 1) copy modulus
    bn_copy(&m->n, n);
    // 2) compute n0inv = -n^{-1} mod 2^64
    m->n0inv = mont_inv64(n->tab[0]);

    // 3) build R² = 2^(128*size) mod n via repeated 64‑bit limb shifts
    //    (3072 bits = 48 limbs, so 2*48 shifts)
    bignum r;
    bn_init(&r);
    r.tab[0] = 1;
    r.size = 1;

    // one “limb” = 2^64
    bignum limb;
    bn_init(&limb);
    limb.size = 2;
    limb.tab[0] = 0;
    limb.tab[1] = 1;

    for (size_t i = 0; i < 2*m->n.size; i++) {
        // r <<= 64 bits
        bignum tmp;
        tmp.size = r.size + 1;
        tmp.tab[0] = 0;
        memcpy(&tmp.tab[1], r.tab, r.size * sizeof(integer));
        // reduce mod n
        if (bn_cmp(&tmp, &m->n) >= 0) {
            bn_sub(&tmp, &m->n, &tmp);
        }
        bn_copy(&r, &tmp);
    }

    // now r = R² mod n
    bn_copy(&m->r2, &r);
}

void mont_mul(const mont_ctx *m, const bignum *a, const bignum *b, bignum *r) {
    size_t n = m->n.size;
    static __uint128_t t[MAX_LIMBS*2 + 2];
    memset(t, 0, sizeof(t));

    // 1) a*b
    for (size_t i = 0; i < a->size; i++) {
        __uint128_t carry = 0;
        for (size_t j = 0; j < b->size; j++) {
            __uint128_t v = (__uint128_t)a->tab[i] * b->tab[j]
                           + t[i+j] + carry;
            t[i+j] = v & (((__uint128_t)1 << WORD_BITS) - 1);
            carry = v >> WORD_BITS;
        }
        t[i + b->size] += carry;
    }

    // 2) Montgomery reduce
    for (size_t i = 0; i < n; i++) {
        integer m0 = (integer)t[i] * m->n0inv;
        __uint128_t carry = 0;
        for (size_t j = 0; j < n; j++) {
            __uint128_t v = (__uint128_t)m0 * m->n.tab[j]
                           + t[i+j] + carry;
            t[i+j] = v & (((__uint128_t)1 << WORD_BITS) - 1);
            carry = v >> WORD_BITS;
        }
        t[i+n] += carry;
    }

    // 3) 拷贝并可能减一次
    for (size_t i = 0; i < n; i++) {
        r->tab[i] = (integer)t[n + i];
    }
    r->size = n;
    if (bn_cmp(r, &m->n) >= 0) {
        bn_sub(r, &m->n, r);
    }
}

// ----------------- Sliding‑window 模幂 -----------------

void mont_exp(const mont_ctx *m, const bignum *a, const bignum *e, bignum *r) {
    const int w = 4;
    const int tbl_size = 1 << (w - 1);
    bignum table[tbl_size];

    // table[0] = aR mod n
    bn_init(&table[0]);
    mont_mul(m, a, &m->r2, &table[0]);
    for (int i = 1; i < tbl_size; i++) {
        mont_mul(m, &table[i-1], &table[0], &table[i]);
    }

    // r = R mod n
    bn_copy(r, &m->r2);

    int total_bits = (int)e->size * WORD_BITS;
    int bit = total_bits - 1;
    while (bit >= 0) {
        if (((e->tab[bit/WORD_BITS] >> (bit%WORD_BITS)) & 1) == 0) {
            mont_mul(m, r, r, r);
            bit--;
        } else {
            int max_w = bit+1 < w ? bit+1 : w;
            int val = 0;
            for (int k = 0; k < max_w; k++) {
                val |= ((int)((e->tab[(bit-k)/WORD_BITS] >> ((bit-k)%WORD_BITS)) & 1)) << k;
            }
            int l = max_w;
            while ((val & 1) == 0) {
                val >>= 1;
                l--;
            }
            int idx = val >> 1;
            for (int k = 0; k < l; k++) {
                mont_mul(m, r, r, r);
            }
            mont_mul(m, r, &table[idx], r);
            bit -= l;
        }
    }

    // 转回普通
    bignum one; bn_init(&one); one.tab[0] = 1;
    mont_mul(m, r, &one, r);
}

// ----------------- Hex/Bytes 转换 -----------------

static void hex2bin(const char *hex, uint8_t *out, size_t outlen) {
    for (size_t i = 0; i < outlen; i++) {
        unsigned byte;
        sscanf(hex + 2*i, "%2x", &byte);
        out[i] = (uint8_t)byte;
    }
}

void bn_from_hex(bignum *a, const char *hex) {
    size_t hexlen = strlen(hex);
    assert(hexlen % 2 == 0);
    size_t byts = hexlen / 2;
    uint8_t *buf = malloc(byts);
    if (!buf) { perror("malloc"); exit(1); }
    hex2bin(hex, buf, byts);

    // skip leading zeros
    size_t first = 0;
    while (first < byts && buf[first] == 0) first++;
    size_t real = byts - first;
    size_t limbs = (real + 7) / 8;
    assert(limbs <= NLIMBS);
    a->size = limbs;
    memset(a->tab, 0, sizeof(integer)*a->size);

    for (size_t i = 0; i < limbs; i++) {
        uint64_t v = 0;
        size_t base = byts >= 8*(i+1) ? byts - 8*(i+1) : first;
        for (size_t j = 0; j < 8 && base + j < byts; j++) {
            v = (v << 8) | buf[base + j];
        }
        a->tab[i] = v;
    }
    free(buf);
}

void bn_to_bytes(const bignum *a, uint8_t *out, size_t len) {
    bignum tmp; bn_copy(&tmp, a);
    for (size_t i = 0; i < len; i++) {
        __uint128_t rem = 0;
        for (int j = (int)tmp.size - 1; j >= 0; j--) {
            __uint128_t cur = (rem << WORD_BITS) | tmp.tab[j];
            tmp.tab[j] = (integer)(cur >> 8);
            rem = (integer)(cur & 0xFF);
        }
        out[len - 1 - i] = (uint8_t)rem;
        while (tmp.size > 1 && tmp.tab[tmp.size - 1] == 0) tmp.size--;
    }
}

