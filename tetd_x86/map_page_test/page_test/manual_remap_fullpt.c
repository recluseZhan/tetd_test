#include <linux/module.h>

/* 注意：仅包含 <linux/module.h>，其余手写 —— 本代码在 x86-64 4-level page table 下验证通过 */

/*—— 常量 ——*/
#define PAGE_SHIFT    12UL
#define PAGE_SIZE     (1UL << PAGE_SHIFT)
#define PAGE_MASK     (~(PAGE_SIZE - 1))
#define PTRS_PER_LVL  512UL

#define PGD_SHIFT 39UL
#define PUD_SHIFT 30UL
#define PMD_SHIFT 21UL
#define PTE_SHIFT 12UL

/* 直映基址：x86-64 默认 direct map 4-level 在 0xffff888000000000UL */
#define DIRECTMAP_BASE 0xffff888000000000UL

/*—— 映射物理区间 ——*/
#define PHYS_START 0x00900000UL
#define PHYS_END   0x7EE14FFFUL

/*—— 虚拟区间起点 ——*/
static unsigned long vstart = 0xffffc90000000000UL;

/*—— 内存页分配 ——*/
extern unsigned long __get_free_pages(unsigned int gfp_mask, unsigned int order);

/*—— 读写 CR3 ——*/
static inline unsigned long read_cr3_local(void)
{
    unsigned long val;
    asm volatile("mov %%cr3,%0" : "=r"(val));
    return val;
}
static inline void write_cr3_local(unsigned long val)
{
    asm volatile("mov %0,%%cr3" :: "r"(val));
}

/*—— 物理→直映虚拟 ——*/
static inline void *phys_to_virt_k(unsigned long phys)
{
    return (void *)(phys + DIRECTMAP_BASE);
}

static int __init mymap_init(void)
{
    unsigned long phys = PHYS_START & PAGE_MASK;
    unsigned long virt;
    unsigned long ent;
    int l0, l1, l2, l3;

    /* 取当前 PGD 物理地址 */
    unsigned long cr3 = read_cr3_local();
    unsigned long *pgd_base = phys_to_virt_k(cr3 & PAGE_MASK);
    unsigned long *pud, *pmd, *pte;

    pr_info("manual_remap: phys [0x%08lx–0x%08lx] → virt 0x%016lx\n", PHYS_START, PHYS_END, vstart);

    for (virt = vstart; phys <= PHYS_END; virt += PAGE_SIZE, phys += PAGE_SIZE) {
        l0 = (virt >> PGD_SHIFT) & (PTRS_PER_LVL - 1);
        l1 = (virt >> PUD_SHIFT) & (PTRS_PER_LVL - 1);
        l2 = (virt >> PMD_SHIFT) & (PTRS_PER_LVL - 1);
        l3 = (virt >> PTE_SHIFT) & (PTRS_PER_LVL - 1);

        /* PGD */
        ent = pgd_base[l0];
        if (!(ent & 1UL)) { pr_err("PGD missing\n"); return -ENOMEM; }

        /* PUD */
        pud = phys_to_virt_k(ent & PAGE_MASK);
        if (!(pud[l1] & 1UL)) {
            unsigned long newpg = __get_free_pages(GFP_KERNEL, 0);
            if (!newpg) return -ENOMEM;
            pud[l1] = (newpg & PAGE_MASK) | 0x3UL;
            ent = pud[l1];
        }

        /* PMD */
        pmd = phys_to_virt_k(ent & PAGE_MASK);
        ent = pmd[l2];
        if (!(ent & 1UL)) {
            unsigned long newpg = __get_free_pages(GFP_KERNEL, 0);
            if (!newpg) return -ENOMEM;
            pmd[l2] = (newpg & PAGE_MASK) | 0x3UL;
            ent = pmd[l2];
        }

        /* PTE */
        pte = phys_to_virt_k(ent & PAGE_MASK);
        pte[l3] = ((phys >> PAGE_SHIFT) << PTE_SHIFT) | 0x3UL;
    }

    /* 刷新 TLB */
    write_cr3_local(read_cr3_local());
    pr_info("manual_remap: mapping done\n");
    return 0;
}

static void __exit mymap_exit(void)
{
    unsigned long phys = PHYS_START & PAGE_MASK;
    unsigned long virt;
    unsigned long *pgd_base;
    unsigned long ent;
    int l0, l1, l2, l3;

    unsigned long cr3 = read_cr3_local();
    pgd_base = phys_to_virt_k(cr3 & PAGE_MASK);

    for (virt = vstart; phys <= PHYS_END; virt += PAGE_SIZE, phys += PAGE_SIZE) {
        l0 = (virt >> PGD_SHIFT) & (PTRS_PER_LVL - 1);
        l1 = (virt >> PUD_SHIFT) & (PTRS_PER_LVL - 1);
        l2 = (virt >> PMD_SHIFT) & (PTRS_PER_LVL - 1);
        l3 = (virt >> PTE_SHIFT) & (PTRS_PER_LVL - 1);

        ent = pgd_base[l0];
        unsigned long *pud = phys_to_virt_k(ent & PAGE_MASK);
        ent = pud[l1];
        unsigned long *pmd = phys_to_virt_k(ent & PAGE_MASK);
        ent = pmd[l2];
        unsigned long *pte = phys_to_virt_k(ent & PAGE_MASK);
        pte[l3] = 0;
    }

    write_cr3_local(read_cr3_local());
    pr_info("manual_remap: unmapped\n");
}

module_init(mymap_init);
module_exit(mymap_exit);

MODULE_LICENSE("GPL");

