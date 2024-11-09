#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/io.h>
MODULE_LICENSE("GPL");

#define ORDER 10
#define DATA_SIZE (4*1024*1024) // 

unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}

static unsigned long target_addr;
int page_copy(void){
    // Allocate 4MB of memory
    target_addr = __get_free_pages(GFP_KERNEL, ORDER);
    if (!target_addr) {
        pr_err("Failed to allocate memory\n");
        return -ENOMEM;
    }
    memset(target_addr,1,PAGE_SIZE);
    // Copy the first 4MB of kernel memory to the allocated page
    unsigned long start,end;
    start = urdtsc();
    memcpy((void *)target_addr, (void *)PAGE_OFFSET, PAGE_SIZE);
    end = urdtsc();
    printk(KERN_INFO "Copying took %llu ns\n", (end - start)*5/12);

    pr_info("4MB of data copied from kernel address 0x%lx to allocated memory 0x%lx\n", PAGE_OFFSET, target_addr);

    return 0;
}
/*
int page_copy(void){
    void *src, *dst;
    unsigned long long start, end;
    size_t i;
     
    // Allocate two 4MB (4KB aligned) memory blocks
    src = (void *)__get_free_pages(GFP_KERNEL, get_order(ORDER));
    dst = (void *)__get_free_pages(GFP_KERNEL, get_order(ORDER));
    if (!src || !dst) {
        printk(KERN_ERR "Memory allocation failed\n");
        if (src)
            free_pages((unsigned long)src, get_order(ORDER));
        if (dst)
            free_pages((unsigned long)dst, get_order(ORDER));
        return -ENOMEM;
    }

    // Fill src with data for copying
    memset(src, 0xAB, DATA_SIZE);
    memset(dst,0x11,DATA_SIZE);
    // Measure copy time using rdtsc
    start = urdtsc();
    memcpy(dst, src, DATA_SIZE);
    end = urdtsc();

    printk(KERN_INFO "Copying took %llu ns\n", (end - start)*5/12);

    // Free allocated memory
    free_pages((unsigned long)src, get_order(ORDER));
    free_pages((unsigned long)dst, get_order(ORDER));

    return 0;
}
*/
static int __init copy_time_init(void) {
    return 0;
}

static void __exit copy_time_exit(void) {
    printk(KERN_INFO "Exiting copy time measurement module\n");
}
EXPORT_SYMBOL(page_copy);
module_init(copy_time_init);
module_exit(copy_time_exit);

