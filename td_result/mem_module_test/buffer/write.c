#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/delay.h>
#define IVSHMEM_BAR0_ADDRESS 0x383800000000  // BAR 2 地址
#define IVSHMEM_BAR0_SIZE (1 * 1024 * 1024)            // BAR 2 大小
void __iomem *ivshmem_base;  // 保存映射的基地址
unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}

#define ORDER 6
#define RING_BUFFER_SIZE (256 * 1024)     // 256KB
#define PAGE_SIZE 4096

char* shared_mem;
unsigned long head = 0;
unsigned long tail = 0;

void write_to_buffer(unsigned long len) {
    unsigned long bytes_written = 0;
    unsigned long phys_addr = 0x00900000;
    unsigned long data = virt_to_phys(phys_addr);
    unsigned long t1,t2,t_all=0;
    while (bytes_written < len) {
        //while (((head + 1) % RING_BUFFER_SIZE) == tail) {
        //    cpu_relax();  
        //}
	//
	t1=urdtsc();
        memcpy(ivshmem_base+head,data+bytes_written,PAGE_SIZE);
        //memcpy(shared_mem+head,data+bytes_written,PAGE_SIZE);
	t2=urdtsc();
	t_all=t_all+(t2-t1)*5/12;
	head = (head+PAGE_SIZE) % RING_BUFFER_SIZE;
	bytes_written += PAGE_SIZE;
    }
    printk("time:%ld",t_all);
}

#define DATA_SIZE (2*1024*1024)
//#define DATA_SIZE 4096
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

