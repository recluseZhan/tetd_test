// rsa.c
#include "bignum.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * -------------- 1) 在这里粘入你的 RSA-3072 参数 hex_n / hex_e / hex_d --------------
 * hex_n, hex_d 都应是 768 字节 (3072 bits) 的十六进制字符串 (1536 hex chars).
 * hex_e 通常是 "010001" (65537).
 */
static const char *hex_n =
  "DC07AC26074D3715562874C08E088ACC8E01A4729FD23818AA5E415B37C70AF70"
  "D1DF8F9D32BAE8554717F50F4E13C5E7ED1EBD1F0E1C3F1FC6EEDF2A9212B728"
  "2625BF552D73B99E07926AF7896BE6F22C931CF815411330C3F778CAF4001E0B"
  "82B22215CB1F63C0942E29EFE5A637C5E4221211B625330C112E037A5B010067"
  "49340E9054A7D97D2FBEBC129429E952075D29070970731A0333682D68DB466"
  "2A66C354E83F0F6AC12597CA4D4B1BC3864C5AA92633BE1EA2E38B03F3FB887D"
  "38E2E32F76B29ED190CEEA60268DDD7D676A840EB8EC0F8A0B6EE85960726737"
  "528BF958BD9643C73F79FD5D115E3405FCF3EEAE01A192C18EBB8A195ABD75DA"
  "6379AFF3E57E8A0C6622EA6385743F258DDA5EBC576F4884FF4EC11DF5D5A157"
  "2C397FCF3796E7C7FC6601CD6E9B78D79E1E4ACE1F2FBCC7E27C9A3C99756DEAC"
  "CD83AAA8B46DBAF3B44B49383F5D6D15A3AE02B99971F46677558CA144C9B421"
  "9DD6CB75793599899C81F3DD1524E3413779C573FDF161828EA0CC56D07FE51D"
  "61D3B5E5";

static const char *hex_e = "10001";

static const char *hex_d =
  "0BD685D68DF20392AC4E04EA0F3A3A6D82FE67A09B24AE4BF56E98C99598A9E855"
  "CD64DFC F2F598B027010CF843D52082B7E93D779C44BCC8B2E54B307927749F3E"
  "2D8B21A4E649BB0529A5AB314111AE2E4412BCE2016C503ED24AE911D08C01FE"
  "1F8358A859AF94C95F0C3F28D03B5FF68CDD C1216B24666059638DDDE1EDACAC"
  "96540199FE4393FC09165ECE5A75455C27C6EF2513AC4499BD17110012C2A469"
  "A9FC8D70BB74CA8FF1FAD44EB3B7BD043D871EBBAC0DF36AAD60D9C6CDD7FDFEE"
  "B98DCDB674D8A3FDE80B27943EB4D881511F99584A52687521410BD3D268224EC"
  "D240778CB7C9399944F91A0A618D3093775E728C572A353B526BB29CF349DAF1C"
  "B6A6F288017DA79D5DB0D602AE3CBDD2B814C43273B5D70642811FDED4618EF4"
  "D512A39DCC44FCF08E60F9EC90C08C08F1520F55CDB280AA477768583FFF55081"
  "C9FC37FD30433389405CD3E69B10F4B4EFFAABD5D936151022D88408AD1C17221"
  "FBEE6DBEDBA1F707E97627F58F3B6E579C36AC425CECEDD8B3F0E4BB3328C9";
/*
 * -------------- 2) 在这里粘入你的明文 ----------------
 * 以字节形式嵌入，len 可任意。下面示例是 ASCII "Hello".
 */
uint8_t input[32];
uint64_t input_len = 32;
/* Forward declarations of original bignum ops */
bignum mult(bignum a, bignum b);
bignum add(bignum a, bignum b);
bignum divi(bignum a, bignum b);
bignum reminder(bignum a, bignum n);
bignum expmod(bignum a, bignum b, bignum n);
bignum digit2bignum(int digit);
void   copy(bignum *dst, bignum src);

