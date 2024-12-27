#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kvm_host.h>
#include <linux/mm.h>
#include <asm/kvm_rme.h>
#include <linux/moduleparam.h>
#include <linux/page-flags.h>

static unsigned long vcpu_addr = 0x0;
module_param(vcpu_addr, ulong, S_IRUGO);
MODULE_PARM_DESC(vcpu_addr, "The address of the VCPU to map protected memory");

static unsigned long base_ipa = 0x7ffff000;
module_param(base_ipa, ulong, S_IRUGO);
MODULE_PARM_DESC(base_ipa, "The address of the base_ipa");

static struct page *dst_page;
static struct kvm_vcpu *vcpu;
static struct kvm *kvm;
static struct realm *realm;
static unsigned long size = 0x1000;
static unsigned long dst_page_pa;
static struct kvm_mmu_memory_cache *memcache;

int realm_map(void){
    int ret;
    ret = realm_map_protected(realm, 0, base_ipa, dst_page, size, memcache);
    if (ret) {
        printk(KERN_ERR "Failed to map protected memory.\n");
        return -EFAULT;
    }

    printk(KERN_INFO "Protected memory mapping successful.\n");
    return 0; 
}
EXPORT_SYMBOL(realm_map);

static int __init realm_map_init(void)
{
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

    memcache = &vcpu->arch.mmu_page_cache;
    printk(KERN_INFO "memcache:%lx\n",memcache);

    dst_page = alloc_page(GFP_KERNEL);
    if (!dst_page) {
        printk(KERN_ERR "Failed to allocate memory page.\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "dst_va:%lx,dst_pa:%lx",dst_page,virt_to_phys(dst_page));

    return 0;
}

static void __exit realm_map_exit(void)
{
    printk(KERN_INFO "Exiting Realm Mapping Module...\n");
}

module_init(realm_map_init);
module_exit(realm_map_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A kernel module to map protected memory in a Realm.");

