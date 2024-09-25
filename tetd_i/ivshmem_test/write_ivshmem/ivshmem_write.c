#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/pci.h>

#define IVSHMEM_BAR0_ADDRESS 0x383800000000
#define IVSHMEM_BAR0_SIZE (1 * 1024 * 1024)


void __iomem *ivshmem_base;  // 保存映射的基地址
struct task_struct *get_task(pid_t pid) {
    struct pid *pid_struct = find_get_pid(pid); // 获取 PID 结构
    if (!pid_struct) {
        return NULL; // PID 不存在
    }
    struct task_struct *task = pid_task(pid_struct, PIDTYPE_PID); // 获取 task_struct
    return task;
}
static int copy_pcb(void){
   struct task_struct *task;
   //task = current;
   task = get_task(2);
   printk("task:0x%lx, task_size=0x%lx",task,sizeof(struct task_struct));
   memcpy(ivshmem_base, task, sizeof(struct task_struct));
   return 0;
}

static int copy_idt(void){
    struct desc_ptr idt_desc;
    unsigned long idt_base;
    size_t idt_size = sizeof(struct desc_struct) * (IDT_ENTRIES);

    // 获取 IDT 的基地址
    asm volatile("sidt %0" : "=m"(idt_desc));
    idt_base = idt_desc.address;
    
    printk("idt:0x%lx, idt_size:0x%lx",idt_base,idt_size);
    memcpy(ivshmem_base, (void *)idt_base, idt_size);

    printk(KERN_INFO "Copied IDT to physical address 0x%lx\n", IVSHMEM_BAR0_ADDRESS);
    return 0; 
}

void read_pcb(void){
    unsigned long value;
    for(int i = 0; i < sizeof(struct task_struct)/8; i++){
        value = readq(ivshmem_base + i * 8);
        pr_info(KERN_CONT "0x%x ", value);
    }
}

void read_idt(void){
    unsigned long value;
    for(int i = 0; i < 256; i++){
        value = readq(ivshmem_base + i * 8);
	pr_info("Read value 0x%x from offset 0x%x\n", value, i*8);
    }
}
// 写入 ivshmem 的函数
static void write_ivshmem(int offset, uint32_t value) {
    if (offset < IVSHMEM_BAR0_SIZE) {
        writel(value, ivshmem_base + offset);  // 写入指定偏移的值
        pr_info("Wrote value 0x%x to offset 0x%x\n", value, offset);
    } else {
        pr_err("Offset out of bounds: 0x%x\n", offset);
    }
}

// 读取 ivshmem 的函数
static uint32_t read_ivshmem(int offset) {
    uint32_t value;

    if (offset < IVSHMEM_BAR0_SIZE) {
        value = readl(ivshmem_base + offset);  // 从指定偏移读取值
        pr_info("Read value 0x%x from offset 0x%x\n", value, offset);
        return value;
    } else {
        pr_err("Offset out of bounds: 0x%x\n", offset);
        return 0;
    }
}
// 初始化函数：映射共享内存
static int __init ivshmem_init(void) {
    uint32_t read_value;

    pr_info("Initializing ivshmem module...\n");

    // 映射 ivshmem 的 BAR 0 内存区域
    ivshmem_base = ioremap(IVSHMEM_BAR0_ADDRESS, IVSHMEM_BAR0_SIZE);
    if (!ivshmem_base) {
        pr_err("Could not map ivshmem memory\n");
        return -EIO;
    }

    pr_info("ivshmem memory mapped at address: %p\n", ivshmem_base);

    // 测试写入数据
  //  write_ivshmem(0, 0x12345678);  // 将 0x12345678 写入偏移 0
    //copy_idt();
    //read_idt();
    copy_pcb();
    read_pcb();  
  //mb();
    // 测试读取数据
    //read_value = read_ivshmem(0);  // 读取偏移 0 的数据
    //pr_info("Read value 0x%x from offset 0x0\n", read_value);

    return 0;
}

// 退出函数：解除映射
static void __exit ivshmem_exit(void) {
    if (ivshmem_base) {
        iounmap(ivshmem_base);
        pr_info("ivshmem memory unmapped\n");
    }

    pr_info("Exiting ivshmem module\n");
}


module_init(ivshmem_init);
module_exit(ivshmem_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("IVSHMEM Kernel Module for Reading and Writing to Shared Memory");

