#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/delay.h>

#define ORDER 6
#define RING_BUFFER_SIZE (256 * 1024)     // 256KB
#define PAGE_SIZE 4096

char* shared_mem;
unsigned long head = 0;
unsigned long tail = 0;

void write_to_buffer(char* data, unsigned long len) {
    unsigned long bytes_written = 0;

    while (bytes_written < len) {
        //while (((head + 1) % RING_BUFFER_SIZE) == tail) {
        //    cpu_relax();  
        //}
	//
        memcpy(shared_mem+head,data+bytes_written,PAGE_SIZE);
	head = (head+PAGE_SIZE) % RING_BUFFER_SIZE;
	bytes_written += PAGE_SIZE;
    }
}

#define DATA_SIZE (1*1024*1024)
void write_kernel_to_buffer(void){
    char data[DATA_SIZE];
    memset(data,1,DATA_SIZE);
    write_to_buffer(data,DATA_SIZE);
}
static int __init write_module_init(void)
{
    
    shared_mem = __get_free_pages(GFP_KERNEL,ORDER);
    if (!shared_mem) {
        pr_err("Failed to allocate shared memory\n");
        return -ENOMEM;
    }
    pr_info("Writer module loaded\n");
    return 0;
}

static void __exit write_module_exit(void)
{
    free_pages(shared_mem,ORDER);
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

