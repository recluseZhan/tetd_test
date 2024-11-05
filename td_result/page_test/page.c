#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/io.h>

#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/pid.h>
#include <linux/uaccess.h>
#include <asm/pgtable.h>
#include <asm/page.h>

MODULE_LICENSE("GPL");

unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}


#define PGD_SIZE (sizeof(pgd_t) * PTRS_PER_PGD)

void pgd_copy(void){
    struct task_struct *task=current;
    pgd_t *old_pgd,*new_pgd;
    old_pgd = task->mm->pgd;
    phys_addr_t ret_cr3,in_cr3;
    
    unsigned long t1,t2,t_all;
    t1=urdtsc();
    new_pgd = kmalloc(PGD_SIZE,GFP_KERNEL);
    memcpy(new_pgd,old_pgd,PGD_SIZE);
    in_cr3 = virt_to_phys(new_pgd);
    
    asm volatile(
        //"movq %%cr3,%%rax\n\t"
        "movq %0,%%cr3\n\t"
        //"movq %%rax,%0\n\t"
        ::"r"(in_cr3):
    );
    t2=urdtsc();
    t_all=(t2-t1)*5/12;
    printk("time=%ld",t_all); 

    /*printk("old:%lx",old_pgd);
    for(int i=0;i<PGD_SIZE;i++){
        printk(KERN_CONT "%hhu ", *(old_pgd+i));
    }
    
    printk("");   
    printk("new:%lx",new_pgd);
    for(int i=0;i<PGD_SIZE;i++){
        printk(KERN_CONT "%hhu ", *(new_pgd+i));
    } */
}
/*
#define PAGE_SIZE_4KB 4096

static void *allocated_page = NULL;
static unsigned long mapped_va = 0;

static int map_data_to_empty_pte(void)
{
    struct page *page;
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long va;
    int err = 0;

    // Step 1: Allocate a 4KB page-aligned buffer.
    allocated_page = (void *)get_zeroed_page(GFP_KERNEL);
    if (!allocated_page) {
        pr_err("Failed to allocate 4KB page-aligned memory\n");
        return -ENOMEM;
    }

    // Step 2: Choose a virtual address for mapping.
    // This is just an example address; choose an address in the kernel space.
    va = __get_free_page(GFP_KERNEL);
    if (!va) {
        pr_err("Failed to allocate virtual address\n");
        free_page((unsigned long)allocated_page);
        return -ENOMEM;
    }
    mapped_va = va;

    // Step 3: Get the page structure for the allocated memory.
    page = virt_to_page(allocated_page);

    // Step 4: Set up the page table entry (PTE).
    pgd = pgd_offset_k(va);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        pr_err("Invalid PGD\n");
        err = -EFAULT;
        goto out_free;
    }

    p4d = p4d_offset(pgd, va);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {
        pr_err("Invalid P4D\n");
        err = -EFAULT;
        goto out_free;
    }

    pud = pud_offset(p4d, va);
    if (pud_none(*pud) || pud_bad(*pud)) {
        pr_err("Invalid PUD\n");
        err = -EFAULT;
        goto out_free;
    }

    pmd = pmd_offset(pud, va);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        pr_err("Invalid PMD\n");
        err = -EFAULT;
        goto out_free;
    }

    pte = pte_offset_kernel(pmd, va);
    if (!pte) {
        pr_err("Invalid PTE\n");
        err = -EFAULT;
        goto out_free;
    }

    // Map the page into the PTE.
    set_pte_at(&init_mm, va, pte, mk_pte(page, PAGE_KERNEL));

    pr_info("Mapped virtual address 0x%lx to allocated page\n", va);
    return 0;

out_free:
    free_page((unsigned long)allocated_page);
    if (mapped_va)
        free_page(mapped_va);
    return err;
}

int map_pte(void)
{
    int ret = map_data_to_empty_pte();
    if (ret) {
        pr_err("Mapping data to empty PTE failed\n");
    }
    return ret;
}

void unmap_pte(void)
{
    if (mapped_va) {
        free_page(mapped_va);
        pr_info("Freed mapped virtual address 0x%lx\n", mapped_va);
    }
    if (allocated_page) {
        free_page((unsigned long)allocated_page);
        pr_info("Freed allocated page\n");
    }
}
*/
static int __init page_init(void)
{
    printk(KERN_INFO "Entering page module\n");
    return 0;
}

static void __exit page_exit(void)
{
    printk(KERN_INFO "Exiting page module\n");
}
EXPORT_SYMBOL(pgd_copy);
//EXPORT_SYMBOL(map_pte);
//EXPORT_SYMBOL(unmap_pte);
module_init(page_init);
module_exit(page_exit);

