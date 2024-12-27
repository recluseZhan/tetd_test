#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kvm_host.h>
#include <linux/mm.h>
#include <asm/kvm_rme.h>
#include <linux/moduleparam.h>
#include <linux/page-flags.h>
#include <asm/rmi_cmds.h>

static unsigned long vcpu_addr = 0;  // 默认值为 0，表示未设置
module_param(vcpu_addr, ulong, S_IRUGO);  // 定义模块参数 vcpu_addr，类型为 unsigned long（地址）
MODULE_PARM_DESC(vcpu_addr, "The address of the VCPU to map protected memory");

static unsigned long base_ipa = 0x7ffff000;
module_param(base_ipa, ulong, S_IRUGO);
MODULE_PARM_DESC(base_ipa, "The address of the base_ipa");

static int __init realm_unmap_init(void)
{
    struct kvm_vcpu *vcpu;                 // 当前 VCPU
    struct kvm *kvm;                       // KVM 实例
    struct realm *realm;                   // Realm 对象
    unsigned long rd;
    
    unsigned long size = 0x1000;           // 映射大小（4KB 页面）
    struct kvm_mmu_memory_cache memcache;
    //struct kvm_mmu_memory_cache *memcache = &vcpu->arch.mmu_page_cache;  // 内存管理缓存
    int ret;
    unsigned long data_out, top_out;
    printk(KERN_INFO "Initializing Realm Mapping...\n");

    // 获取当前 VCPU
    vcpu = (struct kvm_vcpu *)vcpu_addr;
    if (!vcpu) {
        printk(KERN_ERR "Failed to get current running VCPU.\n");
        return -EINVAL;
    }

    // 获取 KVM 实例
    kvm = vcpu->kvm;
    if (!kvm) {
        printk(KERN_ERR "Failed to get KVM instance.\n");
        return -EINVAL;
    }

    // 获取 Realm 对象
    realm = &kvm->arch.realm;
    if (!realm) {
        printk(KERN_ERR "Failed to get Realm object.\n");
        return -EINVAL;
    }
    
    rd = virt_to_phys(realm->rd);
    printk(KERN_INFO "Realm object acquired successfully.\n");
    
    ret = rmi_data_destroy(rd,base_ipa,&data_out,&top_out);
    if (ret) {
        printk(KERN_ERR "Failed to unmap protected memory.\n");
        return -EFAULT;
    }

    printk(KERN_INFO "Protected memory unmapping successful,data_out:%lx,top_out:%lx\n",data_out,top_out);
    return 0;
}

static void __exit realm_unmap_exit(void)
{
    printk(KERN_INFO "Exiting Realm Mapping Module...\n");
}

module_init(realm_unmap_init);
module_exit(realm_unmap_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A kernel module to map protected memory in a Realm.");

