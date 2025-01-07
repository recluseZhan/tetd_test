#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kvm_host.h>
#include <linux/mm.h>
#include <asm/kvm_rme.h>
#include <linux/moduleparam.h>
#include <linux/page-flags.h>
#include <asm/rmi_cmds.h>
static unsigned long vcpu_addr = 0x0;
module_param(vcpu_addr, ulong, S_IRUGO);
MODULE_PARM_DESC(vcpu_addr, "The address of the VCPU to map protected memory");

static unsigned long base_ipa = 0x20000000;
module_param(base_ipa, ulong, S_IRUGO);
MODULE_PARM_DESC(base_ipa, "The address of the base_ipa");

static struct kvm_vcpu *vcpu;
static struct kvm *kvm;
static struct realm *realm;
static phys_addr_t rd;


static int __init unblock_init(void)
{
    int ret = 0;
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

    
    rd = virt_to_phys(realm->rd);

    ret = rmi_ipa_unblock(rd,base_ipa);
    if (ret) {
        printk(KERN_ERR "Failed to unblock memory.\n");
        return -EFAULT;
    }
    printk(KERN_INFO "Unblock successful.\n");

    return 0;
}

static void __exit unblock_exit(void)
{
    printk(KERN_INFO "Exiting Realm Mapping Module...\n");
}

module_init(unblock_init);
module_exit(unblock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("unblock test");