/* Convert a small uint32_t into a bignum */
static bignum u32_to_bignum(uint32_t v) {
    bignum r;
    r.sign = 1;
    if (v == 0) {
        r.size = 1;
        r.tab = malloc(sizeof(integer));
        r.tab[0] = 0;
        return r;
    }
    // base is B (from bignum.h)
    r.tab = malloc(sizeof(integer) * ((32 / E) + 2));
    r.size = 0;
    while (v) {
        r.tab[r.size++] = v % B;
        v /= B;
    }
    return r;
}

/* Hex string → big‐endian bytes */
static void hex2bin(const char *hex, uint8_t *out, size_t outlen) {
    for (size_t i = 0; i < outlen; i++) {
        unsigned byte;
        if (sscanf(hex + 2*i, "%2x", &byte) != 1) {
            fprintf(stderr, "hex parse error at %zu\n", i);
            exit(1);
        }
        out[i] = byte;
    }
}

/* input bytes[u..u+len) → bignum */
static bignum bytes_to_bignum(const uint8_t *bytes, size_t len) {
    bignum r = digit2bignum(0);
    bignum b256 = u32_to_bignum(256);
    for (size_t i = 0; i < len; i++) {
        bignum tmp = r;
        r = mult(r, b256);
        free(tmp.tab);

        bignum ad = u32_to_bignum(bytes[i]);
        tmp = r;
        r = add(r, ad);
        free(tmp.tab);
        free(ad.tab);
    }
    free(b256.tab);
    return r;
}

/* bignum → output[len] big‐endian */
static void bignum_to_bytes(bignum x, uint8_t *out, size_t len) {
    bignum b256 = u32_to_bignum(256);
    for (size_t i = 0; i < len; i++) out[i] = 0;
    for (size_t pos = len; pos > 0; pos--) {
        bignum q = divi(x, b256);
        bignum r = reminder(x, b256);
        out[pos-1] = r.tab[0];
        free(x.tab);
        free(r.tab);
        x = q;
    }
    free(x.tab);
    free(b256.tab);
}

/* Print hex */
static void print_hex(const char *label, const uint8_t *buf, size_t len) {
    printf("%s:", label);
    for (size_t i = 0; i < len; i++) printf("%02x", buf[i]);
    printf("\n");
}
int main(void) {
    /* Key size in bytes */
    size_t key_bytes = strlen(hex_n) / 2;

    /* 1) Build big‐ints n, e, d */
    uint8_t *tmp = malloc(key_bytes);
    bignum n, e, d;
    hex2bin(hex_n, tmp, key_bytes); n = bytes_to_bignum(tmp, key_bytes);
    hex2bin(hex_e, tmp, strlen(hex_e)/2); e = bytes_to_bignum(tmp, strlen(hex_e)/2);
    hex2bin(hex_d, tmp, key_bytes); d = bytes_to_bignum(tmp, key_bytes);
    free(tmp);

    /* 2) Plaintext → m */
    //uint8_t input[32];
    //uint64_t input_len = 32;
    //memset(input,1,input_len);
    for (int i = 0; i < input_len; i++)input[i] = rand() % 256;
    
    bignum m = bytes_to_bignum(input, input_len);
    print_hex("Input", input, input_len);

    /* 3) Sign: sig = m^d mod n */
    bignum sig = expmod(m, d, n);
    uint8_t *out = malloc(key_bytes);
    bignum_to_bytes(sig, out, key_bytes);
    print_hex("Signature", out, key_bytes);

    /* 4) Verify: rec = sig^e mod n */
    bignum rec = expmod(sig, e, n);

    /* 5) Compare m vs rec directly */
    if (m.size == rec.size &&
        memcmp(m.tab, rec.tab, m.size * sizeof(integer)) == 0)
    {
        printf("[+] Verification SUCCESS\n");
    } else {
        printf("[-] Verification FAIL\n");
    }

    /* cleanup */
    //free(out);
    //free(sig.tab);
    //free(rec.tab);
    //free(m.tab);
    //free(d.tab);
    //free(e.tab);
    //free(n.tab);

    return 0;
}

