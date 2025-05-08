#include "bignum.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#define KEY_BITS    3072
#define SIG_BYTES   (KEY_BITS / 8)

bignum n, e, d;

void save_bn(const char *fn, const bignum *a) {
    FILE *f = fopen(fn, "wb");
    uint8_t buf[SIG_BYTES] = {0};
    bn_to_bytes_be(a, buf, SIG_BYTES);
    fwrite(buf, 1, SIG_BYTES, f);
    fclose(f);
}

void load_bn(const char *fn, bignum *a) {
    FILE *f = fopen(fn, "rb");
    uint8_t buf[SIG_BYTES] = {0};
    fread(buf, 1, SIG_BYTES, f);
    fclose(f);
    bn_from_bytes_be(a, buf, SIG_BYTES);
}

void ensure_keys() {
    FILE *f = fopen("rsa_n.key", "rb");
    if (!f) {
        printf("[*] Generating RSA-3072 keys...\n");
        srand((unsigned)time(NULL));    // <== **初始化随机种子**
	rsa_keygen(&n, &e, &d, KEY_BITS);
        save_bn("rsa_n.key", &n);
        save_bn("rsa_e.key", &e);
        save_bn("rsa_d.key", &d);
    } else {
        fclose(f);
        load_bn("rsa_n.key", &n);
        load_bn("rsa_e.key", &e);
        load_bn("rsa_d.key", &d);
    }
}

void hex_print(const char *label, const uint8_t *buf, size_t len) {
    printf("%s:", label);
    for (size_t i = 0; i < len; i++) printf("%02x", buf[i]);
    printf("\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <hex-data>\n", argv[0]);
        return 1;
    }

    ensure_keys();

    size_t in_len = strlen(argv[1]) / 2;
    uint8_t *input = malloc(in_len);
    for (size_t i = 0; i < in_len; i++)
        sscanf(argv[1] + 2 * i, "%2hhx", &input[i]);

    bignum m, sig;
    bn_from_bytes_be(&m, input, in_len);
    rsa_sign(&m, &d, &n, &sig);

    uint8_t sigbuf[SIG_BYTES];
    bn_to_bytes_be(&sig, sigbuf, SIG_BYTES);
    hex_print("Signature", sigbuf, SIG_BYTES);

    int ok = rsa_verify(&m, &sig, &e, &n);
    printf("Verify: %s\n", ok ? "SUCCESS" : "FAIL");

    free(input);
    return 0;
}

