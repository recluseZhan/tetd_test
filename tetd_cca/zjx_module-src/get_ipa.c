#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kvm_host.h>
#include <linux/mm.h>
#include <asm/kvm_rme.h>
#include <linux/moduleparam.h>
#include <linux/page-flags.h>
#include <asm/rsi.h>

static unsigned long base_ipa = 0x90000000;
module_param(base_ipa, ulong, S_IRUGO);
MODULE_PARM_DESC(base_ipa, "The address of the base_ipa");

static unsigned long size = 4096;
module_param(size, ulong, S_IRUGO);
MODULE_PARM_DESC(size, "This is size");

static unsigned long empty = 0;
module_param(empty, ulong, S_IRUGO);
MODULE_PARM_DESC(empty, "This is empty flag");

static int __init realm_get_init(void)
{
    phys_addr_t start_ipa = (phys_addr_t)base_ipa;
    phys_addr_t end_ipa = base_ipa + size;
    
    if(empty==0){
        set_memory_range_protected(start_ipa,end_ipa);
    }else{
        set_memory_range_shared(start_ipa,end_ipa);
    }
    printk(KERN_INFO "Protected memory getting successful.\n");
    return 0;
}

static void __exit realm_get_exit(void)
{
    printk(KERN_INFO "Exiting Realm getting Module...\n");
}

module_init(realm_get_init);
module_exit(realm_get_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A kernel module to get protected memory in a Realm.");

