#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>        // ioremap 和 iounmap
#include <linux/fs.h>        // 注册字符设备
#include <linux/uaccess.h>   // copy_to_user 和 copy_from_user
#include <linux/param.h>     // 命令行参数

#define DEVICE_NAME "ioremap_test"  // 设备名称
#define IO_ADDRESS_SIZE 0x1000     // 映射的大小为 4KB

static unsigned long base_ipa = 0x0;  // 默认物理地址，后续可以通过参数传递
static void *virt_addr = NULL;          // 虚拟地址指针

module_param(base_ipa, ulong, S_IRUGO); // 允许通过命令行传递物理地址

static int __init ioremap_test_init(void)
{
    printk(KERN_INFO "Initializing ioremap_test module\n");

    // 使用 ioremap 将物理地址映射到内核虚拟地址
    virt_addr = ioremap(base_ipa, IO_ADDRESS_SIZE);
    if (!virt_addr) {
        printk(KERN_ERR "Failed to map physical address %lx to virtual address\n", base_ipa);
        return -ENOMEM;
    }

    printk(KERN_INFO "Mapped physical address %lx to virtual address %p\n", base_ipa, virt_addr);

    // 对映射的内存进行读写操作
    // 假设我们映射的地址是一个字节数组
    *(unsigned char *)virt_addr = 0xAA;  // 写入数据
    printk(KERN_INFO "Write value 0xAA to mapped address %p\n", virt_addr);

    unsigned char val = *(unsigned char *)virt_addr;  // 读取数据
    printk(KERN_INFO "Read value 0x%x from mapped address %p\n", val, virt_addr);

    return 0;
}

static void __exit ioremap_test_exit(void)
{
    if (virt_addr) {
        iounmap(virt_addr);  // 卸载映射
        printk(KERN_INFO "Unmapped virtual address %p\n", virt_addr);
    }

    printk(KERN_INFO "Exiting ioremap_test module\n");
}

module_init(ioremap_test_init);
module_exit(ioremap_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple kernel module to use ioremap for physical address mapping");

