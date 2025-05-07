// File: aes_gcm_module.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cpufeature.h>
#include <linux/types.h>
//#include "aes_gcm_asm.h"
extern void aes_gcm_encrypt(const u8 *plaintext, const u8 *key, const u8 *iv, u8 *ciphertext, u8 *tag);

static int __init aes_gcm_init(void)
{
    u8 key[16] = {
        0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B, 0x0C,0x0D,0x0E,0x0F
    };
    u8 iv[12] = {
        0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17,
        0x18,0x19,0x1A,0x1B
    };
    u8 pt[16] = {
        0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B, 0x0C,0x0D,0x0E,0x0F
    };
    u8 ct[16], tag[16];

    pr_info("aes_gcm: Key: %*phN\n", 16, key);
    pr_info("aes_gcm: IV:  %*phN\n", 12, iv);
    pr_info("aes_gcm: Plaintext: %*phN\n", 16, pt);

    aes_gcm_encrypt(pt, key, iv, ct, tag);
    pr_info("aes_gcm: Ciphertext: %*phN\n", 16, ct);
    pr_info("aes_gcm: Tag:        %*phN\n", 16, tag);
    return 0;
}

static void __exit aes_gcm_exit(void)
{
    pr_info("aes_gcm: Module exit\n");
}

module_init(aes_gcm_init);
module_exit(aes_gcm_exit);

MODULE_AUTHOR("StackExchange User");
MODULE_DESCRIPTION("AES-GCM-128 with AES-NI/PCLMUL in assembly");
MODULE_LICENSE("GPL");

