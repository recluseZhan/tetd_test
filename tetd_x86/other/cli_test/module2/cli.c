#include <asm/processor.h>
#include <asm/alternative.h>
#include <linux/interrupt.h>
#include <uapi/asm/kvm_para.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/current.h>
#include <linux/sched.h>

#include <asm/uaccess.h>
#include <linux/ctype.h>
#include <linux/smp.h>

#include <asm/segment.h>
#include <linux/buffer_head.h>
#include <asm/processor.h>
#include <linux/sched.h>
#include <linux/delay.h>
MODULE_LICENSE("GPL");
void clear_ifg(void)
{
	__asm("cli \n"
		:
		:
		:
		);
}

void start_ifg(void)
{
	__asm("sti \n"
		:
		:
		:
		);
}
static int __init cli_init(void)
{   
    
    int cpu_id = 1;
    /*
    struct task_struct *task;
    cpumask_t mask;
    cpumask_clear(&mask);
    cpumask_set_cpu(cpu_id,&mask);
    task=current;
    if(sched_setaffinity(task,&mask)){
        printk("file\n");
        return -1;
    }*/
    printk("cli:cpu_id:%d\n",cpu_id);
    printk(KERN_ALERT"cli module is entering..\n");
    //clear_ifg();
    msleep(5);
    printk("sleep\n\n\n");
    //start_ifg();
    return 0;
}

static void __exit cli_exit(void)
{
    printk(KERN_ALERT"cli module is leaving..\n");
}


module_init(cli_init);
module_exit(cli_exit);
