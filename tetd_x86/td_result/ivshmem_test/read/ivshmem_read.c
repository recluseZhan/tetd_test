#include<linux/init.h>
#include<linux/module.h>
#include<linux/string.h>
#include<linux/kernel.h>
#include<linux/export.h>
#include<linux/scatterlist.h>
#include<linux/crypto.h>
#include <crypto/sha256_base.h>
#include <linux/err.h>
#include<crypto/skcipher.h>
#include<asm/desc.h>
#include<linux/interrupt.h>
#include<asm/irq_vectors.h>
#include<asm/io.h>

#include <linux/random.h>   
#include <linux/gfp.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <crypto/akcipher.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/highmem.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#define IVSHMEM_BAR0_ADDRESS 0x383800000000  // BAR 2 地址
#define IVSHMEM_BAR0_SIZE (1 * 1024 * 1024)            // BAR 2 大小
void __iomem *ivshmem_base;  // 保存映射的基地址
			     //
#define AES_KEY_SIZE 16
#define AES_BLOCK_SIZE 16
static const unsigned char aes_key[AES_KEY_SIZE] = "0123456789abcdef";
#define DUMP_SIZE 4096

static int aes_encrypt(const unsigned char *input, unsigned char *output){
    struct crypto_skcipher *tfm;
    struct skcipher_request *req;
    struct scatterlist sg_src, sg_dst;
    int ret;
    // allocate context
    tfm = crypto_alloc_skcipher("ecb-aes-aesni", 0, 0);
    if(IS_ERR(tfm)){
        printk(KERN_ERR"Error allocating cipher\n");
        return PTR_ERR(tfm);
    }
    // init req
    req = skcipher_request_alloc(tfm, GFP_KERNEL);
    if(!req){
        printk(KERN_ERR"Error allocating request\n");
        crypto_free_skcipher(tfm);
        return -ENOMEM;
    }
    // set key
    ret = crypto_skcipher_setkey(tfm, aes_key, AES_KEY_SIZE);
    if(ret){
        printk(KERN_ERR"Error setting key\n");
        skcipher_request_free(req);
        crypto_free_skcipher(tfm);
        return ret;
    }
    // prepare input and output scatterlist
    sg_init_one(&sg_src, input, AES_BLOCK_SIZE);
    sg_init_one(&sg_dst, output, AES_BLOCK_SIZE);
    // init req
    skcipher_request_set_crypt(req, &sg_src, &sg_dst, AES_BLOCK_SIZE, NULL);
    // encrypt
    ret = crypto_skcipher_encrypt(req);
    if(ret){
        printk(KERN_ERR"Encryption failed\n");
        skcipher_request_free(req);
        crypto_free_skcipher(tfm);
        return ret;
    }
    // free
    skcipher_request_free(req);
    crypto_free_skcipher(tfm);
    return ret;
}
static int aes_decrypt(const unsigned char *input, unsigned char *output){
    struct crypto_skcipher *tfm;
    struct skcipher_request *req;
    struct scatterlist sg_src, sg_dst;
    int ret;
    // allocate context
    tfm = crypto_alloc_skcipher("ecb-aes-aesni", 0, 0);
    if(IS_ERR(tfm)){
        printk(KERN_ERR"Error allocating cipher\n");
        return PTR_ERR(tfm);
    }
    // init req
    req = skcipher_request_alloc(tfm, GFP_KERNEL);
    if(!req){
        printk(KERN_ERR"Error allocating request\n");
        crypto_free_skcipher(tfm);
        return -ENOMEM;
    }
    // set key
    ret = crypto_skcipher_setkey(tfm, aes_key, AES_KEY_SIZE);
    if(ret){
        printk(KERN_ERR"Error setting key\n");
        skcipher_request_free(req);
        crypto_free_skcipher(tfm);
        return ret;
    }
    // prepare input and output scatterlist
    sg_init_one(&sg_src, input, AES_BLOCK_SIZE);
    sg_init_one(&sg_dst, output, AES_BLOCK_SIZE);
    // init req
    skcipher_request_set_crypt(req, &sg_src, &sg_dst, AES_BLOCK_SIZE, NULL);
    // decrypt
    ret = crypto_skcipher_decrypt(req);
    if(ret){
        printk(KERN_ERR"Decryption failed\n");
        skcipher_request_free(req);
        crypto_free_skcipher(tfm);
        return ret;
    }
    // free
    skcipher_request_free(req);
    crypto_free_skcipher(tfm);
    return ret;
}
void work_encrypt(const unsigned char *input, unsigned char *output){
    for(int i = 0; i < DUMP_SIZE / AES_BLOCK_SIZE; i++){
        aes_encrypt(input + i * AES_BLOCK_SIZE, output + i * AES_BLOCK_SIZE);
    }
}
void work_decrypt(const unsigned char *input, unsigned char *output){
    for(int i = 0; i < DUMP_SIZE / AES_BLOCK_SIZE; i++){
        aes_decrypt(input + i * AES_BLOCK_SIZE, output + i * AES_BLOCK_SIZE);
    }
}

void aes(void){

    unsigned char* data = kmalloc(DUMP_SIZE, GFP_KERNEL);
    if (!data) {
        pr_err("Failed to allocate memory\n");
        
    }

    // 生成4096字节的随机数据
    get_random_bytes(data, DUMP_SIZE);


    unsigned char data_crypto[DUMP_SIZE];
    unsigned char data_page[DUMP_SIZE]="hello_world,hhhhhh,dddddd,tttttt";
    work_encrypt(data,ivshmem_base);
    //for(int i=0;i<4096;i++){
    //    writel(*(data_page+i), ivshmem_base+i);
    //
    //}
    //memset(data_page,3,4096);
    //memcpy(ivshmem_base,data_page,4096);
    //unsigned long v;
     for(int i=0;i< 512;i++){
        v = readq(ivshmem_base+i*8);
	pr_info(KERN_CONT "0x%lx ", v);
    }
    //memcpy(ivshmem_base,data_page,4096);
    //printk("en:");
    //for(int i = 0; i < DUMP_SIZE; i++)
    //printk(KERN_CONT"%02x ",*(data_crypto+i));
}

static int copy_aes(void){

    // 获取 IDT 的基地址
    //memcpy(ivshmem_base, (void *)idt_base, idt_size);

    return 0;
}
// 初始化函数：映射共享内存
static int __init ivshmem_init(void) {
    uint32_t read_value;

    pr_info("Initializing ivshmem module...\n");

    // 映射 ivshmem 的 BAR 0 内存区域
    ivshmem_base = ioremap(IVSHMEM_BAR0_ADDRESS, IVSHMEM_BAR0_SIZE);
    if (!ivshmem_base) {
        pr_err("Could not map ivshmem memory\n");
        return -EIO;
    }

    pr_info("ivshmem memory mapped at address: %p\n", ivshmem_base);

    // 测试读取数据
    aes();
    //read_value = read_ivshmem(0);  // 读取偏移 0 的数据
    //pr_info("Read value 0x%x from offset 0x0\n", read_value);

    return 0;
}

// 退出函数：解除映射
static void __exit ivshmem_exit(void) {
    if (ivshmem_base) {
        iounmap(ivshmem_base);
        pr_info("ivshmem memory unmapped\n");
    }

    pr_info("Exiting ivshmem module\n");
}
module_init(ivshmem_init);
module_exit(ivshmem_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("IVSHMEM Kernel Module for Reading and Writing to Shared Memory");

