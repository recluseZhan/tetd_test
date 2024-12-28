#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kvm_host.h>
#include <linux/mm.h>
#include <asm/kvm_rme.h>
#include <linux/moduleparam.h>
#include <linux/page-flags.h>
#include <asm/rmi_cmds.h>


static unsigned long vcpu_addr = 0;
module_param(vcpu_addr, ulong, S_IRUGO);
MODULE_PARM_DESC(vcpu_addr, "The address of the VCPU to map protected memory");

static unsigned long base_ipa = 0x90000000;
module_param(base_ipa, ulong, S_IRUGO);
MODULE_PARM_DESC(base_ipa, "The address of the base_ipa");

static int __init realm_unmap_undelegate_init(void)
{
    struct kvm_vcpu *vcpu;
    struct kvm *kvm;
    struct realm *realm;
    
    unsigned long size = 0x1000;

    printk(KERN_INFO "Initializing Realm Mapping...\n");

    vcpu = (struct kvm_vcpu *)vcpu_addr;
    if (!vcpu) {
        printk(KERN_ERR "Failed to get current running VCPU.\n");
        return -EINVAL;
    }

    kvm = vcpu->kvm;
    if (!kvm) {
        printk(KERN_ERR "Failed to get KVM instance.\n");
        return -EINVAL;
    }

    realm = &kvm->arch.realm;
    if (!realm) {
        printk(KERN_ERR "Failed to get Realm object.\n");
        return -EINVAL;
    }
    
    printk(KERN_INFO "Realm object acquired successfully.\n");
    
    kvm_realm_unmap_range(kvm,base_ipa,size); 
    //realm_destroy_undelegate_range(realm,base_ipa,size);
    printk(KERN_INFO "Protected memory unmapping successful.\n");
    return 0;
}

static void __exit realm_unmap_undelegate_exit(void)
{
    printk(KERN_INFO "Exiting Realm Mapping Module...\n");
}

module_init(realm_unmap_undelegate_init);
module_exit(realm_unmap_undelegate_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A kernel module to unmap protected memory in a Realm.");

