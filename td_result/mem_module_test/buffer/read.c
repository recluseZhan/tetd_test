#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#define RING_BUFFER_SIZE (256 * 1024)  // 256KB
//#define DATA_SIZE (1*1024*1024)
#define DATA_SIZE 4096
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
            cpu_relax();  // 
        }
        memcpy(print_buffer,shared_mem+tail,PAGE_SIZE);
	
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
    if (!shared_mem) {
        pr_err("memory fail\n");
        return -ENOMEM;
    }
    pr_info("insmod read module\n");
    return 0;
}

static void __exit read_module_exit(void)
{
    pr_info("read module exit\n");
}

EXPORT_SYMBOL(read_from_buffer);

module_init(read_module_init);
module_exit(read_module_exit);
MODULE_LICENSE("GPL");

