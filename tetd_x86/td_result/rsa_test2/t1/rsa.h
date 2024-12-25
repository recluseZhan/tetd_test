#ifndef __RSA_H__
#define __RSA_H__

#include "bignum.h"

void keygen(bignum * n, bignum * e, bignum * d, int length);
bignum RSAencrypt(bignum m, bignum e, bignum n);
bignum RSAdecrypt(bignum c, bignum d, bignum n);
void testRSA(int length);

#endif

