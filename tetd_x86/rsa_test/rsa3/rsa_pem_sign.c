#include "bignum.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define KEY_BYTES  (512/8)   // 512-bit 演示
#define SIG_BYTES  KEY_BYTES

// 随机生成的一对 512-bit RSA 参数
static const char *hex_n =
  "e103abd94892e3e74afd724bf28e78366d9676bccc70118bd0aa1968dbb143d1";
static const char *hex_e = "010001";
static const char *hex_d =
  "c5148b1ad7c6d85847c52eabb879f26cccee8eb70d2282dc6050309c18f85d85";

// 原文嵌入
static const uint8_t input_data[] = { 0x48,0x65,0x6c,0x6c,0x6f }; // "Hello"
static const size_t input_len = sizeof(input_data);

// hex → byte
static void hex2bin(const char *hex, uint8_t *out, size_t outlen) {
    for (size_t i = 0; i < outlen; i++) {
        sscanf(hex + 2*i, "%2hhx", &out[i]);
    }
}

// hex 打印
static void hexout(const char *label, const uint8_t *buf, size_t len) {
    printf("%s:", label);
    for (size_t i = 0; i < len; i++) printf("%02x", buf[i]);
    printf("\n");
}

int main(void) {
    // 构建 n,e,d
    bignum n,e,d;
    uint8_t tmp[KEY_BYTES];

    hex2bin(hex_n, tmp, KEY_BYTES);
    bn_from_be(&n, tmp, KEY_BYTES);

    size_t e_len = strlen(hex_e)/2;
    hex2bin(hex_e, tmp, e_len);
    bn_from_be(&e, tmp, e_len);

    hex2bin(hex_d, tmp, KEY_BYTES);
    bn_from_be(&d, tmp, KEY_BYTES);

    // 打印原文
    hexout("Input", input_data, input_len);

    // 签名： sig = m^d mod n
    bignum m,sig;
    bn_from_be(&m, input_data, input_len);
    bn_modexp(&m, &d, &n, &sig);
    uint8_t sigbuf[SIG_BYTES];
    bn_to_be(&sig, sigbuf, SIG_BYTES);
    hexout("Signature", sigbuf, SIG_BYTES);

    // 验签： rec = sig^e mod n
    bignum rec;
    bn_modexp(&sig, &e, &n, &rec);
    uint8_t recbuf[input_len];
    bn_to_be(&rec, recbuf, input_len);
    hexout("Recovered", recbuf, input_len);

    if (memcmp(recbuf, input_data, input_len)==0)
        printf("[+] Verification SUCCESS\n");
    else
        printf("[-] Verification FAIL\n");

    return 0;
}

