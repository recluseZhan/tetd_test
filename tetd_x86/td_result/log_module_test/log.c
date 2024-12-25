#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <asm/io.h>
MODULE_LICENSE("GPL");

#define PAGE_SIZE 4096
unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}

//#define GPA 0x1067ef000
//#define GPA2 0x1000
#define SIZE 128
static char* page_memory = NULL;
//static char __iomem *vaddr;

int log_copy(void){
    unsigned long t1,t2,t3,t4,t_all;
   
    struct task_struct *task = current;   // 获取当前进程的信息
    struct tty_struct *tty = task->signal->tty;
    struct timespec64 ts;

    char info_str_login[256];
    char *page_memory;  // 指向分配的物理页内存
    size_t info_len; 
    
    // 分配适当大小的内存，确保页面对齐
    page_memory = (char *)__get_free_pages(GFP_KERNEL, 0);

    if (!page_memory) {
        pr_err("Failed to allocate memory page.\n");
        return -ENOMEM;
    }
    int pid1=1;
    int uid1=task->cred->uid.val;
    int tty1=1;
    int ses1=task->sessionid;
    // 获取当前时间
    ktime_get_real_ts64(&ts);
   t1 = urdtsc();
     snprintf(info_str_login, sizeof(info_str_login),
        "PID: %d, UID: %d, TTY Number: %d, Session ID: %d, Current time: %lld.%09lld",
        pid1,
        uid1,
        tty1,
        ses1,
        ts.tv_sec, ts.tv_nsec);
    
    // 输出格式化的字符串
    //pr_info("Process Info: %s\n", info_str_login);    
    t2=urdtsc();
    t_all=(t2-t1)*5/12;
    printk("time(login_get)=%ld",t_all);     
    // 计算字符串的实际长度（不包含 "END"）
    info_len = strlen(info_str_login);

    // 确保字符串长度 + "END" 不超过 4KB
    if (info_len + 4 > PAGE_SIZE) {
        pr_warn("String is too long to fit in a single page!\n");
        return -ENOMEM;
    }

    // 将格式化字符串和 "END" 拷贝到分配的内存中
    t3=urdtsc();
    memcpy(page_memory,info_str_login,info_len);
    //snprintf(page_memory, info_len + 4, "%sEND", info_str_login);
    t4=urdtsc();
    t_all=(t4-t3)*5/12;
    printk("time(login_copy)=%ld",t_all);

    pr_info("info size:%d", info_len);
    // 输出物理页面内存的地址和内容（可选）
    pr_info("Page memory address: %p\n", page_memory);
    pr_info("Page content: %s\n", page_memory);

    return 0;
}
int log_copy2(void){
    unsigned long t1,t2,t3,t4,t_all;

    struct task_struct *task = current;   // 获取当前进程的信息
    struct tty_struct *tty = task->signal->tty;
    struct timespec64 ts;

    char info_str_module[128];
    char *page_memory2;  // 指向分配的物理页内存
    size_t info_len;

    // 分配适当大小的内存，确保页面对齐
    page_memory2 = (char *)__get_free_pages(GFP_KERNEL, 0);

    if (!page_memory2) {
        pr_err("Failed to allocate memory page.\n");
        return -ENOMEM;
    }
    ktime_get_real_ts64(&ts);
    t1 = urdtsc();
    scnprintf(info_str_module, sizeof(info_str_module),
       "NAME: %s, UID: %d, PPID: %d, Parent Process Name: %s, Current time: %lld.%09lld",
       "Malware",
       task->cred->uid.val,
       task->real_parent->pid,
       task->real_parent->comm,
       ts.tv_sec, ts.tv_nsec);
    t2=urdtsc();
    t_all=(t2-t1)*5/12;
    printk("%d",sizeof(char));
    printk("time(module_get)=%ld",t_all);
    // 计算字符串的实际长度（不包含 "END"）
    info_len = strlen(info_str_module);

    // 确保字符串长度 + "END" 不超过 4KB
    if (info_len + 4 > PAGE_SIZE) {
        pr_warn("String is too long to fit in a single page!\n");
        return -ENOMEM;
    }

    // 将格式化字符串和 "END" 拷贝到分配的内存中
    t3=urdtsc();
    memcpy(page_memory2,info_str_module,info_len);
    //snprintf(page_memory2, info_len + 4, "%sEND", info_str_module);
    t4=urdtsc();
    t_all=(t4-t3)*5/12;
    printk("time(module_copy)=%ld",t_all);

    pr_info("info size:%d", info_len);
    // 输出物理页面内存的地址和内容（可选）
    pr_info("Page memory address: %p\n", page_memory2);
    pr_info("Page content: %s\n", page_memory2);
   
    return 0;
}
/*
*/
static int __init log_init(void)
{
    printk(KERN_INFO "Entering page module\n");
    //vaddr = ioremap(GPA, SIZE);
    //if (!vaddr) {
    //    pr_err("Failed to map physical address 0x%lx\n", (unsigned long)GPA);
    //    return -ENOMEM;
    //}
    return 0;
}

static void __exit log_exit(void)
{
    printk(KERN_INFO "Exiting page module\n");
     //if (vaddr) {
    //    iounmap(vaddr);  // 解除映射
    //    pr_info("Unmapped virtual address: %p\n", vaddr);
    //}
}
EXPORT_SYMBOL(log_copy);
EXPORT_SYMBOL(log_copy2);
module_init(log_init);
module_exit(log_exit);

