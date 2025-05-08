#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>

extern void aes_gcm_encrypt(const u8 *pt, const u8 key[16],
                            const u8 iv[12], size_t len,
                            u8 *ct, u8 tag[16]);


static int __init aes_gcm_mod_init(void)
{
    u8 sample_key[16] = {
    0x00,0x11,0x22,0x33, 0x44,0x55,0x66,0x77,
    0x88,0x99,0xaa,0xbb, 0xcc,0xdd,0xee,0xff
    };

    u8 sample_iv[12] = {
    0x10,0x11,0x12,0x13,
    0x14,0x15,0x16,0x17,
    0x18,0x19,0x1a,0x1b
    };

    char sample_plain[] = "Hello, AES-GCM World!"; // 20 bytes
    size_t len = sizeof(sample_plain) - 1;
    u8 *cipher;
    u8 tag[16];
    
    cipher = kmalloc(len + 16, GFP_KERNEL);
    if (!cipher) {
        pr_err("aes_gcm: alloc fail\n");
        return -ENOMEM;
    }

    pr_info("aes_gcm: plaintext='%s'\n", sample_plain);

    aes_gcm_encrypt((const u8 *)sample_plain,sample_key,sample_iv,len,cipher,tag);

    pr_info("aes_gcm: ciphertext:");
    print_hex_dump(KERN_INFO, "    ", DUMP_PREFIX_OFFSET, 16, 1,
                   cipher, len, false);

    pr_info("aes_gcm: tag:");
    print_hex_dump(KERN_INFO, "    ", DUMP_PREFIX_OFFSET, 16, 1,
                   tag, 16, false);

    kfree(cipher);
    return 0;
}

static void __exit aes_gcm_mod_exit(void)
{
    pr_info("aes_gcm: module exit\n");
}

module_init(aes_gcm_mod_init);
module_exit(aes_gcm_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Standalone AES-GCM-128 using AES-NI in .S");

