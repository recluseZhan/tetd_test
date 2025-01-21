#include <linux/module.h>
#include <linux/scatterlist.h>
#include <crypto/skcipher.h>

#define AES_KEY_SIZE 16
#define AES_BLOCK_SIZE 16
static const unsigned char aes_key[AES_KEY_SIZE] = "0123456789abcdef";
#define DUMP_SIZE 4096

static unsigned long aes_encrypt(const unsigned char *input, unsigned char *output){
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
    return 0;
}
unsigned long work_encrypt(const unsigned char *input, unsigned char *output){
    for(int i = 0; i < DUMP_SIZE / AES_BLOCK_SIZE; i++){
        aes_encrypt(input + i * AES_BLOCK_SIZE, output + i * AES_BLOCK_SIZE);
    }
    return 0;
}

#define IVSHMEM_BAR0_ADDRESS 0x383800000000  // BAR 2 addr
#define IVSHMEM_BAR0_SIZE (1 * 1024 * 1024)  // BAR 2 size
void __iomem *ivshmem_base;

#define ORDER 6
#define RING_BUFFER_SIZE (256 * 1024)  // 256KB
#define PAGE_SIZE 4096

char* shared_mem;
unsigned long head = 0;
unsigned long tail = 0;

void write_to_buffer(unsigned long len) {
    unsigned long bytes_written = 0;
    unsigned long phys_addr = 0x00900000;
    void *data = phys_to_virt(phys_addr);
    while (bytes_written < len) {
        while (((head + 1) % RING_BUFFER_SIZE) == tail) {
            cpu_relax();  
        }	
	//memset(ivshmem_base,1,RING_BUFFER_SIZE);
	//work_encrypt(data+bytes_written,ivshmem_base+head);
        memcpy(ivshmem_base+head,data+bytes_written,PAGE_SIZE);
	head = (head+PAGE_SIZE) % RING_BUFFER_SIZE;
	bytes_written += PAGE_SIZE;
    }
}

#define DATA_SIZE (4096*3)
void write_kernel_to_buffer(void){
    write_to_buffer(DATA_SIZE);
}
static int __init write_module_init(void)
{
    
    shared_mem = __get_free_pages(GFP_KERNEL,ORDER);
    if (!shared_mem) {
        pr_err("Failed to allocate shared memory\n");
        return -ENOMEM;
    }
    ivshmem_base = ioremap(IVSHMEM_BAR0_ADDRESS, IVSHMEM_BAR0_SIZE);
    if (!ivshmem_base) {
        pr_err("Could not map ivshmem memory\n");
        return -EIO;
    }
    pr_info("Writer module loaded\n");
    return 0;
}

static void __exit write_module_exit(void)
{
    free_pages(shared_mem,ORDER);
    if (ivshmem_base) {
        iounmap(ivshmem_base);
        pr_info("ivshmem memory unmapped\n");
    }
    pr_info("Writer module unloaded\n");
}
EXPORT_SYMBOL(shared_mem);
EXPORT_SYMBOL(head);
EXPORT_SYMBOL(tail);
EXPORT_SYMBOL(write_to_buffer);
EXPORT_SYMBOL(write_kernel_to_buffer);

module_init(write_module_init);
module_exit(write_module_exit);
MODULE_LICENSE("GPL");
