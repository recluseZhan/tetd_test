#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");

#define NUM_ENTRIES 512

typedef uint64_t PageTableEntry;

typedef struct CustomPageTable {
    PageTableEntry entries[NUM_ENTRIES];
} CustomPageTable;

void initializePageTable(CustomPageTable *pageTable) {
    for (int i = 0; i < NUM_ENTRIES; ++i) {
        pageTable->entries[i] = 0; // Initialize all entries to 0
    }
}

void mapPage(CustomPageTable *pageTable, uint64_t virtualAddress, uint64_t physicalAddress) {
    uint64_t pgdIndex = (virtualAddress >> 39) & 0x1FF;
    uint64_t pudIndex = (virtualAddress >> 30) & 0x1FF;
    uint64_t pmdIndex = (virtualAddress >> 21) & 0x1FF;
    uint64_t pteIndex = (virtualAddress >> 12) & 0x1FF;

    PageTableEntry *pgdEntry = &(pageTable->entries[pgdIndex]);
    if (!(*pgdEntry & 0x1)) {
        // Allocate PUD and set present bit
        *pgdEntry = (uint64_t)kmalloc(sizeof(CustomPageTable), GFP_KERNEL) | 0x3;
        initializePageTable((CustomPageTable*)(*pgdEntry & ~0xFFF));
    }

    CustomPageTable *pud = (CustomPageTable*)(*pgdEntry & ~0xFFF);

    PageTableEntry *pudEntry = &(pud->entries[pudIndex]);
    if (!(*pudEntry & 0x1)) {
        // Allocate PMD and set present bit
        *pudEntry = (uint64_t)kmalloc(sizeof(CustomPageTable), GFP_KERNEL) | 0x3;
        initializePageTable((CustomPageTable*)(*pudEntry & ~0xFFF));
    }

    CustomPageTable *pmd = (CustomPageTable*)(*pudEntry & ~0xFFF);

    PageTableEntry *pmdEntry = &(pmd->entries[pmdIndex]);
    if (!(*pmdEntry & 0x1)) {
        // Allocate PT and set present bit
        *pmdEntry = (uint64_t)kmalloc(sizeof(CustomPageTable), GFP_KERNEL) | 0x3;
        initializePageTable((CustomPageTable*)(*pmdEntry & ~0xFFF));
    }

    CustomPageTable *pte = (CustomPageTable*)(*pmdEntry & ~0xFFF);

    // Map the physical address
    pte->entries[pteIndex] = (physicalAddress & PTE_PFN_MASK) | 0x3; // Set present bit and writeable
}

void changeProcessPageTable_by_pid(int t_pid, CustomPageTable *newPageTable) {
    unsigned long pte_mask;
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
            p->mm->pgd = (pgd_t *)newPageTable;
            break;
	}
    }
}

static int __init my_module_init(void)
{
    CustomPageTable *newPageTable;
    //int pid = 4312; // Example PID

    // Allocate and initialize new page table
    newPageTable = kmalloc(sizeof(CustomPageTable), GFP_KERNEL);
    if (!newPageTable) {
        printk(KERN_INFO "Failed to allocate new page table\n");
        return -ENOMEM;
    }
    initializePageTable(newPageTable);

    // Map some physical memory to the new page table
    // Example: Map physical address 0x10000 to virtual address 0xFFFF000000000000
    int a=1;
    mapPage(newPageTable, &a, 0x10000);
    //changeProcessPageTable_by_pid(current->pid, newPageTable);
    printk("a=%d\n",a);
    // Change page table for process with given PID
    //changeProcessPageTable_by_pid(pid, newPageTable);

    return 0;
}

static void __exit my_module_exit(void)
{
    printk(KERN_INFO "Exiting my module\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

