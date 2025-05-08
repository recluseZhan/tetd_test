// bignum.c - Minimal Big Integer (bignum) implementation without external libs
#include "bignum.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>   
#include <stdint.h>



// —— 新增：前 100 个小素数，用于快速试除 —— 
static const uint32_t small_primes[100] = {
     3,   5,   7,  11,  13,  17,  19,  23,  29,  31,
    37,  41,  43,  47,  53,  59,  61,  67,  71,  73,
    79,  83,  89,  97, 101, 103, 107, 109, 113, 127,
   131, 137, 139, 149, 151, 157, 163, 167, 173, 179,
   181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
   239, 241, 251, 257, 263, 269, 271, 277, 281, 283,
   293, 307, 311, 313, 317, 331, 337, 347, 349, 353,
   359, 367, 373, 379, 383, 389, 397, 401, 409, 419,
   421, 431, 433, 439, 443, 449, 457, 461, 463, 467,
   479, 487, 491, 499, 503, 509, 521, 523, 541, 547
};

// bn_mod_word: 计算 a mod w，w 是 uint32_t
static uint32_t bn_mod_word(const bignum *a, uint32_t w) {
    uint64_t rem = 0;
    for (int i = (int)a->size - 1; i >= 0; i--) {
        rem = ((rem << 32) | a->limbs[i]) % w;
    }
    return (uint32_t)rem;
}

// 试除：如果 a 能被 small_primes 中任意一个整除（且不等于该素数），则判合数
static int bn_trial_division(const bignum *a) {
    for (int i = 0; i < 100; i++) {
        uint32_t p = small_primes[i];
        uint32_t r = bn_mod_word(a, p);
        if (r == 0) {
            // 如果 a 就等于 p，本身可以接受；否则合数
            if (a->size == 1 && a->limbs[0] == p) 
                continue;
            return 0;
        }
    }
    return 1;
}


//―――――――― 初始化与工具 ――――――――

void bn_zero(bignum *r) {
    memset(r, 0, sizeof(bignum));
    r->sign = 1;
}

void bn_from_u32(bignum *r, uint32_t x) {
    bn_zero(r);
    if (x > 0) {
        r->limbs[0] = x;
        r->size = 1;
    }
}

void bn_copy(bignum *dst, const bignum *src) {
    memcpy(dst, src, sizeof(bignum));
}

// 修剪前导零
static void bn_trim(bignum *r) {
    while (r->size > 0 && r->limbs[r->size - 1] == 0)
        r->size--;
    if (r->size == 0)
        r->sign = 1;
}

int bn_cmp(const bignum *a, const bignum *b) {
    if (a->sign != b->sign) return a->sign - b->sign;
    if (a->size != b->size) return (a->size - b->size) * a->sign;
    for (int i = (int)a->size - 1; i >= 0; i--) {
        if (a->limbs[i] != b->limbs[i])
            return (a->limbs[i] > b->limbs[i] ? 1 : -1) * a->sign;
    }
    return 0;
}

int bn_is_zero(const bignum *a) {
    return a->size == 0;
}

int bn_is_one(const bignum *a) {
    return a->size == 1 && a->limbs[0] == 1;
}

// 加法（同号）
void bn_add(const bignum *a, const bignum *b, bignum *r) {
    const bignum *x = a, *y = b;
    if (y->size > x->size) { x = b; y = a; }

    bn_zero(r);
    r->sign = x->sign;
    uint64_t carry = 0;
    size_t i;
    for (i = 0; i < y->size; i++) {
        uint64_t sum = (uint64_t)x->limbs[i] + y->limbs[i] + carry;
        r->limbs[i] = (uint32_t)sum;
        carry = sum >> 32;
    }
    while (i < x->size) {
        uint64_t sum = (uint64_t)x->limbs[i] + carry;
        r->limbs[i] = (uint32_t)sum;
        carry = sum >> 32;
        i++;
    }
    if (carry && i < BN_MAX_LIMBS)
        r->limbs[i++] = (uint32_t)carry;
    r->size = i;
    bn_trim(r);
}

// 减法（假设 a >= b）
void bn_sub(const bignum *a, const bignum *b, bignum *r) {
    bn_zero(r);
    r->sign = 1;
    uint64_t borrow = 0;
    size_t i;
    for (i = 0; i < b->size; i++) {
        uint64_t diff = (uint64_t)a->limbs[i] - b->limbs[i] - borrow;
        r->limbs[i] = (uint32_t)diff;
        borrow = (diff >> 63) & 1;
    }
    while (i < a->size) {
        uint64_t diff = (uint64_t)a->limbs[i] - borrow;
        r->limbs[i] = (uint32_t)diff;
        borrow = (diff >> 63) & 1;
        i++;
    }
    r->size = a->size;
    bn_trim(r);
}

