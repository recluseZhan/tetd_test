#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/highmem.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/resource.h>

#define IVSHMEM_BAR0_ADDRESS 0x383800000000  // ivshmem BAR 2 地址
#define IVSHMEM_BAR0_SIZE (1 * 1024 * 1024)   // ivshmem 大小，1MB

#define START_PHYS_ADDR 0x00900000UL
#define END_PHYS_ADDR   0x7ee14fffUL

static void *ivshmem;
static unsigned long ivshmem_offset = 0;

unsigned long urdtsc(void)
{
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long)hi << 32) | lo;
}

static int __init memory_copy_init(void) {
    phys_addr_t addr;
    void *vaddr;
    size_t page_size = PAGE_SIZE;
    unsigned long t1, t2, t_all = 0;
    unsigned long total_bytes = 0;

    ivshmem = ioremap(IVSHMEM_BAR0_ADDRESS, IVSHMEM_BAR0_SIZE);
    if (!ivshmem) {
        pr_err("Failed to remap ivshmem memory\n");
        return -ENOMEM;
    }

    pr_info("Starting to copy System RAM region from 0x%lx to 0x%lx\n",
            (unsigned long)START_PHYS_ADDR, (unsigned long)END_PHYS_ADDR);

    for (addr = START_PHYS_ADDR; addr <= END_PHYS_ADDR; addr += page_size) {
        if (!pfn_valid(addr >> PAGE_SHIFT))
            continue;

        vaddr = memremap(addr, page_size, MEMREMAP_WB);
        if (!vaddr) {
            pr_warn("Failed to remap address: 0x%lx\n", (unsigned long)addr);
            continue;
        }

        t1 = urdtsc();
        memcpy(ivshmem + (ivshmem_offset % IVSHMEM_BAR0_SIZE), vaddr, page_size);
        t2 = urdtsc();
        t_all += (t2 - t1) * 5 / 12;

        total_bytes += page_size;
        ivshmem_offset += page_size;

        if (ivshmem_offset >= IVSHMEM_BAR0_SIZE)
            ivshmem_offset = 0;

        // 可选：打印每页信息
        pr_info("Copied page at phys=0x%lx to ivshmem_offset=0x%lx,*vaddr=0x%u\n", (unsigned long)addr, ivshmem_offset,*(uint8_t*)vaddr);

        memunmap(vaddr);
    }

    pr_info("Total copied: %lu bytes (%lu KB, %lu MB)\n",
            total_bytes, total_bytes / 1024, total_bytes / (1024 * 1024));
    pr_info("Copying finished. Total time: %lu ns\n", t_all);
    return 0;
}

static void __exit memory_copy_exit(void) {
    if (ivshmem) {
        iounmap(ivshmem);
        pr_info("ivshmem unmapped, module unloaded.\n");
    }
}

module_init(memory_copy_init);
module_exit(memory_copy_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Kernel Module to copy specified physical memory to ivshmem");

