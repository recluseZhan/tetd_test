#define KVM_HYPERCALL   ".byte 0x0f,0x01,0xc1"
#define KVM_HC_VAPIC_POLL_IRQ  2

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
//vmcall
static inline long kvm_hypercall0(unsigned int nr)
{
	long ret;
	asm volatile(KVM_HYPERCALL
		     : "=a"(ret)
		     : "a"(nr)
		     : "memory");
	return ret;
}

static inline long kvm_hypercall4(unsigned int nr, unsigned long p1,
				  unsigned long p2, unsigned long p3,
				  unsigned long p4)
{
    long ret;
    asm volatile(KVM_HYPERCALL
        : "=a"(ret)
        : "a"(nr), "b"(p1), "c"(p2), "d"(p3), "S"(p4)
        : "memory");
    return ret;
}
void bind_pa(unsigned long gpa, unsigned long hpa){
    kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ, hpa, gpa, 0, 0);
}
void target_dump(unsigned long hpa, unsigned long gpa, unsigned long target_gva, unsigned long flag){
    kvm_hypercall4(KVM_HC_VAPIC_POLL_IRQ, hpa, gpa, target_gva, flag);
}
static int __init hyper_init(void){
    printk(KERN_INFO "loading hyper module\n");

    return 0;
}
static void __exit hyper_exit(void){
    printk(KERN_INFO "unloading hyper module\n");
}

EXPORT_SYMBOL(bind_pa);
EXPORT_SYMBOL(target_dump);
module_init(hyper_init);
module_exit(hyper_exit);
