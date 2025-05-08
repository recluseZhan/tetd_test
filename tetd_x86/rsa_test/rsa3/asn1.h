#ifndef ASN1_H
#define ASN1_H

#include <stddef.h>
#include <stdint.h>
#include "bignum.h"

// 从 PEM 文件中读取 Base64 区域并解码为 DER，返回长度
size_t pem_read_der(const char *filename, uint8_t *out, size_t maxlen);

// 从 DER 中解析 RSA 私钥 PKCS#1：modulus, publicExp, privateExp
// 返回 1 成功
int asn1_parse_priv(const uint8_t *der, size_t derlen,
                    bignum *n, bignum *e, bignum *d);

// 从 DER 中解析 RSA 公钥 PKCS#1：modulus, publicExp
int asn1_parse_pub(const uint8_t *der, size_t derlen,
                   bignum *n, bignum *e);

#endif // ASN1_H

