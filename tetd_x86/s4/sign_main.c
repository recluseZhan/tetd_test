/*
 * RSA-3072 signature using Montgomery multiplication and CRT optimization
*/

#include "bignum.h"

#define RSA_KEY_BITS 3072
#define RSA_KEY_BYTES (RSA_KEY_BITS / 8)

typedef uint8_t u8;

// Because CRT, two LUTs of dp/dq (3072 / 2 + 1)
#define LUT_SIZE 1537
static struct bn lut[LUT_SIZE];

extern void sha256_ni_transform(uint8_t *hash, const uint8_t *data, uint64_t len);

/* Key parameters in hex */
// modulus n (3072‑bit)
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

// public exponent e
static const char *hex_e = "010001";

// private exponent d
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

// prime1 p
static const char *hex_p =
  "ea5640bbc5535566424d91d3f7421b09fb05702a43e4efa31e6cd1947edb20b3"
  "3b564182ce682a42336dc4ec11bd3adcb355f372e436df97d888888c242360eb"
  "fdd75cc3edd9c3bf51d215c0aabc821f984305783c5677cf2f1ab6ae06907544"
  "8ed55cbea218ee6378253e80ea3f4af62e4eaeb48ab33f19f7e199d51cc05f92"
  "71037972edfeb2922439718a42cc4187c88b7c31d44608a5f641473ea3bbbaea"
  "3338d43059f3de1612cd19058d85b470b3c95824cf659eb897f93b110f1bd3d9";

// prime2 q
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

