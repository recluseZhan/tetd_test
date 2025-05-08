// asn1.c — 通用 PEM→DER + PKCS#1/PKCS#8 解析
#include "asn1.h"
#include "bignum.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// —— Base64 解码表 ——
static int B64[256];
__attribute__((constructor))
static void init_b64(void) {
    memset(B64, -1, sizeof(B64));
    const char *chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    for (int i = 0; chars[i]; i++)
        B64[(unsigned char)chars[i]] = i;
}

// 通用 PEM→DER：匹配任意 “-----BEGIN … KEY-----” 到 “-----END … KEY-----”
size_t pem_read_der(const char *filename, uint8_t *out, size_t maxlen) {
    init_b64();
    FILE *f = fopen(filename, "r");
    if (!f) { perror(filename); return 0; }
    char line[1024];
    int in_b64 = 0, q = 0, line_no = 0;
    char buf4[4];
    size_t olen = 0;

    printf("[pem] open '%s'\n", filename);
    while (fgets(line, sizeof(line), f)) {
        line_no++;
        if (!in_b64) {
            // 进入 Base64 段：任意 BEGIN 行
            if (strstr(line, "-----BEGIN ") && strstr(line, "KEY-----")) {
                in_b64 = 1;
                printf("[pem] line %d: BEGIN detected: %s", line_no, line);
            }
        } else {
            // 看到 END 行就退出
            if (strstr(line, "-----END ") && strstr(line, "KEY-----")) {
                printf("[pem] line %d: END   detected: %s", line_no, line);
                break;
            }
            // 否则当作 Base64 字符行，逐字符收集
            for (char *p = line; *p; p++) {
                int v = B64[(unsigned char)*p];
                if (v >= 0 || *p == '=') {
                    buf4[q++] = *p;
                    if (q == 4) {
                        uint32_t val = 0;
                        int pad = 0;
                        for (int i = 0; i < 4; i++) {
                            if (buf4[i] == '=') {
                                val <<= 6; pad++;
                            } else {
                                val = (val << 6) | B64[(unsigned char)buf4[i]];
                            }
                        }
                        if (olen + 3 - pad > maxlen) {
                            fprintf(stderr, "[pem] buffer full at olen=%zu\n", olen);
                            fclose(f);
                            return olen;
                        }
                        if (pad < 3) out[olen++] = (val >> 16) & 0xFF;
                        if (pad < 2) out[olen++] = (val >>  8) & 0xFF;
                        if (pad < 1) out[olen++] =  val        & 0xFF;
                        q = 0;
                    }
                }
            }
        }
    }
    fclose(f);
    printf("[pem] decoded DER bytes: %zu\n", olen);
    return olen;
}

// 读取 ASN.1 长度字段
static size_t asn1_len(const uint8_t **p, const uint8_t *end) {
    if (*p >= end) return 0;
    uint8_t b = *(*p)++;
    if (!(b & 0x80)) return b;
    int n = b & 0x7F;
    size_t L = 0;
    while (n-- > 0 && *p < end) {
        L = (L << 8) | *(*p)++;
    }
    return L;
}

// 读取 ASN.1 INTEGER 到 bignum
static int asn1_get_int(const uint8_t **p, const uint8_t *end, bignum *r) {
    if (*p >= end || *(*p)++ != 0x02) return 0;
    size_t L = asn1_len(p, end);
    if (L > 0 && **p == 0) { (*p)++; L--; }
    if (*p + L > end) return 0;
    bn_from_be(r, *p, L);
    *p += L;
    return 1;
}

// 内部：解析 PKCS#1 RSAPrivateKey（只读 n,e,d）
static int parse_pkcs1_priv(const uint8_t *der, size_t len,
                            bignum *n, bignum *e, bignum *d) {
    const uint8_t *p = der, *end = der + len;
    if (p >= end || *p++ != 0x30) return 0;
    asn1_len(&p, end);  // SEQUENCE 长度
    // skip version
    if (*p++ != 0x02) return 0;
    asn1_len(&p, end);
    if (!asn1_get_int(&p,end,n)) return 0;
    if (!asn1_get_int(&p,end,e)) return 0;
    if (!asn1_get_int(&p,end,d)) return 0;
    return 1;
}

// 公共接口：支持 PKCS#1 与 PKCS#8 私钥
int asn1_parse_priv(const uint8_t *der, size_t len,
                    bignum *n, bignum *e, bignum *d) {
    const uint8_t *p = der, *end = der + len;
    if (p >= end || *p++ != 0x30) return 0;
    size_t seq_len = asn1_len(&p, end);
    if (p + seq_len > end) return 0;
    const uint8_t *seq_end = p + seq_len;

    // Peek version INTEGER
    if (p[0] == 0x02) {
        // PKCS#1
        return parse_pkcs1_priv(der, len, n, e, d);
    } 
    // PKCS#8: SEQ { version, algId, privOctetString }
    // Skip version
    if (*p++ != 0x02) return 0;
    asn1_len(&p, seq_end);
    // Skip algId SEQ
    if (*p++ != 0x30) return 0;
    asn1_len(&p, seq_end);
    // Skip algorithm OID
    if (*p++ != 0x06) return 0;
    asn1_len(&p, seq_end);
    // Optional NULL
    if (*p == 0x05) { p++; asn1_len(&p, seq_end); }
    // OCTET STRING
    if (*p++ != 0x04) return 0;
    size_t priv_len = asn1_len(&p, seq_end);
    if (p + priv_len > seq_end) return 0;
    // Recurse into PKCS#1 structure
    return parse_pkcs1_priv(p, priv_len, n, e, d);
}

// 公钥解析：支持 PKCS#1 与 X.509/SPKI
static int parse_pkcs1_pub(const uint8_t *der, size_t len,
                           bignum *n, bignum *e) {
    const uint8_t *p = der, *end = der + len;
    if (p >= end || *p++ != 0x30) return 0;
    asn1_len(&p, end);
    if (!asn1_get_int(&p,end,n)) return 0;
    if (!asn1_get_int(&p,end,e)) return 0;
    return 1;
}

int asn1_parse_pub(const uint8_t *der, size_t len,
                   bignum *n, bignum *e) {
    const uint8_t *p = der, *end = der + len;
    if (p >= end || *p++ != 0x30) return 0;
    size_t seq_len = asn1_len(&p, end);
    if (p + seq_len > end) return 0;

    // Peek next byte to see if it's an AlgorithmIdentifier SEQ (SPKI)
    if (p[0] == 0x30) {
        // X.509/SPKI: SEQ { algId, BIT STRING }
        // Skip algId SEQ
        p++; asn1_len(&p, p+seq_len);
        // BIT STRING tag
        if (*p++ != 0x03) return 0;
        size_t bs_len = asn1_len(&p, p+seq_len);
        // skip unused bits count
        p++; bs_len--;
        // parse inner PKCS#1 RSAPublicKey
        return parse_pkcs1_pub(p, bs_len, n, e);
    } else {
        // Direct PKCS#1 RSAPublicKey
        return parse_pkcs1_pub(der, len, n, e);
    }
}

