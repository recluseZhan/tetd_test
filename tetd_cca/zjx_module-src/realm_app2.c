#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <asm/io.h>

extern void read_page(void);

static int __init read_page_init(void)
{
    printk(KERN_INFO "Module2 exiting\n");
    read_page();
    return 0;
}

static void __exit read_page_exit(void)
{
    printk(KERN_INFO "Module2 exiting\n");
}

module_init(read_page_init);
module_exit(read_page_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("read a 4KB page to all 1s");