static void print_hash(u8 *hash) {
    for (int i = 0; i < 32; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

/* Utility: print byte array as hex string */
void print_hex(const u8 *data, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

void init_lut(struct bn *x, struct bn *m, int mBits, struct bn *r2m) {
    montMult(x, r2m, m, mBits, &lut[0]);
    for (int i = 1; i <= mBits; i++) {
        montMult(&lut[i-1], &lut[i-1], m, mBits, &lut[i]);
    }
}

/* Montgomery multiplication (X * Y * R^{-1} mod M) */
void montMult(struct bn* x, struct bn* y, struct bn* m, int mBits, struct bn* out){
    struct bn t;
    bignum_init(&t);
    for(int i = mBits; i > 0; i--){
        int t0 = bignum_getbit(&t, 0);
        int xi = bignum_getbit(x, mBits - i);
        int y0 = bignum_getbit(y, 0);
        if(xi) bignum_add(&t, y, &t);
        if(t0 ^ (xi & y0)) bignum_add(&t, m, &t);
        bignum_rshift(&t, &t, 1);
    }
    if(bignum_cmp(&t, m) >= 0) bignum_sub(&t, m, &t);
    bignum_assign(out, &t);
}

/* Modular exponentiation using Montgomery reduction */
void modExp(struct bn* x, struct bn* e, int eBits, struct bn* m, int mBits, struct bn* r2m, struct bn* out){
    struct bn z, p1, p2, z1, z2, tmp;
    bignum_from_int(&z, 1);
    montMult(&z, r2m, m, mBits, &z1);
    montMult(x, r2m, m, mBits, &p1);
    for(int i = 0; i < eBits; i++){
        montMult(&p1, &p1, m, mBits, &p2);
        if(bignum_getbit(e, i)) montMult(&z1, &p1, m, mBits, &z2);
        else bignum_assign(&z2, &z1);
        bignum_assign(&p1, &p2);
        bignum_assign(&z1, &z2);
    }
    bignum_from_int(&tmp, 1);
    montMult(&z1, &tmp, m, mBits, out);
}

/* Mod Exp using precomputed LUT */
void modExpLUT(struct bn* x, struct bn* e, int eBits, struct bn* m, int mBits, struct bn* r2m, struct bn* out){
	struct bn z,one;
	struct bn parr[3];
	struct bn zarr[3];
	int b = 1;
	int i = 0;

	bignum_from_int(&z, 1);
	montMult(&z,r2m,m, mBits, &zarr[1]);
	bignum_assign(&parr[1],&lut[0]);

	for(; i < eBits; i++){
	    bignum_assign(&parr[2],&lut[i+1]);
	    if(bignum_getbit(e, i) == 1){
	        montMult(&zarr[1],&lut[i],m,mBits,&zarr[2]);
	    }else{
		bignum_assign(&zarr[2],&zarr[1]);
	    }
	    bignum_assign(&parr[1], &parr[2]);
	    bignum_assign(&zarr[1], &zarr[2]);
       	    b++;
	}
	bignum_from_int(&one, 1);
	montMult(&zarr[1], &one, m, mBits, out);
}

/* Load big number from hex string */
static void hex_to_bn(struct bn *n, const char *hex){
    bignum_from_string(n, (char*)hex, strlen(hex));
}

/* Convert big-endian byte array to bn */
static void bytes_to_bn(struct bn *n, const u8 *in, uint64_t in_len){
    bignum_init(n);
    for(int i = 0; i < in_len; i++){
        int b = in[in_len - 1 - i];
        int w = i / WORD_SIZE;
        int off = i % WORD_SIZE;
        n->array[w] |= ((DTYPE)b) << (8 * off);
    }
}

/* Convert bn to big-endian byte array */
static void bn_to_bytes(const struct bn *n, u8 *out, uint64_t in_len){
    for(int i = 0; i < in_len; i++){
        int w = i / WORD_SIZE;
        int off = i % WORD_SIZE;
        out[in_len - 1 - i] = (u8)((n->array[w] >> (8 * off)) & 0xFF);
    }
}

/* RSA-3072 signature using CRT optimization */
int rsa_sign(const u8 *input, u8 *output, uint64_t input_len){
    struct bn n, p, q, dp, dq, qinv;
    struct bn m, m1, m2, h, tmp;
    struct bn r2p, r2q, R, R2;
    u8 hash[32];
    
    sha256_ni_transform(hash, input, input_len/64);
    printf("SHA-256 hash:");
    print_hash(hash);

    /* Load key parameters */
    hex_to_bn(&n, hex_n);
    hex_to_bn(&p, hex_p);
    hex_to_bn(&q, hex_q);
    hex_to_bn(&dp, hex_dp);
    hex_to_bn(&dq, hex_dq);
    hex_to_bn(&qinv, hex_qinv);

    /* Compute bit lengths */
    int pBits = bignum_numbits(&p);
    int qBits = bignum_numbits(&q);

    /* R^2 mod p */
    bignum_from_int(&R, 1);
    bignum_lshift(&R, &R, pBits);
    bignum_mul(&R, &R, &R2);
    bignum_mod(&R2, &p, &r2p);

    /* R^2 mod q */
    bignum_from_int(&R, 1);
    bignum_lshift(&R, &R, qBits);
    bignum_mul(&R, &R, &R2);
    bignum_mod(&R2, &q, &r2q);

    /* Load message */
    bytes_to_bn(&m, hash, 32);

    // Init p LUT
    init_lut(&m, &p, pBits, &r2p);
    /* Compute m1 = m^dp mod p */
    //modExp(&m, &dp, pBits, &p, pBits, &r2p, &m1);
    modExpLUT(&m, &dp, pBits, &p, pBits, &r2p, &m1);
    
    // Init q LUT
    init_lut(&m, &q, qBits, &r2q);
    /* Compute m2 = m^dq mod q */
    //modExp(&m, &dq, qBits, &q, qBits, &r2q, &m2);
    modExpLUT(&m, &dq, qBits, &q, qBits, &r2q, &m2);

    /* h = (m1 - m2) mod p */
    if(bignum_cmp(&m1, &m2) < 0){
        bignum_add(&m1, &p, &tmp);
        bignum_sub(&tmp, &m2, &h);
    } else {
        bignum_sub(&m1, &m2, &h);
    }

    /* h = h * qinv mod p */
    bignum_mul(&h, &qinv, &tmp);
    bignum_mod(&tmp, &p, &h);

    /* s = m2 + h * q */
    bignum_mul(&h, &q, &tmp);
    bignum_add(&m2, &tmp, &tmp);

    /* Output signature */
    bn_to_bytes(&tmp, output, RSA_KEY_BYTES);
     
    /* Print signature in hex */
    printf("Signature: ");
    for(int i = 0; i < RSA_KEY_BYTES; i++){
        printf("%02x", output[i]);
    }
    printf("\n");

    return 0;
}

int main() {
    uint64_t input_len = 4096;
    u8 *input = (uint8_t *)malloc(input_len);
    u8 *output = (uint8_t *)malloc(RSA_KEY_BYTES);
    for (int i = 0; i < input_len; i++){
        input[i] = rand() % 256;
    }
    rsa_sign(input, output, input_len);

    free(input);
    free(output);
    return 0;
}
