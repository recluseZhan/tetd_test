#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/kfifo.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

#define PAGE_SIZE 4096
#define RESERVED_PHYS_ADDR 0x1000
static void __iomem *mapped_page = NULL;

int kernel_pa2va(void){
    mapped_page = ioremap(RESERVED_PHYS_ADDR, PAGE_SIZE);
    if(!mapped_page){
        printk(KERN_ERR "failed to map pa %lx\n", RESERVED_PHYS_ADDR);
	return -ENOMEM;
    }
    printk(KERN_INFO "pa %lx to va %lx\n", RESERVED_PHYS_ADDR, mapped_page);
    
    /*
    for(int i = 0; i < PAGE_SIZE; i++){
	if(ioread8(mapped_page+i) != 0){
            printk(KERN_INFO "%u\n", ioread8(mapped_page+i));    
	}
    } 
    for(int i = 0; i < PAGE_SIZE; i++){
        iowrite8(2,mapped_page+i);
    }*/

    return 0;
}

#define FIFO_SIZE 1024
static struct kfifo tdi_fifo;
int fifo_map(void){
    int ret;
    ret = kfifo_init(&tdi_fifo, (void*)mapped_page, FIFO_SIZE);
    if(ret){
        printk(KERN_INFO "failed to fifo mapping\n");
        iounmap(mapped_page);
        return ret;
    }
    printk(KERN_INFO "fifo mapped\n");
    return 0;
}

void private2share(void){
    uint64_t gpa = 0x1000;
    uint64_t opcode = 0x10001;
    uint64_t page_size = PAGE_SIZE;
    uint64_t ret = 0;
    
    asm(
           "movq $0,%%r10;\n\t"
           "movq %1,%%r11;\n\t"
	   "movq %2,%%r12;\n\t"
	   "movq %3,%%r13;\n\t"
	   "tdcall;\n\t"
	   "movq %%r10,%0;\n\t"
	   :"=r"(ret)
	   :"r"(opcode), "r"(gpa), "r"(page_size)
       );
    /*
    asm(
           "movq %1, %%r10;\n\t"
           "movq %2, %%r11;\n\t"
           "movq %3, %%r12;\n\t"
           "tdcall;\n\t"
           "movq %%r10, %0;\n\t"
           :"=r"(ret)
           :"r"(opcode), "r"(gpa), "r"(page_size)
       );
    */
    printk("ret=%lx\n", ret);

}

void set_clean_env(void){
    memset(mapped_page, 0, PAGE_SIZE);
    kfifo_reset(&tdi_fifo);
}

void fifo_in(unsigned long va){
    kfifo_in(&tdi_fifo, va, 1);
}

#define DUMP_SIZE 512
void dump(u8 *data){
    kfifo_in(&tdi_fifo, data, DUMP_SIZE);
    for(int i = 0; i< FIFO_SIZE; i++){
	printk(KERN_CONT "%u ",ioread8(mapped_page+i));
    }
}

void tdi_run(u8 *data, uint64_t data_size){
    if(kfifo_len(&tdi_fifo) + DUMP_SIZE > FIFO_SIZE){
        set_clean_env();
    }
    
    for(int i = 0; i <= data_size/DUMP_SIZE; i++){
        dump(data + i * DUMP_SIZE);
    }
    
    private2share();
    printk("\n\n");
}

uint64_t read_func(void){
    uint64_t gva;
    uint32_t low, high;
    low = ioread32(mapped_page);
    high = ioread32(mapped_page + 4);
    gva = ((uint64_t)high << 32) | low;
    printk(KERN_INFO "target gva: %llx\n", gva);
    //memset(mapped_page, 0, 8);
    return gva;
}

static int __init tdi_init(void){
    printk(KERN_INFO "loading tdi module\n");
    kernel_pa2va();
    fifo_map();
    set_clean_env();
    //private2share();
    
    return 0;
}
static void __exit tdi_exit(void){
    iounmap(mapped_page);
    //private2share();
    printk(KERN_INFO "unloading tdi module\n");
}
EXPORT_SYMBOL(tdi_run);
EXPORT_SYMBOL(read_func);
module_init(tdi_init);
module_exit(tdi_exit);
