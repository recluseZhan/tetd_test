// sign_main.c
// 完整版：包含 rsa_sign_crt 实现 + 主程序，无任何省略

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "bignum.h"

extern void sha256_ni_transform(uint8_t *hash, const uint8_t *data, size_t len);

// modulus n
static const char *hex_n =
  "be18818a7e95c519e0be692f0ca7f4a6da1522e2d40f0a256ee5d073d9230215"
  "b4566c6951a70b3b667ab7f893f6bbff3ac88173694d6c31cee8c2f263772720"
  "080b44656c690ef50fd579da476e2dc76fd4be713b68854b645828130aae5e57"
  "da604c45811fff3f368405d8ef8fbaf3904e934d55a273cf607b3f7b48dcc9ea"
  "6886f62091da7a68368d98685152dfe30927c18df8ab072ef2d81d8d44e92e8e"
  "f5a385ee45bdab8cd0b6588b53b5e6644eae3dc175ee58081a8099b66f1d0437"
  "61e3cdb49e4a9402890830395264dcb65127630a53550bb0a3e942c6644267b7"
  "5cd33bf1cb8d8fbcfb1b88ced120747e3ad90881cc7b2d2c8f0c1e5423321482"
  "79d09649c03270bcb9632c4d51fd1a7a168208eb1d6d04f94da86db850396281"
  "f31813510f333b68c4f3c1fdf9090291ac9f80d4bd440fae5c1e47220ddd2a68"
  "47158670a476ec0be66150316a847240fa4bbeb9a894d7f90fa18275dbc7f0e9"
  "e8dbb031456ab60f0495cb3db4c80d6cb73671ba617f812479c50212ccca9e67";

// public exponent
static const char *hex_e = "010001";

// private exponent
static const char *hex_d =
  "0e84373ae8ee6da8cb92d43175994b2e300536841fdc2e282c4f271b51aa420f"
  "2a40574860fb37f90745b9257b1722701bd5bd479f9e51497f1acdeda34edc11"
  "597692cddea37f32e8f74d0b85879268b7cf8538dc67df95f6f461cb0d2117af"
  "44768a1c875476dcd040460f5f9698438a9cead4185aaed6ad457c298bfce051"
  "22e8fb84028a03eda5622c10a5f5303259914c4f427d67afb177c8cf459890fc"
  "f5cbd0ff20704583715a1b6a385af1b1c34311bae3a899d709e9bc2899aa4642"
  "055a05f565326f04508c4be52c49496c291f542e81b175bcb0e03bf274a5a265"
  "af1923350f419b00bce8389bc67f30a732dc90d09be7c89436a5df7301b668cb"
  "f1090a89c7c48cbc3a433a326916f842d7dcbefb834b8d74775909ed7d0a8a9e"
  "b1a2636a6d134220695a9ca6e03a363317b6d2df11c08c9ff9326dd47df90efa"
  "df8be58e05b0ae1e95c32238871239259e240fb8a6ffd83b49b0a5bf75d15413"
  "cfd8ea2166638580ccf32c5740a4b5f23e1b213d3775d919967198222475e2a1";


// p
static const char *hex_p =
  "ea5640bbc5535566424d91d3f7421b09fb05702a43e4efa31e6cd1947edb20b3"
  "3b564182ce682a42336dc4ec11bd3adcb355f372e436df97d888888c242360eb"
  "fdd75cc3edd9c3bf51d215c0aabc821f984305783c5677cf2f1ab6ae06907544"
  "8ed55cbea218ee6378253e80ea3f4af62e4eaeb48ab33f19f7e199d51cc05f92"
  "71037972edfeb2922439718a42cc4187c88b7c31d44608a5f641473ea3bbbaea"
  "3338d43059f3de1612cd19058d85b470b3c95824cf659eb897f93b110f1bd3d9";

// q
static const char *hex_q =
  "cfab4117838cdefc5410c543c1b942e97c9753de4b1e05b567e8292bdd257133"
  "51aa6b8109f7e4491944ca9a99b968b48c78040cb8d87bc1271422ea9e3d309e"
  "bed4965f8b4bcb32b8ed3daeb0639dd1bd63986410a3a8c06e8c034c5c6a06da"
  "e4bbdb307ee2c198b69afc67901d0a19b51f00ae0723ce9befe56d476d6f794d"
  "55c67e61f9c8bf76c3774b02c1005f3efa1465359b9b10e97da33c44703da11f"
  "1ef0455c74acf7dffe3b7be8711ffa2ae678d48b8b0e4b9e8bccbc9e8b1bdc3f";

