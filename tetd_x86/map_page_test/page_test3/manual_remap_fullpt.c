#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/gfp.h>

/*—— 支持 x86_64 五级页表 ——*/
#define PAGE_SHIFT    12UL
#define PAGE_SIZE     (1UL << PAGE_SHIFT)
#define PAGE_MASK     (~(PAGE_SIZE - 1))
#define PTRS_PER_LVL  512UL

/* 各级索引偏移 */
#define PTE_SHIFT     12UL
#define PMD_SHIFT     21UL
#define PUD_SHIFT     30UL
#define P4D_SHIFT     39UL
#define PGD_SHIFT     48UL

/* 要映射的物理区间 */
#define PHYS_START    0x00900000UL
#define PHYS_END      0x00901000UL

/* 映射到 vmalloc 区域起始 */
#define VSTART_ADDR   VMALLOC_START

/* 引用内核函数 */
extern unsigned long __get_free_pages(unsigned int gfp_mask, unsigned int order);

/* 读 CR3 */
static inline unsigned long read_cr3(void)
{
    unsigned long val;
    asm volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

/* 物理 -> 直映虚拟 */
static inline void *phys_to_virt_k(unsigned long phys)
{
    return __va(phys);
}

/**
 * manual_remap_range - 将物理地址区间映射到指定的虚拟区间
 * @phys_start: 起始物理地址（页对齐）
 * @phys_end: 结束物理地址（页对齐）
 * @virt_start: 起始虚拟地址（页对齐）
 *
 * 返回 0 表示成功，否则失败
 */
static int manual_remap_range(unsigned long phys_start, unsigned long phys_end, unsigned long virt_start)
{
    unsigned long phys = phys_start;
    unsigned long virt = virt_start & PAGE_MASK;
    unsigned long cr3 = read_cr3() & PAGE_MASK;
    unsigned long *pgd = phys_to_virt_k(cr3);
    unsigned long *p4d, *pud, *pmd, *pte;
    unsigned long ent;
    int i0,i1,i2,i3,i4;

    for (; phys <= phys_end; phys += PAGE_SIZE, virt += PAGE_SIZE) {
        i0 = (virt >> PGD_SHIFT) & (PTRS_PER_LVL - 1);
        i1 = (virt >> P4D_SHIFT) & (PTRS_PER_LVL - 1);
        i2 = (virt >> PUD_SHIFT) & (PTRS_PER_LVL - 1);
        i3 = (virt >> PMD_SHIFT) & (PTRS_PER_LVL - 1);
        i4 = (virt >> PTE_SHIFT) & (PTRS_PER_LVL - 1);

        /* PML5/PGD */
        if (!(pgd[i0] & 1UL)) {
            unsigned long np = __get_free_pages(GFP_KERNEL, 0);
            if (!np) return -ENOMEM;
            pgd[i0] = (np & PAGE_MASK) | 0x3UL;
        }
        p4d = phys_to_virt_k(pgd[i0] & PAGE_MASK);

        /* P4D */
        if (!(p4d[i1] & 1UL)) {
            unsigned long np = __get_free_pages(GFP_KERNEL, 0);
            if (!np) return -ENOMEM;
            p4d[i1] = (np & PAGE_MASK) | 0x3UL;
        }
        pud = phys_to_virt_k(p4d[i1] & PAGE_MASK);

        /* PUD */
        if (!(pud[i2] & 1UL)) {
            unsigned long np = __get_free_pages(GFP_KERNEL, 0);
            if (!np) return -ENOMEM;
            pud[i2] = (np & PAGE_MASK) | 0x3UL;
        }
        pmd = phys_to_virt_k(pud[i2] & PAGE_MASK);

        /* PMD */
        if (!(pmd[i3] & 1UL)) {
            unsigned long np = __get_free_pages(GFP_KERNEL, 0);
            if (!np) return -ENOMEM;
            pmd[i3] = (np & PAGE_MASK) | 0x3UL;
        }
        pte = phys_to_virt_k(pmd[i3] & PAGE_MASK);

        /* PTE */
        pte[i4] = (phys & PAGE_MASK) | 0x3UL;
    }
    return 0;
}

/**
 * manual_remap_init - 模块初始化，调用 remap 并验证
 */
static int __init manual_remap_init(void)
{
    int ret;
    unsigned long *test_ptr;
    int i;

    pr_info("manual_remap_fullpt: start mapping phys [0x%lx-0x%lx] to virt 0x%lx\n",
            PHYS_START, PHYS_END, VSTART_ADDR);

    ret = manual_remap_range(PHYS_START & PAGE_MASK,
                             PHYS_END & PAGE_MASK,
                             VSTART_ADDR);
    if (ret) {
        pr_err("manual_remap_fullpt: remap_range failed %d\n", ret);
        return ret;
    }
    pr_info("manual_remap_fullpt: mapping done\n");

    /* 验证：读取映射区前 16 字节 */
    test_ptr = (unsigned long *)VSTART_ADDR;
    pr_info("manual_remap_fullpt: first 16 bytes at virt:\n");
    for (i = 0; i < 4; i++) {
        pr_info("  [0x%lx] = 0x%016lx\n",
                (unsigned long)(VSTART_ADDR + i * sizeof(unsigned long)),
                test_ptr[i]);
    }

    return 0;
}

static void __exit manual_remap_exit(void)
{
    pr_info("manual_remap_fullpt: module unloaded\n");
}

module_init(manual_remap_init);
module_exit(manual_remap_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("抽象 remap 函数并增加验证打印示例");

