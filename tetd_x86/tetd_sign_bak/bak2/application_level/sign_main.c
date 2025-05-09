#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/sha.h>

extern void sha256_ni_transform(uint8_t *state, const uint8_t *data, size_t len);

void print_hash(uint8_t *hash) {
    for (int i = 0; i < 32; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

int sign_with_rsa(RSA *rsa, const uint8_t *hash, uint8_t *signature, unsigned int *sig_len) {
    return RSA_sign(NID_sha256, hash, SHA256_DIGEST_LENGTH, signature, sig_len, rsa);
}

int verify_signature(RSA *rsa, const uint8_t *hash, const uint8_t *signature, unsigned int sig_len) {
    return RSA_verify(NID_sha256, hash, SHA256_DIGEST_LENGTH, signature, sig_len, rsa);
}

RSA *load_private_key(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Unable to open private key file");
        return NULL;
    }
    RSA *rsa = PEM_read_RSAPrivateKey(file, NULL, NULL, NULL);
    fclose(file);
    return rsa;
}

RSA *load_public_key(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Unable to open public key file");
        return NULL;
    }
    RSA *rsa = PEM_read_RSA_PUBKEY(file, NULL, NULL, NULL);
    fclose(file);
    return rsa;
}

void print_signature(unsigned char *signature, size_t sig_len) {
    printf("Signature: ");
    for (size_t i = 0; i < sig_len; i++) {
        printf("%02x", signature[i]);
    }
    printf("\n");
}

int main() {
    const char *message = "Hello, world!";
    uint8_t hash[32] = {0};
    uint8_t data[64] = {0};
    uint8_t signature[4096];
    unsigned int sig_len;

    // Calculate the hash value.
    memcpy(data, message, strlen(message));
    sha256_ni_transform(hash, data, strlen(message));

    printf("SHA-256 hash of '%s': ", message);
    print_hash(hash);

    // RSA signature.
    RSA *rsa_priv = load_private_key("private_key.pem");
    if (!rsa_priv) {
        RSA_free(rsa_priv);
        return 1;
    }
    
    RSA *rsa_pub = load_public_key("public_key.pem");
    if (!rsa_pub) {
        RSA_free(rsa_pub);
        return 1;
    }

    if (sign_with_rsa(rsa_priv, hash, signature, &sig_len) != 1) {
        fprintf(stderr, "Error signing hash: %s\n", ERR_error_string(ERR_get_error(), NULL));
        RSA_free(rsa_priv);
        return 1;
    }

    printf("Signature generated successfully.\n");
    print_signature(signature, sig_len);

    // RSA signature verification.
    if (verify_signature(rsa_pub, hash, signature, sig_len) != 1) {
        fprintf(stderr, "Signature verification failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
        RSA_free(rsa_pub);
        return 1;
    }

    printf("Signature verified successfully.\n");

    RSA_free(rsa_priv);
    RSA_free(rsa_pub);
    return 0;
}

