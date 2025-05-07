#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

extern void aes_gcm_encrypt(u8 *dst, const u8 *src);

static const u8 plaintext[16] = {
    0x00,0x11,0x22,0x33, 0x44,0x55,0x66,0x77,
    0x88,0x99,0xaa,0xbb, 0xcc,0xdd,0xee,0xff
};

static int __init aes_gcm_init(void)
{
    u8 out[32];
    int i;

    /* 调用汇编函数进行加密 */
    aes_gcm_encrypt(out, plaintext);

    /* 输出密文和 Tag（16 字节 + 16 字节） */
    printk(KERN_INFO "AES-GCM ciphertext+tag: ");
    for (i = 0; i < 32; i++)
        printk("%02x", out[i]);
    printk("\n");

    return 0;
}

static void __exit aes_gcm_exit(void)
{
    printk(KERN_INFO "aes_gcm module unloaded\n");
}

module_init(aes_gcm_init);
module_exit(aes_gcm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("AES-GCM-128 Encryption Example Module");

