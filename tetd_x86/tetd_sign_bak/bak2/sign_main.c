#include "bignum.h"

extern void sha256_ni_transform(uint8_t *hash, const uint8_t *data, size_t len);

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


static const char *hex_e = "010001";

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

/* Forward declarations of original bignum ops */
bignum mult(bignum a, bignum b);
bignum add(bignum a, bignum b);
bignum divi(bignum a, bignum b);
bignum reminder(bignum a, bignum n);
bignum expmod(bignum a, bignum b, bignum n);
bignum digit2bignum(int digit);
void copy(bignum *dst, bignum src);

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
    for (size_t i = 0; i < len; i++){
        printf("%02x", buf[i]);
    }
    printf("\n");
}

static void print_hash(uint8_t *hash) {
    for (int i = 0; i < 32; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

void hash_sign(uint8_t *input, uint64_t input_len, uint8_t *output){
    uint8_t hash[32] = {0};
    sha256_ni_transform(hash, input, input_len/64);
    printf("SHA-256 hash:");
    print_hash(hash);
    
    /* Key size in bytes */
    size_t key_bytes = strlen(hex_n) / 2;
    
    /* Build big‐ints n, e, d */
    uint8_t *tmp = malloc(key_bytes);
    bignum n, e, d;
    hex2bin(hex_n, tmp, key_bytes);
    n = bytes_to_bignum(tmp, key_bytes);
    hex2bin(hex_e, tmp, strlen(hex_e)/2);
    e = bytes_to_bignum(tmp, strlen(hex_e)/2);
    hex2bin(hex_d, tmp, key_bytes);
    d = bytes_to_bignum(tmp, key_bytes);
    free(tmp);

    /* Plaintext -> m */
    bignum m = bytes_to_bignum(hash, 32);
    
    /* Sign: sig = m^d mod n */
    bignum sig = expmod(m, d, n);
    bignum_to_bytes(sig, output, key_bytes);
    print_hex("Signature", output, key_bytes);
    
    free(n.tab);
    free(e.tab);
    free(d.tab);
    free(m.tab);
}

int main(){
    uint64_t input_len = 4096;
    uint8_t *input = (uint8_t *)malloc(input_len);
    uint8_t *output = (uint8_t *)malloc(4096);
    for (int i = 0; i < input_len; i++){
        input[i] = rand() % 256;
    }
    hash_sign(input,input_len,output);
    free(input);
    free(output);
    return 0;
}