// dp = d mod (p-1)
static const char *hex_dp =
  "80f75f40f7e45907a24dd687f578683121f968359a901062918809ffd3a0fd6a"
  "26a9ddfc0364c87d0e98c6dbb51793a18b012eae7872cab362f421521f416941"
  "f7bde2cbe1c70b37ebebd714e5e2412aea3d587147a3bee443644c4d92474682"
  "a8a93c159e58623a54110b491fa21dbb46d8fec555e038462e69f6a396e338b5"
  "fb8491fe7284cdfa27fd85433667fb58a34240e39b93bf86b6f44e25672f163b"
  "765d3c4a2ff74aae93425516fd402c822a30115690ddb87097130f32e041cc61";

// dq = d mod (q-1)
static const char *hex_dq =
  "39d37d0f9820fce294620b1bfff09f7236b048b9487f76a579e68ab85d1f14f3"
  "6e551f14527b458519552f79773ecaaa23c733917b5344ba9730233ce38d3461"
  "afcfb083b05d0d9af525929c771f760db647ea624ac3d7b4a5d3b503696458be"
  "511d023ef1c620946e9c9f9612ea132aef654fa225f8d18a5875b1454772d7fb"
  "31dc50cae56c01ec87274baadba547e058709f721aff45e94e83cabf5fe5b95c"
  "da179ba8d0106e0160609df32eb4f1cb4a5bf10b5e503fbc493f5c726557ffaf";

// qinv = q^(-1) mod p
static const char *hex_qinv =
  "ae172813b4ef72775c2798b194348355cb7e7f74947d364b83f6e296e9e00559"
  "efb482990ad4fb335b9cb6ed7c68d5a19d244bade44292d94f33010f3815a991"
  "c0211990a002ef686c35ecd3e0602129bb1e35bf9cd5663b52ce9c97154fcfca"
  "8d5403e6c59de8e785ee64b3ab6b7593f0c899ea9f823f22b5cbeb766b8fbe8b"
  "232143dd9a28c0f0550b87fedb19cdf52e02009773b9cdc722cace719b3665d9"
  "3bf21dadac7d5669bbfb14000ef48f66c174bbc24739987c3a6b20eb9da62152";
// =======================
// 2) CRT 签名函数定义
// =======================
unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}

// sign_main.c
// … your #includes, key hexs, print_hex, etc …

//extern bignum reminder(bignum a, bignum n);  // from bignum.c

// Replace your old rsa_sign_crt with this:

void rsa_sign_crt(const mont_ctx *Mbig,
                  const bignum  *m,      // plaintext message int
                  const bignum  *p,      // prime1
                  const bignum  *q,      // prime2
                  const bignum  *dp,     // d mod (p-1)
                  const bignum  *dq,     // d mod (q-1)
                  const bignum  *qinv,   // q^{-1} mod p
                  bignum        *sig)    // out: signature mod n
{
    // 1) build Montgomery contexts for p and q
    mont_ctx Mp, Mq;
    mont_init(&Mp, p);
    mont_init(&Mq, q);

    // 2) compute “one” and get R mod p,q
    bignum one; bn_init(&one); one.tab[0] = 1;

    // 3) reduce m mod p via Montgomery:  m1R = m * R mod p
    bignum m1R; bn_init(&m1R);
    mont_mul(&Mp, m, &Mp.r2, &m1R);

    //    then convert back to plain to get m mod p:  m1 = m1R * 1 * R^{-1} mod p = m mod p
    bignum m1; bn_init(&m1);
    mont_mul(&Mp, &m1R, &one, &m1);

    // 4) similarly reduce m mod q
    bignum m2R; bn_init(&m2R);
    mont_mul(&Mq, m, &Mq.r2, &m2R);
    bignum m2; bn_init(&m2);
    mont_mul(&Mq, &m2R, &one, &m2);

    // 5) compute s1 = m1^dp mod p    (Montgomery exp returns plain)
    bignum s1; bn_init(&s1);
    // m1 needs to be lifted into Mont domain for exponent:
    bignum m1R2; bn_init(&m1R2);
    mont_mul(&Mp, &m1, &Mp.r2, &m1R2);
    mont_exp(&Mp, &m1R2, dp, &s1);

    // 6) compute s2 = m2^dq mod q
    bignum s2; bn_init(&s2);
    bignum m2R2; bn_init(&m2R2);
    mont_mul(&Mq, &m2, &Mq.r2, &m2R2);
    mont_exp(&Mq, &m2R2, dq, &s2);

    // 7) diff = (s1>=s2 ? s1-s2 : s1 + p - s2)
    bignum diff, tmp; bn_init(&diff); bn_init(&tmp);
    if (bn_cmp(&s1, &s2) >= 0) {
        bn_sub(&s1, &s2, &diff);
    } else {
        bn_add(&s1, p, &tmp);
        bn_sub(&tmp, &s2, &diff);
    }

    // 8) h = diff * qinv mod p
    bignum h; bn_init(&h);
    mont_mul(&Mp, &diff, qinv, &h);

    // 9) qh = h * q  (Montgomery on q)
    bignum qh; bn_init(&qh);
    mont_mul(&Mq, &h, q, &qh);

    // 10) recombine: sig = s2 + qh
    bn_add(&s2, &qh, sig);

    // 11) if sig ≥ n, subtract n once
    if (bn_cmp(sig, &Mbig->n) >= 0) {
        bignum t; bn_init(&t);
        bn_sub(sig, &Mbig->n, &t);
        bn_copy(sig, &t);
    }
}


