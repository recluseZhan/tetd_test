#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/io.h>
MODULE_LICENSE("GPL");

//unsigned long new_pgd[512];
unsigned long new_pud[512];
unsigned long new_pmd[512];
unsigned long new_pte[512];

unsigned long pgd_all[512];
unsigned long pud_all[512^2];
unsigned long pmd_all[512^3];
unsigned long pte_all[512^4];
#define PGD_SIZE (sizeof(pgd_t) * PTRS_PER_PGD)
#define NEW_STACK_SIZE 8192
/*
unsigned long *cr3;
pgd_t new_pgd[PGD_SIZE];
pgd_t* old_pgd;
pgd_t* new_pgd1;
*/
void stack_copy(void){
    uint8_t *new_stack;
    new_stack = kmalloc(NEW_STACK_SIZE, GFP_KERNEL);
    asm volatile(
        "movq %0,%%rsp\n\t"
        "sub $8,%%rsp\n\t"
        "call continue_func\n\t"
        ::"r"(new_stack+NEW_STACK_SIZE):
    );
}
void continue_func(void *new_stack){
    int a =1;
    printk("a = %lx\n",&a);
}
void pgd_copy(unsigned long t_pid){
    struct task_struct *task=current;
    pgd_t *old_pgd,*new_pgd;
    old_pgd = task->mm->pgd;
    new_pgd = kmalloc(PGD_SIZE,GFP_KERNEL);
    memcpy(new_pgd,old_pgd,PGD_SIZE);
    //for(int i=0;i<PGD_SIZE;i++){
    //    printk("%lx %lx\n",*(old_pgd+i),*(new_pgd+i));
    //}
    phys_addr_t ret_cr3,in_cr3;
    in_cr3 = virt_to_phys(new_pgd);
    //unsigned long *vcr3;
    asm volatile(
        //"movq %%cr3,%%rax\n\t"
        "movq %0,%%cr3\n\t"
        //"movq %%rax,%0\n\t"
        ::"r"(in_cr3):
    );
    stack_copy();
    
    //vcr3=phys_to_virt(ret_cr3);
    //printk("cr3 %lx %lx v:%lx %lx\n",ret_cr3,in_cr3,vcr3,new_pgd);
/*
    for(int i=0;i<PGD_SIZE;i++){
        new_pgd[i]=*(old_pgd+i);
    }
    new_pgd1 = new_pgd;
    for(int i=0;i<PGD_SIZE;i++){
        //printk("%lx %lx\n",*(old_pgd+i),*(new_pgd1+i));
    }
    printk("add %lx %lx\n",old_pgd,new_pgd1);
    //task->mm->pgd = new_pgd1;
    pgd_t *new_pgd2;
    new_pgd2 = kmalloc(4096,GFP_KERNEL);
    memcpy(new_pgd2,old_pgd,4096);
    for(int i=0;i<4096;i++){
        printk("%lx %lx\n",*(old_pgd+i),*(new_pgd2+i));
    }
    phys_addr_t ret_cr3,in_cr3;
    in_cr3 = virt_to_phys(new_pgd2);
    unsigned long *vcr3;
    asm volatile(
        "movq %%cr3,%%rax\n\t"
        "movq %1,%%cr3\n\t"
        "movq %%rax,%0\n\t"
        :"=r"(ret_cr3):"r"(in_cr3):
    );
    vcr3=phys_to_virt(ret_cr3);
    printk("cr3 %lx %lx\n",ret_cr3,vcr3);
    */
}
/*
void all_copy(unsigned long t_pid){
    unsigned long *pgd, *pud, *pmd, *pte;
    struct task_struct *task,*p;
    struct list_head *pos;
    int count = 0;
    task = &init_task;
    list_for_each(pos,&task->tasks)
    {
        p=list_entry(pos, struct task_struct, tasks);
        count++;
	if (p->pid == t_pid)
	{
	    pgd = (unsigned long)p->mm->pgd;
	    break;
	}
    }
    unsigned long t1=0;
    unsigned long t2=0;
    unsigned long t3=0;
    unsigned long t4=0;
    for(unsigned long i=0;i<512;i++){
        pgd_all[t1] = *(pgd+i);
        pud = (unsigned long *)(((unsigned long)pgd_all[t1] & PTE_PFN_MASK) + PAGE_OFFSET);
        t1++;
        for(unsigned long j=0;j<512;j++){
            pud_all[j] = *(pud+j);         
            t2++;
        }
    }
    for(unsigned long i=0;i<512^2;i++){
        pmd = (unsigned long *)(((unsigned long)pud_all[i] & PTE_PFN_MASK) + PAGE_OFFSET);
        for(unsigned long j=0;j<512;j++){
            pmd_all[t3]=*(pmd+j);
            t3++;
        }
    }
    for(unsigned long i=0;i<512^3;i++){
        pte = (unsigned long *)(((unsigned long)pmd_all[i] & PTE_PFN_MASK) + PAGE_OFFSET);
        for(unsigned long j=0;j<512;j++){
            pte_all[t4]=*(pte+j);
            t4++;
        }
    }
}


unsigned long copy_table(unsigned long vaddr,unsigned long t_pid){
    unsigned long paddr=0;
    unsigned long page_addr=0;
    unsigned long P_OFFSET=0;
    unsigned long *pgd, *pud, *pmd, *pte;
    
    struct task_struct *task,*p;
    struct list_head *pos;
    int count = 0;
    
    task = &init_task;
    list_for_each(pos,&task->tasks)
    {
        p=list_entry(pos, struct task_struct, tasks);
        count++;
	if (p->pid == t_pid)
	{
	    pgd = (unsigned long)p->mm->pgd;
	    for(int i=0;i<512;i++){
	        new_pgd[i] = *(pgd+i);
	    }
	    pgd = pgd + ((vaddr>>39) & 0x1FF);
	    break;
	}
    }
    
    pud = (unsigned long *)(((unsigned long)*pgd & PTE_PFN_MASK) + PAGE_OFFSET);
    for(int i=0;i<512;i++){
	new_pud[i] = *(pud+i);
    }
    pud = pud + ((vaddr>>30) & 0x1FF);
    
    pmd = (unsigned long *)(((unsigned long)*pud & PTE_PFN_MASK) + PAGE_OFFSET);
    for(int i=0;i<512;i++){
	new_pmd[i] = *(pmd+i);
    }
    pmd = pmd + ((vaddr>>21) & 0x1FF);

    pte = (unsigned long *)(((unsigned long)*pmd & PTE_PFN_MASK) + PAGE_OFFSET);
    for(int i=0;i<512;i++){
	new_pte[i] = *(pte+i);
    }
    //pte = pte + ((vaddr>>12) & 0x1FF);  
    //page_addr= (*pte) & PAGE_MASK;
    //P_OFFSET=vaddr&~PAGE_MASK;
    //paddr=page_addr|P_OFFSET;
    return 0;
}

void change_cr3(unsigned long t_pid) {
    struct task_struct *task,*p;
    struct list_head *pos;
    int count = 0;
    task = &init_task;
    list_for_each(pos,&task->tasks)
    {
        p=list_entry(pos, struct task_struct, tasks);
        count++;
	if (p->pid == t_pid)
	{
            p->mm->pgd = (pgd_t *)new_pgd;
            break;
	}
    }
}*/

static int __init page_init(void)
{
   
    //int pid = 5098; // Example PID
    //unsigned long pa;
    //unsigned long va =0x7ffcb57e2a30;
    //pa = v2p(va,pid);
    // Allocate and initialize new page table
   

    // Map some physical memory to the new page table
    // Example: Map physical address 0x10000 to virtual address 0xFFFF000000000000
    //mapPage(newPageTable, &a, 0x10000);
    //changeProcessPageTable_by_pid(current->pid, newPageTable);
    //printk("va=%lx pa=%lx\n",va,pa);
    // Change page table for process with given PID
    //changeProcessPageTable_by_pid(pid, newPageTable);
    printk(KERN_INFO "Entering page module\n");
    return 0;
}

static void __exit page_exit(void)
{
    printk(KERN_INFO "Exiting page module\n");
}
EXPORT_SYMBOL(pgd_copy);
/*
EXPORT_SYMBOL(copy_table);
EXPORT_SYMBOL(change_cr3);*/
module_init(page_init);
module_exit(page_exit);

