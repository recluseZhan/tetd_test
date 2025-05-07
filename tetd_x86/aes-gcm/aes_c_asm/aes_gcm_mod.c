// aes_gcm_mod.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/byteorder/generic.h>  // for cpu_to_be32, cpu_to_be64

// 汇编实现的单块 AES-ECB 加密：
//   void aes_encrypt_128(const u8 *in, u8 *out, const u8 *key);
// 参数寄存器约定：RDI=in, RSI=out, RDX=key
extern void aes_encrypt_128(const u8 *in, u8 *out, const u8 *key);

#define GHASH_POLY 0xe1u

// 在 GF(2^128) 上按 NIST 标准“无进位乘法 + 规约”
// Xi[16], H[16] -> Z[16]
static void ghash_mul_block(const u8 Xi[16], const u8 H[16], u8 Z[16])
{
    u8 V[16], Zt[16] = {0};
    int i, bit;

    // V = H
    memcpy(V, H, 16);

    // 按位处理 X
    for (i = 0; i < 16; i++) {
        for (bit = 7; bit >= 0; bit--) {
            if ((Xi[i] >> bit) & 1) {
                // Zt ^= V
                int j;
                for (j = 0; j < 16; j++)
                    Zt[j] ^= V[j];
            }
            // 下面计算 V = V >> 1 ；若最低位是 1，则 V ^= R
            {
                int lsb = V[15] & 1;
                // 整体右移 1 bit
                for (int k = 15; k > 0; k--)
                    V[k] = (V[k] >> 1) | ((V[k-1] & 1) << 7);
                V[0] >>= 1;
                if (lsb) {
                    // R = 0xe1 0000...00
                    V[0] ^= GHASH_POLY;
                }
            }
        }
    }
    memcpy(Z, Zt, 16);
}

// AES-GCM 加密主函数
// key: 16 字节，iv: 12 字节，pt: 明文，len: 明文长度（字节）
// ct: 密文缓冲（至少 len 字节），tag: 16 字节缓冲
static void gcm_encrypt(
    const u8 *key, const u8 *iv,
    const u8 *pt, size_t len,
    u8 *ct, u8 *tag)
{
    u8 H[16], J0[16], E0[16], Yi[16], buf[16];
    size_t i, full_blocks = len / 16;
    size_t rem = len % 16;

    // 1. H = AES_K(0^128)
    memset(buf, 0, 16);
    aes_encrypt_128(buf, H, key);

    // 2. 构造 J0 = IV || 0x00000001
    memcpy(J0, iv, 12);
    J0[12] = J0[13] = J0[14] = 0;
    J0[15] = 1;

    // 3. E0 = AES_K(J0) （用于最终 Tag 计算）
    aes_encrypt_128(J0, E0, key);

    // 4. GHASH 初始 Y0 = 0
    memset(Yi, 0, 16);

    // 5. CTR 分块加密 + GHASH 更新
    for (i = 0; i < full_blocks; i++) {
        u8 ctr_blk[16];
        // 计数器块 = J0 + (i+1)；只增最后 32 位
        memcpy(ctr_blk, J0, 16);
        {
            u32 ctr = cpu_to_be32((u32)(i + 1));
            memcpy(&ctr_blk[12], &ctr, 4);
        }
        // keystream = AES_K(ctr_blk)
        aes_encrypt_128(ctr_blk, buf, key);

        // XOR 明文 -> 密文，并 GHASH 更新
        for (int j = 0; j < 16; j++) {
            ct[i*16 + j] = pt[i*16 + j] ^ buf[j];
            buf[j] = ct[i*16 + j];
        }
        ghash_mul_block(buf, H, Yi);
    }

    // 6. 处理剩余 <16 字节
    if (rem) {
        u8 ctr_blk[16];
        memcpy(ctr_blk, J0, 16);
        {
            u32 ctr = cpu_to_be32((u32)(full_blocks + 1));
            memcpy(&ctr_blk[12], &ctr, 4);
        }
        aes_encrypt_128(ctr_blk, buf, key);
        // 只处理 rem 字节，其余填 0
        for (int j = 0; j < rem; j++) {
            ct[full_blocks*16 + j] = pt[full_blocks*16 + j] ^ buf[j];
            buf[j] = ct[full_blocks*16 + j];
        }
        // 剩余 buf[j]=0
        for (int j = rem; j < 16; j++)
            buf[j] = 0;
        ghash_mul_block(buf, H, Yi);
    }

    // 7. 最终长度块 L = [0^64 || bit_len(plaintext)^64]
    {
        u8 len_blk[16] = {0};
        u64 bits = (u64)len * 8;
        u64 be_bits = cpu_to_be64(bits);
        memcpy(&len_blk[8], &be_bits, 8);
        // YS = (Yi ⊕ L) * H
        for (int j = 0; j < 16; j++)
            buf[j] = Yi[j] ^ len_blk[j];
        ghash_mul_block(buf, H, Yi);
    }

    // 8. Tag = E0 ⊕ Yi
    for (int j = 0; j < 16; j++)
        tag[j] = E0[j] ^ Yi[j];
}

// --- 以下是模块装载 / 卸载，以及测试代码 ---
static int __init aes_gcm_mod_init(void)
{
    static const u8 test_key[16] = {
        0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
        0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff
    };
    static const u8 test_iv[12] = {
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1a,0x1b
    };
    const char *plain = "Hello, AES-GCM in kernel!";
    size_t len = strlen(plain);
    u8 *cipher, tag[16];

    pr_info("aes_gcm: encrypting %zu bytes\n", len);
    cipher = kmalloc(len, GFP_KERNEL);
    if (!cipher)
        return -ENOMEM;

    gcm_encrypt(test_key, test_iv,
                (u8 *)plain, len,
                cipher, tag);

    pr_info("aes_gcm: CT=");
    print_hex_dump(KERN_INFO, "   ", DUMP_PREFIX_OFFSET,
                   16, 1, cipher, len, false);
    pr_info("aes_gcm: TAG=%*phN\n", 16, tag);

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
MODULE_DESCRIPTION("AES-GCM-128 Kernel Module — pure C + your aes_encrypt_128.asm");

