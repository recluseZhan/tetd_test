#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <asm/io.h>

#define MAPPED_IPA 0x8000000000  // 假设的已映射 IPA 地址

static int __init realm_program_init(void)
{
    void *mapped_mem;

    // 将映射的 IPA 地址转换为虚拟地址
    mapped_mem = ioremap(MAPPED_IPA, PAGE_SIZE);
    if (!mapped_mem) {
        pr_err("Failed to ioremap IPA: %lx\n", MAPPED_IPA);
        return -ENOMEM;
    }

    pr_info("Successfully ioremap IPA: %lx to virtual address: %p\n", MAPPED_IPA, mapped_mem);

    // 现在你可以访问映射的内存
    pr_info("Data at mapped memory: %lx\n", *((unsigned long *)mapped_mem));

    // 使用映射的内存
    // Example: 写入数据
    *((unsigned long *)mapped_mem) = 0x12345678;

    // 你可以根据需要访问或操作这个内存

    return 0;
}

static void __exit realm_program_exit(void)
{
    pr_info("Realm program module unloaded.\n");
}

module_init(realm_program_init);
module_exit(realm_program_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A Realm program to use mapped IPA memory");

