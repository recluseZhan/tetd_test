#include <linux/module.h>
//#include <linux/vmalloc.h>
#include <asm/io.h>

#define PAGE_SHIFT    12UL
#define PAGE_SIZE     (1UL << PAGE_SHIFT)
#define PAGE_MASK     (~(PAGE_SIZE - 1))
#define PTRS_PER_LVL  512UL

#define PTE_SHIFT     12UL
#define PMD_SHIFT     21UL
#define PUD_SHIFT     30UL
#define P4D_SHIFT     39UL
#define PGD_SHIFT     48UL

#define PHYS_START    0x00900000UL
#define PHYS_END      0x00901000UL
#define PTE_FLAGS   (_PAGE_PRESENT | _PAGE_RW | _PAGE_PWT | _PAGE_PCD)  // 0x1A3

//#define PHYS_START 0x383800000000
//#define PHYS_END 0x383800001000

#define VSTART_ADDR 0xffa0000000000000UL
//#define VSTART_ADDR   VMALLOC_START
//#define __VMALLOC_BASE_L4     0xffffc90000000000UL
//#define __VMALLOC_BASE_L5     0xffa0000000000000UL

extern unsigned long __get_free_pages(unsigned int gfp_mask, unsigned int order);

static inline unsigned long read_cr3(void)
{
    unsigned long val;
    asm volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline void *phys_to_virt_k(unsigned long phys)
{
    return __va(phys);
}

static inline void manual_invlpg(unsigned long vaddr) {
    asm volatile("invlpg (%0)" ::"r" (vaddr) : "memory");
}

void manual_flush_tlb_kernel_range(unsigned long start, unsigned long end) {
    start = start & PAGE_MASK;
    end = ALIGN(end, PAGE_SIZE);
    for (; start < end; start += PAGE_SIZE) {
        manual_invlpg(start);
    }
}

static int manual_remap_range(unsigned long phys_start, unsigned long phys_end, unsigned long virt_start)
{
    unsigned long phys = phys_start & PAGE_MASK;
    unsigned long virt = virt_start & PAGE_MASK;
    unsigned long cr3 = read_cr3() & PAGE_MASK;
    unsigned long *pgd = phys_to_virt_k(cr3);
    unsigned long *p4d, *pud, *pmd, *pte;
    unsigned long ent;
    int i0,i1,i2,i3,i4;
    phys_end = phys_end & PAGE_MASK;

    for (; phys <= phys_end; phys += PAGE_SIZE, virt += PAGE_SIZE) {
        i0 = (virt >> PGD_SHIFT) & (PTRS_PER_LVL - 1);
        i1 = (virt >> P4D_SHIFT) & (PTRS_PER_LVL - 1);
        i2 = (virt >> PUD_SHIFT) & (PTRS_PER_LVL - 1);
        i3 = (virt >> PMD_SHIFT) & (PTRS_PER_LVL - 1);
        i4 = (virt >> PTE_SHIFT) & (PTRS_PER_LVL - 1);

        /* PML5/PGD */
        if (!(pgd[i0] & 1UL)) {
            unsigned long np = __get_free_pages(GFP_KERNEL | __GFP_ZERO, 0);
            if (!np) return -ENOMEM;
	    printk("will get free page\n");
            //pgd[i0] = (((unsigned long)np - PAGE_OFFSET) & PAGE_MASK) | 0x3UL;
	    np = virt_to_phys((void *)np);
	    pgd[i0] = (np & PAGE_MASK) | 0x3UL;
	    printk("get free page\n");
        }
        p4d = phys_to_virt_k(pgd[i0] & PAGE_MASK);

        /* P4D */
        if (!(p4d[i1] & 1UL)) {
            unsigned long np = __get_free_pages(GFP_KERNEL | __GFP_ZERO, 0);
            if (!np) return -ENOMEM;
            //p4d[i1] = (((unsigned long)np - PAGE_OFFSET) & PAGE_MASK) | 0x3UL;
	    np = virt_to_phys((void *)np);
	    p4d[i1] = (np & PAGE_MASK) | 0x3UL;
	    printk("get free page\n");
        }
        pud = phys_to_virt_k(p4d[i1] & PAGE_MASK);

        /* PUD */
        if (!(pud[i2] & 1UL)) {
            unsigned long np = __get_free_pages(GFP_KERNEL | __GFP_ZERO, 0);
            if (!np) return -ENOMEM;
            //pud[i2] = (((unsigned long)np - PAGE_OFFSET) & PAGE_MASK) | 0x3UL;
	    np = virt_to_phys((void *)np);
	    pud[i2] = (np & PAGE_MASK) | 0x3UL;
	    printk("get free page\n");
        }
        pmd = phys_to_virt_k(pud[i2] & PAGE_MASK);

        /* PMD */
        if (!(pmd[i3] & 1UL)) {
            unsigned long np = __get_free_pages(GFP_KERNEL | __GFP_ZERO, 0);
            if (!np) return -ENOMEM;
	    //pmd[i3] = (((unsigned long)np - PAGE_OFFSET) & PAGE_MASK) | 0x3UL;
            np = virt_to_phys((void *)np);
	    pmd[i3] = (np & PAGE_MASK) | 0x3UL;
	    printk("get free page\n");
        }
        pte = phys_to_virt_k(pmd[i3] & PAGE_MASK);

        /* PTE */
        pte[i4] = (phys & PAGE_MASK) | PTE_FLAGS;
    }
    //flush_tlb_kernel_range(virt_start, virt_start + (phys_end - phys_start));
    return 0;
}

static int __init manual_remap_init(void)
{
    int ret;
    unsigned long *test_ptr;
    int i;

    pr_info("manual_remap_fullpt: start mapping phys [0x%lx-0x%lx] to virt 0x%lx\n",
            PHYS_START, PHYS_END, VSTART_ADDR);

    ret = manual_remap_range(PHYS_START, PHYS_END, VSTART_ADDR);
    if (ret) {
        pr_err("manual_remap_fullpt: remap_range failed %d\n", ret);
        return ret;
    }
    manual_flush_tlb_kernel_range(VSTART_ADDR, VSTART_ADDR + 0x1000);
    pr_info("manual_remap_fullpt: mapping done\n");

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