// 乘法
void bn_mul(const bignum *a, const bignum *b, bignum *r) {
    bn_zero(r);
    if (a->size == 0 || b->size == 0) return;
    for (size_t i = 0; i < a->size; i++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < b->size; j++) {
            uint64_t prod = (uint64_t)a->limbs[i] * b->limbs[j] + r->limbs[i + j] + carry;
            r->limbs[i + j] = (uint32_t)prod;
            carry = prod >> 32;
        }
        r->limbs[i + b->size] = (uint32_t)carry;
    }
    r->size = a->size + b->size;
    bn_trim(r);
}

// 除法（只实现用于 RSA 的简单版本，a / b）
void bn_divmod(const bignum *a, const bignum *b, bignum *q, bignum *rem) {
    bignum R, B, T;
    bn_copy(&R, a);
    bn_copy(&B, b);
    bn_zero(q);
    bn_zero(rem);
    bn_zero(&T);

    int bits = (int)(a->size * 32);
    for (int i = bits - 1; i >= 0; i--) {
        bn_add(&R, &R, &R);  // 左移
        bn_add(q, q, q);
        if (bn_cmp(&R, &B) >= 0) {
            bn_sub(&R, &B, &R);
            bn_from_u32(&T, 1);
            bn_add(q, &T, q);
        }
    }
    bn_copy(rem, &R);
}

// 模运算：r = a mod m
void bn_mod(const bignum *a, const bignum *mod, bignum *r) {
    bignum q, rem;
    bn_divmod(a, mod, &q, r);
}

// 模乘：r = (a * b) mod m
void bn_modmul(const bignum *a, const bignum *b, const bignum *mod, bignum *r) {
    bignum t;
    bn_mul(a, b, &t);
    bn_mod(&t, mod, r);
}

// 模幂：r = (base ^ exp) mod mod
void bn_modexp(const bignum *base, const bignum *exp, const bignum *mod, bignum *r) {
    bignum result, b, e;
    bn_from_u32(&result, 1);
    bn_copy(&b, base);
    bn_copy(&e, exp);

    for (size_t i = 0; i < e.size; i++) {
        for (int j = 0; j < 32; j++) {
            if (e.limbs[i] & (1U << j)) {
                bn_modmul(&result, &b, mod, &result);
            }
            bn_modmul(&b, &b, mod, &b);
        }
    }
    bn_copy(r, &result);
}

// 模逆元：r = a^{-1} mod m
int bn_modinv(const bignum *a, const bignum *mod, bignum *r) {
    bignum t, newt, r0, newr, quotient, temp;

    bn_zero(&t);
    bn_from_u32(&newt, 1);
    bn_copy(&r0, mod);
    bn_copy(&newr, a);

    while (!bn_is_zero(&newr)) {
        bignum q, tmp;
        bn_divmod(&r0, &newr, &q, &tmp);

        // t, newt = newt, t - q*newt
        bn_copy(&temp, &newt);
        bn_mul(&q, &newt, &tmp);
        bn_sub(&t, &tmp, &newt);
        bn_copy(&t, &temp);

        // r, newr = newr, r - q*newr
        bn_copy(&temp, &newr);
        bn_mul(&q, &newr, &tmp);
        bn_sub(&r0, &tmp, &newr);
        bn_copy(&r0, &temp);
    }

    if (!bn_is_one(&r0)) return 0; // 不存在逆元
    while ((int)newt.sign < 0) {
        bn_add(&newt, mod, &newt);
    }
    bn_copy(r, &newt);
    return 1;
}


// 读取 /dev/urandom 填充随机字节
static void bn_rand_bytes(uint8_t *buf, size_t len) {
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) {
        perror("/dev/urandom");
        exit(1);
    }
    if (fread(buf, 1, len, f) != len) {
        perror("fread /dev/urandom");
        fclose(f);
        exit(1);
    }
    fclose(f);
}

