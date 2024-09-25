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
void ptid(void *vcpu){
    //unsigned int cur_cpu = smp_processor_id();
    printk("step1 IPI cur_pid: %d\n\n\n",get_cpu());
}
void do_smp_call(void){
    //cpumask_t cpu_mask;
    int cpu_id = get_cpu();
    printk("attack cpu_id: %d\n\n\n\n",cpu_id);
    smp_call_function_single(1,ptid,NULL,1);
}

static int __init interrupt_init(void)
{   
    do_smp_call();
    printk(KERN_ALERT"handle_limit module is entering..\n");
    return 0;
}

static void __exit interrupt_exit(void)
{
    printk(KERN_ALERT"handle_limit module is leaving..\n");
}


module_init(interrupt_init);
module_exit(interrupt_exit);