// =======================
// 3) 辅助：打印 hex
// =======================
static void print_hex(const char *label, const uint8_t *buf, size_t len) {
    printf("%s:", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", buf[i]);
    }
    printf("\n");
}

// =======================
// 4) 主程序
// =======================
// … your usual #includes, hex_N/E/P/Q/… definitions …
// helper to print debug markers
#define DBG(...)  do { printf(__VA_ARGS__); fflush(stdout); } while(0)
int main(void) {
    unsigned long t1,t2,t_all=0;
    DBG("[+] start\n");

    // 1) parse key hex → bignums
    bignum bn_n, bn_e, bn_p, bn_q, bn_dp, bn_dq, bn_qinv;
    bn_from_hex(&bn_n,   hex_n);   DBG("[got n, size=%zu]\n", bn_n.size);
    bn_from_hex(&bn_e,   hex_e);   DBG("[got e, size=%zu]\n", bn_e.size);
    bn_from_hex(&bn_p,   hex_p);   DBG("[got p, size=%zu]\n", bn_p.size);
    bn_from_hex(&bn_q,   hex_q);   DBG("[got q, size=%zu]\n", bn_q.size);
    bn_from_hex(&bn_dp,  hex_dp);  DBG("[got dp, size=%zu]\n", bn_dp.size);
    bn_from_hex(&bn_dq,  hex_dq);  DBG("[got dq, size=%zu]\n", bn_dq.size);
    bn_from_hex(&bn_qinv,hex_qinv);DBG("[got qinv, size=%zu]\n",bn_qinv.size);

    // 2) init Montgomery for n
    DBG("[+] before mont_init]\n");
    mont_ctx M; mont_init(&M, &bn_n);
    DBG("[+] after mont_init]\n");

    // 3) prepare+hash message
    DBG("[+] about to hash message]\n");
    size_t msg_len = 4096;
    uint8_t *msg = malloc(msg_len);
    for (size_t i = 0; i < msg_len; i++) msg[i] = rand() % 256;
    uint8_t hash[32]={0};
    sha256_ni_transform(hash, msg, msg_len/64);
    printf("hash[0]=%lx\n",hash[0]);
    DBG("[+] hash done]\n");

    // 4) build m = integer(hash)
    DBG("[+] about to build m]\n");
    bignum m; bn_init(&m);
    for (int i = 0; i < 32; i++) {
        //  m = m*256 + hash[i]
        bignum limb, t; bn_init(&limb); limb.tab[0] = hash[i];
        mont_mul(&M, &m, &M.r2, &t);
        mont_mul(&M, &t, &limb, &m);
    }
    // out of Montgomery
    { bignum one; bn_init(&one); one.tab[0] = 1; mont_mul(&M, &m, &one, &m); }
    DBG("[+] built m (size=%zu)\n", m.size);

    // 5) CRT sign
    DBG("[+] about to CRT sign]\n");
    bignum sig; bn_init(&sig);
    t1=urdtsc();
    rsa_sign_crt(&M, &m, &bn_p,&bn_q,&bn_dp,&bn_dq,&bn_qinv,&sig);
    t2=urdtsc();
    printf("time=%lld\n",(t2-t1)*5/12);
    DBG("[+] CRT done (sig size=%zu)\n", sig.size);

    // 6) print signature
    DBG("[+] about to output signature]\n");
    uint8_t out[384];
    bn_to_bytes(&sig, out, 384);
    print_hex("Signature", out, 384);

    // 7) verify
    DBG("[+] about to verify]\n");
    bignum rec; bn_init(&rec);
    mont_exp(&M, &sig, &bn_e, &rec);
    if (bn_cmp(&rec, &m) == 0) {
        printf("[+] Verification OK\n");
    } else {
        printf("[-] Verification FAIL\n");
    }
    //

    //
    DBG("[+] all done]\n");
    free(msg);
    return 0;
}
