#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h> // For SHA-NI instructions

#define SHA_DATA_SIZE (4096) // 4KB aligned data size

extern void sha256_ni_transform(uint32_t *digest, void *data, uint32_t numBlocks);

unsigned long rdtsc() {
    unsigned long lo, hi;
    __asm__ __volatile__ (
        "rdtsc" : "=a"(lo), "=d"(hi)
    );
    return (hi << 32) | lo;
}

int main() {
    uint32_t digest[8] = {0}; // Placeholder for the digest
    uint8_t *data = (uint8_t *)aligned_alloc(4096, SHA_DATA_SIZE); // Allocate 4KB aligned memory
    if (!data) {
        perror("Failed to allocate memory");
        return EXIT_FAILURE;
    }
    
    // Fill data with random bytes
    for (size_t i = 0; i < SHA_DATA_SIZE; ++i) {
        data[i] = rand() % 256;
    }

    uint32_t numBlocks = SHA_DATA_SIZE / 64; // Number of 512-bit blocks
    unsigned long t1, t2;

    // Call the assembly function
    t1 = rdtsc();
    sha256_ni_transform(digest, data, numBlocks);
    t2 = rdtsc();

    printf("SHA256 computation time (cycles): %lu\n", (t2 - t1)*5/12);
    
    // Print the resulting digest
    printf("SHA256 Digest: %08x %08x %08x %08x %08x %08x %08x %08x\n",
           digest[0], digest[1], digest[2], digest[3],
           digest[4], digest[5], digest[6], digest[7]);

    free(data); // Free allocated memory
    return 0;
}

