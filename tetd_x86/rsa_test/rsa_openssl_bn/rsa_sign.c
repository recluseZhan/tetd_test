// rsa_sign.c
#include "bignum.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#define N_FILE     "rsa_n.key"
#define E_FILE     "rsa_e.key"
#define D_FILE     "rsa_d.key"
#define KEY_BITS   3072
#define SIG_BYTES  ((KEY_BITS+7)/8)

static bignum g_n, g_e, g_d;

// 保存/加载（大端 + 前置长度）
static void save_bn(const char *fn, const bignum a) {
    uint8_t buf[SIG_BYTES];
    int len = bn_to_bytes_be(a, buf, SIG_BYTES);
    FILE *f = fopen(fn, "wb");
    fwrite(&len, sizeof(int), 1, f);
    fwrite(buf, 1, len, f);
    fclose(f);
}

static bignum load_bn(const char *fn) {
    FILE *f = fopen(fn, "rb");
    int len; fread(&len, sizeof(int), 1, f);
    uint8_t *buf = malloc(len);
    fread(buf, 1, len, f);
    fclose(f);
    bignum r = bn_from_bytes_be(buf, len);
    free(buf);
    return r;
}

static void ensure_keys() {
    struct stat st;
    if (stat(N_FILE, &st) != 0) {
        keygen(&g_n, &g_e, &g_d, KEY_BITS);
        save_bn(N_FILE, g_n);
        save_bn(E_FILE, g_e);
        save_bn(D_FILE, g_d);
    } else {
        g_n = load_bn(N_FILE);
        g_e = load_bn(E_FILE);
        g_d = load_bn(D_FILE);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hex>\n", argv[0]);
        return 1;
    }
    ensure_keys();

    // 解析 hex
    size_t in_len = strlen(argv[1]) / 2;
    uint8_t *in = malloc(in_len);
    for (size_t i = 0; i < in_len; i++)
        sscanf(argv[1] + 2*i, "%2hhx", &in[i]);

    bignum m = bn_from_bytes_be(in, in_len);
    bignum s = RSA_sign(m, g_d, g_n);

    printf("Signature (%d bytes):\n", SIG_BYTES);
    uint8_t sig[SIG_BYTES];
    bn_to_bytes_be(s, sig, SIG_BYTES);
    for (int i = 0; i < SIG_BYTES; i++) printf("%02x", sig[i]);
    printf("\n");

    int ok = RSA_verify(m, s, g_e, g_n);
    printf("Verify: %s\n", ok ? "SUCCESS" : "FAIL");

    free(in);
    bn_free(m); bn_free(s);
    bn_free(g_n); bn_free(g_e); bn_free(g_d);
    return 0;
}

