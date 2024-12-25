#include <stdio.h>
#include <stdlib.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}
void handleErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}

int main() {
    int ret;
    RSA *rsa = NULL;
    BIGNUM *bne = NULL;
    unsigned char *plaintext = (unsigned char *)"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    unsigned char encrypted[384];  // 3072 bits = 384 bytes
    unsigned char decrypted[384];  // 3072 bits = 384 bytes
    int encrypted_length, decrypted_length;
    
    // Generate RSA Key
    int bits = 3072;
    unsigned long e = RSA_F4;  // 65537

    bne = BN_new();
    ret = BN_set_word(bne, e);
    if (ret != 1) {
        handleErrors();
    }

    rsa = RSA_new();
    ret = RSA_generate_key_ex(rsa, bits, bne, NULL);
    if (ret != 1) {
        handleErrors();
    }

    // Save private key
    FILE *priv_file = fopen("private_key.pem", "wb");
    if (!priv_file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    PEM_write_RSAPrivateKey(priv_file, rsa, NULL, NULL, 0, NULL, NULL);
    fclose(priv_file);

    // Save public key
    FILE *pub_file = fopen("public_key.pem", "wb");
    if (!pub_file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    PEM_write_RSA_PUBKEY(pub_file, rsa);
    fclose(pub_file);

    // Encrypt plaintext
    unsigned long t1,t2,t_rsa;
    t1=urdtsc();
    encrypted_length = RSA_public_encrypt(strlen((char *)plaintext), plaintext, encrypted, rsa, RSA_PKCS1_OAEP_PADDING);
    t2=urdtsc();
    t_rsa = (t2-t1)*5/17;
    printf("rsa time:%ld\n",t_rsa);
    if (encrypted_length == -1) {
        handleErrors();
    }
    
    printf("Encrypted message:\n");
    for (int i = 0; i < encrypted_length; i++) {
        printf("%02x ", encrypted[i]);
    }
    printf("\n");

    // Decrypt message
    decrypted_length = RSA_private_decrypt(encrypted_length, encrypted, decrypted, rsa, RSA_PKCS1_OAEP_PADDING);
    if (decrypted_length == -1) {
        handleErrors();
    }

    printf("Decrypted message: %.*s\n", decrypted_length, decrypted);

    // Cleanup
    RSA_free(rsa);
    BN_free(bne);

    return 0;
}

