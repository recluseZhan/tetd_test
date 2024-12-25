#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}
//#define SHA_DATA_SIZE 4096
#define SHA_DATA_SIZE (1ULL<<20)
extern void sha256_ni_transform(uint32_t *digest, void *data, uint32_t numBlocks);
static int __init my_module_init(void) {
    uint32_t digest[8] = {0};  // Placeholder for the digest
    uint8_t *data;
    data = kmalloc(SHA_DATA_SIZE,GFP_KERNEL);
    get_random_bytes(data,SHA_DATA_SIZE);
    uint32_t numBlocks = SHA_DATA_SIZE / 64;     // Assuming only one block
    unsigned long t1,t2;
    // Call the assembly function
    t1 = urdtsc();
    sha256_ni_transform(digest, data, numBlocks);
    t2 = urdtsc();
    printk("%ld\n",(t2-t1)*5/17);
    // Print the resulting digest
    printk(KERN_INFO "SHA256 Digest: %08x %08x %08x %08x %08x %08x %08x %08x\n",
           digest[0], digest[1], digest[2], digest[3],
           digest[4], digest[5], digest[6], digest[7]);
    return 0;
}

static void __exit my_module_exit(void) {
    printk(KERN_INFO "Exiting my_module\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

