#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/highmem.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <asm/io.h>

#define IVSHMEM_BAR0_ADDRESS 0x383800000000  // ivshmem BAR 2 地址
#define IVSHMEM_BAR0_SIZE (1 * 1024 * 1024)   // ivshmem 大小，1MB

static void *ivshmem;  // ivshmem 内存映射地址
static unsigned long ivshmem_offset = 0;  // ivshmem 数据的写入偏移量

unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}

// 使用内核模块加载时的初始化函数
static int __init memory_copy_init(void) {
    unsigned long pfn;
    unsigned long max_pfn;
    void *vaddr;
    phys_addr_t phys;
    size_t page_size = PAGE_SIZE;

    unsigned long t1,t2,t_all=0;
    // 映射 ivshmem 的地址
    ivshmem = ioremap(IVSHMEM_BAR0_ADDRESS, IVSHMEM_BAR0_SIZE);
    if (!ivshmem) {
        pr_err("Failed to remap ivshmem memory\n");
        return -ENOMEM;
    }

    // 获取系统最大页帧号
    max_pfn = get_num_physpages();

    pr_info("Start copying 2GB physical memory to ivshmem...\n");

    // 遍历所有有效的物理内存页
    for (pfn = 0; pfn < max_pfn; pfn++) {
        if (!pfn_valid(pfn)) {
            continue;
        }

        // 获取该页帧的物理地址
        phys = (phys_addr_t)pfn << PAGE_SHIFT;

        // 使用 memremap 映射物理地址为内核虚拟地址
        vaddr = memremap(phys, page_size, MEMREMAP_WB);
        if (!vaddr) {
            pr_err("Failed to remap physical page at PFN %lu\n", pfn);
            continue;
        }

        // 将内存内容复制到 ivshmem
        t1=urdtsc();
        memcpy(ivshmem + (ivshmem_offset % IVSHMEM_BAR0_SIZE), vaddr, page_size);
        t2=urdtsc();
        t_all=t_all+(t2-t1)*5/12;
        ivshmem_offset += page_size;

        // 如果 ivshmem 满了，回到起始地址重新开始
        if (ivshmem_offset >= IVSHMEM_BAR0_SIZE) {
            ivshmem_offset = 0;
        }

        // 解除映射
        memunmap(vaddr);

        // 为了避免过多的日志输出，适当调整日志的打印频率
        if (pfn % (max_pfn / 100) == 0) {
            pr_info("Copied %lu/%lu pages...\n", pfn, max_pfn);
        }
    }
    printk(KERN_INFO "Copying took %llu ns\n", t_all);
    pr_info("2GB physical memory copied to ivshmem completed.\n");
    return 0;
}

// 卸载内核模块时的清理函数
static void __exit memory_copy_exit(void) {
    if (ivshmem) {
        iounmap(ivshmem);
        pr_info("ivshmem memory unmapped\n");
    }
}

module_init(memory_copy_init);
module_exit(memory_copy_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Kernel Module to copy 2GB physical memory to ivshmem");

