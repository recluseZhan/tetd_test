#include <linux/module.h>
#include <linux/io.h>

#define IVSHMEM_BAR0_ADDRESS 0x383800000000  // BAR 2 addr
#define IVSHMEM_BAR0_SIZE (1 * 1024 * 1024)  // BAR 2 size
void __iomem *ivshmem_base;

#define RING_BUFFER_SIZE (256 * 1024)  // 256KB
#define DATA_SIZE (4096*3)
#define PAGE_SIZE 4096
extern char *shared_mem;
extern unsigned long head;
extern unsigned long tail;

int read_from_buffer(void)
{
    char print_buffer[PAGE_SIZE];
    unsigned long bytes_readed  = 0;
    while (bytes_readed < DATA_SIZE) {
        while (tail == head) {
            cpu_relax(); 
        }
        memcpy(print_buffer,ivshmem_base+tail,PAGE_SIZE);
	
        tail = (tail+PAGE_SIZE)%RING_BUFFER_SIZE;
	bytes_readed += PAGE_SIZE;
	
	for(int i=0;i<PAGE_SIZE;i++){
	    printk(KERN_CONT "%u",print_buffer[i]);
        }
    }
    return bytes_readed;
}

static int __init read_module_init(void)
{
    ivshmem_base = ioremap(IVSHMEM_BAR0_ADDRESS, IVSHMEM_BAR0_SIZE);
    if (!ivshmem_base) {
        pr_err("Could not map ivshmem memory\n");
        return -EIO;
    }
    pr_info("insmod read module\n");
    return 0;
}

static void __exit read_module_exit(void)
{
    pr_info("read module exit\n");
    if (ivshmem_base) {
        iounmap(ivshmem_base);
        pr_info("ivshmem memory unmapped\n");
    }
}

EXPORT_SYMBOL(read_from_buffer);

module_init(read_module_init);
module_exit(read_module_exit);
MODULE_LICENSE("GPL");
