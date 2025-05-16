#include <linux/module.h>
#include <linux/init.h>

/*—— 仅包含 <linux/module.h> —— 完整支持 x86_64 五级页表 ——*/

/*—— 常量 ——*/
#define PAGE_SHIFT    12UL
#define PAGE_SIZE     (1UL << PAGE_SHIFT)
#define PAGE_MASK     (~(PAGE_SIZE - 1))
#define PTRS_PER_LVL  512UL

/*—— 五级页表各级偏移 ——*/
#define PTE_SHIFT     12UL
#define PMD_SHIFT     21UL
#define PUD_SHIFT     30UL
#define P4D_SHIFT     39UL
#define PGD_SHIFT     48UL

/*—— 待映射的物理区间 ——*/
#define PHYS_START    0x00900000UL
#define PHYS_END      0x00901000UL

/*—— 要映射到的虚拟区间起点 ——*/
#define VSTART_ADDR   0xfffff00000000000UL

extern unsigned long __get_free_pages(unsigned int gfp_mask, unsigned int order);
/* 动态 PAGE_OFFSET 导出符号 */
extern unsigned long page_offset_base;

/*—— 从 CR3 取顶级 PGD ——*/
static inline unsigned long read_cr3_local(void)
{
    unsigned long val;
    asm volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}
static inline void write_cr3_local(unsigned long val)
{
    asm volatile("mov %0, %%cr3" :: "r"(val));
}

/*—— 物理→直映虚拟 ——*/
static inline void *phys_to_virt_k(unsigned long phys)
{
    return (void *)(phys + page_offset_base);
}

static int __init manual_remap5_init(void)
{
    unsigned long phys = PHYS_START & PAGE_MASK;
    unsigned long virt = VSTART_ADDR;
    unsigned long ent;
    int i0,i1,i2,i3,i4;
    unsigned long cr3;
    unsigned long *pgd;

    cr3 = read_cr3_local();
    pgd = phys_to_virt_k(cr3 & PAGE_MASK);

    pr_info("manual_remap5: phys [0x%lx–0x%lx] → virt start 0x%lx, PAGE_OFFSET=0x%lx\n",
            PHYS_START, PHYS_END, VSTART_ADDR, page_offset_base);

    for (; phys <= PHYS_END; phys += PAGE_SIZE, virt += PAGE_SIZE) {
        i0 = (virt >> PGD_SHIFT) & (PTRS_PER_LVL - 1);
        i1 = (virt >> P4D_SHIFT) & (PTRS_PER_LVL - 1);
        i2 = (virt >> PUD_SHIFT) & (PTRS_PER_LVL - 1);
        i3 = (virt >> PMD_SHIFT) & (PTRS_PER_LVL - 1);
        i4 = (virt >> PTE_SHIFT) & (PTRS_PER_LVL - 1);

        /* PML5/PGD */
        ent = pgd[i0];
        if (!(ent & 1UL)) {
            pr_err("PGD missing idx %d\n", i0);
            return -ENOMEM;
        }

        /* P4D */
        {
            unsigned long *p4d = phys_to_virt_k(ent & PAGE_MASK);
            ent = p4d[i1];
            if (!(ent & 1UL)) {
                unsigned long np = __get_free_pages(GFP_KERNEL, 0);
                if (!np) return -ENOMEM;
                p4d[i1] = (np & PAGE_MASK) | 0x3UL;
                ent = p4d[i1];
            }
        }
        /* PUD */
        {
            unsigned long *pud = phys_to_virt_k(ent & PAGE_MASK);
            ent = pud[i2];
            if (!(ent & 1UL)) {
                unsigned long np = __get_free_pages(GFP_KERNEL, 0);
                if (!np) return -ENOMEM;
                pud[i2] = (np & PAGE_MASK) | 0x3UL;
                ent = pud[i2];
            }
        }
        /* PMD */
        {
            unsigned long *pmd = phys_to_virt_k(ent & PAGE_MASK);
            ent = pmd[i3];
            if (!(ent & 1UL)) {
                unsigned long np = __get_free_pages(GFP_KERNEL, 0);
                if (!np) return -ENOMEM;
                pmd[i3] = (np & PAGE_MASK) | 0x3UL;
                ent = pmd[i3];
            }
        }
        /* PTE */
        {
            unsigned long *pte = phys_to_virt_k(ent & PAGE_MASK);
            pte[i4] = ((phys >> PAGE_SHIFT) << PTE_SHIFT) | 0x3UL;
        }
    }

    /* 刷新 TLB */
    write_cr3_local(read_cr3_local());
    pr_info("manual_remap5: mapping done\n");
    return 0;
}

static void __exit manual_remap5_exit(void)
{
    unsigned long phys = PHYS_START & PAGE_MASK;
    unsigned long virt = VSTART_ADDR;
    unsigned long ent;
    int i0,i1,i2,i3,i4;
    unsigned long cr3 = read_cr3_local();
    unsigned long *pgd = phys_to_virt_k(cr3 & PAGE_MASK);

    for (; phys <= PHYS_END; phys += PAGE_SIZE, virt += PAGE_SIZE) {
        i0 = (virt >> PGD_SHIFT) & (PTRS_PER_LVL - 1);
        i1 = (virt >> P4D_SHIFT) & (PTRS_PER_LVL - 1);
        i2 = (virt >> PUD_SHIFT) & (PTRS_PER_LVL - 1);
        i3 = (virt >> PMD_SHIFT) & (PTRS_PER_LVL - 1);
        i4 = (virt >> PTE_SHIFT) & (PTRS_PER_LVL - 1);

        ent = pgd[i0];
        {
            unsigned long *p4d = phys_to_virt_k(ent & PAGE_MASK);
            ent = p4d[i1];
            unsigned long *pud = phys_to_virt_k(ent & PAGE_MASK);
            ent = pud[i2];
            unsigned long *pmd = phys_to_virt_k(ent & PAGE_MASK);
            ent = pmd[i3];
            unsigned long *pte = phys_to_virt_k(ent & PAGE_MASK);
            pte[i4] = 0;
        }
    }

    write_cr3_local(read_cr3_local());
    pr_info("manual_remap5: unmapped\n");
}

module_init(manual_remap5_init);
module_exit(manual_remap5_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("x86_64 五级页表手动全区映射示例");

