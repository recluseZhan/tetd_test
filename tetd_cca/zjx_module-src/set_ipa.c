#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kvm_host.h>
#include <linux/mm.h>
#include <asm/kvm_rme.h>
#include <linux/moduleparam.h>
#include <linux/page-flags.h>


static unsigned long vcpu_addr = 0;  // 默认值为 0，表示未设置
module_param(vcpu_addr, ulong, S_IRUGO);  // 定义模块参数 vcpu_addr，类型为 unsigned long（地址）
MODULE_PARM_DESC(vcpu_addr, "The address of the VCPU to map protected memory");


static int __init realm_map_init(void)
{
    struct kvm_vcpu *vcpu;                 // 当前 VCPU
    struct kvm *kvm;                       // KVM 实例
    struct realm *realm;                   // Realm 对象
    unsigned long hva = 0x10000000;
    unsigned long base_ipa = 0x7ffff000; // 映射基础地址
    struct page *dst_page;
    unsigned long size = 0x1000;           // 映射大小（4KB 页面）
    unsigned long dst_page_pa;
    //kvm_pfn_t pfn = 0x100000;              // 页面的 PFN（物理帧号）
    struct kvm_mmu_memory_cache memcache;
    //struct kvm_mmu_memory_cache *memcache = &vcpu->arch.mmu_page_cache;  // 内存管理缓存
    int ret;

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

    printk(KERN_INFO "Realm object acquired successfully.\n");
    
    dst_page = alloc_page(GFP_KERNEL);
    if (!dst_page) {
        printk(KERN_ERR "Failed to allocate memory page.\n");
        return -ENOMEM;
    }
    dst_page_pa = virt_to_phys(dst_page);
    printk(KERN_INFO "dst_page_hva:%lx,dst_page_hpa:%lx",dst_page,dst_page_pa);

    memset(&memcache, 0, sizeof(memcache));
    // 调用 realm_map_protected 将内存映射到 Realm 中
    ret = realm_map_protected(realm, hva, base_ipa, dst_page, size, &memcache);
    if (ret) {
        printk(KERN_ERR "Failed to map protected memory.\n");
        return -EFAULT;
    }

    printk(KERN_INFO "Protected memory mapping successful.\n");
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

