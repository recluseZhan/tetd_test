#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <asm/io.h>

static struct page *page;
static void *vaddr;
static phys_addr_t paddr;

void read_page(void){
    char value;
    // Read the first byte of the page
    value = *((char *)vaddr);
    printk(KERN_INFO "Value: %u\n", value);
}
EXPORT_SYMBOL(read_page);

static int __init allocate_page_init(void)
{
    // Allocate a single page
    page = alloc_pages(GFP_KERNEL, 0); // Order 0 means one page
    if (!page) {
        printk(KERN_ERR "Failed to allocate page\n");
        return -ENOMEM;
    }

    // Map the page to kernel virtual address space
    vaddr = page_to_virt(page);

    // Fill the page with all 1s (0xFF)
    memset(vaddr, 0xFF, PAGE_SIZE);

    // Get the physical address
    paddr = page_to_phys(page);

    printk(KERN_INFO "Page allocated\n");
    printk(KERN_INFO "Virtual address: %lx\n", vaddr);
    printk(KERN_INFO "Physical address: %lx\n", paddr);

    return 0;
}

static void __exit allocate_page_exit(void)
{
    printk(KERN_INFO "Module exiting\n");
}

module_init(allocate_page_init);
module_exit(allocate_page_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("Allocate and initialize a 4KB page to all 1s");

