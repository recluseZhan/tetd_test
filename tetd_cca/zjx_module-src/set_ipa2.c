#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kvm_host.h>
#include <linux/mm.h>
#include <asm/kvm_rme.h>
#include <linux/moduleparam.h>
#include <linux/page-flags.h>

extern int realm_map(void);
static int __init realm_map_init(void)
{
    realm_map();
    printk(KERN_INFO "Protected memory mapping2 successful.\n");
    return 0;
}

static void __exit realm_map_exit(void)
{
    printk(KERN_INFO "Exiting Realm Mapping2 Module...\n");
}

module_init(realm_map_init);
module_exit(realm_map_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A kernel module to map protected memory in a Realm.");

