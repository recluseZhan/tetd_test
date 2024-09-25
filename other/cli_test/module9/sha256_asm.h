#ifndef SHA256_ASM_H
#define SHA256_ASM_H

extern void sha256_ni_transform(uint32_t *digest, const void *data, uint32_t numBlocks);

#endif /* SHA256_ASM_H */