// 改用 /dev/urandom，快速生成任意 bit 长度的随机 bignum
void bn_rand_bits(bignum *r, int bits) {
    // 清零
    bn_zero(r);

    // 计算所需字节数
    int bytes = (bits + 7) / 8;
    uint8_t buf[bytes];

    // 从 /dev/urandom 读取
    bn_rand_bytes(buf, bytes);

    // 转成 limbs（小端）。
    // buf 是大端，此处按 4 字节一 limb 转换
    size_t max_limbs = (bytes + 3) / 4;
    for (size_t i = 0; i < max_limbs; i++) {
        uint32_t limb = 0;
        size_t chunk = (bytes >= 4 ? 4 : bytes);
        for (size_t j = 0; j < chunk; j++) {
            limb = (limb << 8) | buf[bytes - (i * 4 + chunk) + j];
        }
        r->limbs[i] = limb;
        if (bytes >= 4) bytes -= 4;
        else bytes = 0;
    }
    r->size = max_limbs;
    bn_trim(r);

    // 强制最高位为 1（保证位长度），最低位为 1（确保是奇数）
    int top_bit = (bits - 1) % 32;
    r->limbs[r->size - 1] |= (1U << top_bit);
    r->limbs[0] |= 1;
}
// Miller-Rabin 素性测试
int bn_is_probable_prime(const bignum *n, int k) {
    bignum one, n1, d, a, x, tmp;
    bn_from_u32(&one, 1);
    bn_sub(n, &one, &n1); // n-1

    int s = 0;
    bn_copy(&d, &n1);
    while ((d.limbs[0] & 1) == 0) {
        for (size_t i = 0; i < d.size; i++) {
            d.limbs[i] >>= 1;
            if (i + 1 < d.size && (d.limbs[i + 1] & 1))
                d.limbs[i] |= 0x80000000;
        }
        bn_trim(&d);
        s++;
    }

    for (int i = 0; i < k; i++) {
        do {
            bn_rand_bits(&a, bn_cmp(n, &one) - 1);
        } while (bn_cmp(&a, &one) <= 0 || bn_cmp(&a, n) >= 0);

        bn_modexp(&a, &d, n, &x);
        if (bn_cmp(&x, &one) == 0 || bn_cmp(&x, &n1) == 0)
            continue;
        int continue_outer = 0;
        for (int r = 1; r < s; r++) {
            bn_modmul(&x, &x, n, &x);
            if (bn_cmp(&x, &n1) == 0) {
                continue_outer = 1;
                break;
            }
        }
        if (continue_outer)
            continue;
        return 0; // composite
    }
    return 1;
}

// RSA Keygen
void rsa_keygen(bignum *n, bignum *e, bignum *d, int bits) {
    bignum p, q, p1, q1, phi;
    int tries;

    // 生成 p
    tries = 0;
    do {
        bn_rand_bits(&p, bits / 2);
        // 强制最高位与最低位：保证 bit 长度 与 奇数
        p.limbs[p.size - 1] |= (1U << ((bits/2 - 1) % 32));
        p.limbs[0] |= 1;
        bn_trim(&p);

        tries++;
        if (tries % 1000 == 0) 
            printf("  ... tried %d candidates for p\n", tries);
    } while (!bn_trial_division(&p) || !bn_is_probable_prime(&p, 10));

    // 生成 q
    tries = 0;
    do {
        bn_rand_bits(&q, bits / 2);
        q.limbs[q.size - 1] |= (1U << ((bits/2 - 1) % 32));
        q.limbs[0] |= 1;
        bn_trim(&q);

        tries++;
        if (tries % 1000 == 0) 
            printf("  ... tried %d candidates for q\n", tries);
    } while (!bn_trial_division(&q) || !bn_is_probable_prime(&q, 10));

    // n = p * q
    bn_mul(&p, &q, n);

    // phi = (p-1)*(q-1)
    bn_sub(&p, &(bignum){.limbs={1}, .size=1, .sign=1}, &p1);
    bn_sub(&q, &(bignum){.limbs={1}, .size=1, .sign=1}, &q1);
    bn_mul(&p1, &q1, &phi);

    // 公钥 e 固定 65537
    bn_from_u32(e, 65537);

    // d = e^{-1} mod phi
    bn_modinv(e, &phi, d);
}

// RSA Operations
void rsa_encrypt(const bignum *m, const bignum *e, const bignum *n, bignum *r) {
    bn_modexp(m, e, n, r);
}
void rsa_decrypt(const bignum *c, const bignum *d, const bignum *n, bignum *r) {
    bn_modexp(c, d, n, r);
}
void rsa_sign(const bignum *m, const bignum *d, const bignum *n, bignum *r) {
    bn_modexp(m, d, n, r);
}
int rsa_verify(const bignum *m, const bignum *s, const bignum *e, const bignum *n) {
    bignum m1;
    bn_modexp(s, e, n, &m1);
    int ok = bn_cmp(m, &m1) == 0;
    return ok;
}

// from big-endian bytes
void bn_from_bytes_be(bignum *r, const uint8_t *buf, size_t len) {
    bn_zero(r);
    size_t i = 0;
    while (len > 0 && i < BN_MAX_LIMBS) {
        size_t chunk = len >= 4 ? 4 : len;
        uint32_t v = 0;
        for (size_t j = 0; j < chunk; j++) {
            v <<= 8;
            v |= buf[len - chunk + j];
        }
        r->limbs[i++] = v;
        len -= chunk;
    }
    r->size = i;
    bn_trim(r);
}

// to big-endian bytes
void bn_to_bytes_be(const bignum *a, uint8_t *buf, size_t len) {
    memset(buf, 0, len);
    size_t total = a->size * 4;
    for (size_t i = 0; i < a->size; i++) {
        uint32_t v = a->limbs[i];
        size_t offset = len - (i + 1) * 4;
        if (offset + 4 > len) continue;
        buf[offset + 0] = (v >> 24) & 0xff;
        buf[offset + 1] = (v >> 16) & 0xff;
        buf[offset + 2] = (v >> 8)  & 0xff;
        buf[offset + 3] = (v >> 0)  & 0xff;
    }
}

