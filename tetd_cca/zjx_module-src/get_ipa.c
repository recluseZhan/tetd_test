#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kvm_host.h>
#include <linux/mm.h>
#include <asm/kvm_rme.h>
#include <linux/moduleparam.h>
#include <linux/page-flags.h>
#include <asm/rsi.h>

static unsigned long base_ipa = 0x7ffff000;
module_param(base_ipa, ulong, S_IRUGO);
MODULE_PARM_DESC(base_ipa, "The address of the base_ipa");

static int __init realm_get_init(void)
{
    phys_addr_t start_ipa = base_ipa;
    phys_addr_t end_ipa = base_ipa + 0x1000;
    
    set_memory_range_protected(start_ipa,end_ipa);

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

