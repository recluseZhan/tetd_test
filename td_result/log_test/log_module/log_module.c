#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");

void log_call(unsigned long gpa_addr) {
    char *data;

    // 将GPA地址映射到内核地址空间
    data = phys_to_virt(gpa_addr);
    if (!data) {
        pr_err("Failed to map GPA address\n");
        return;
    }

    // 打印地址和数据内容
    pr_info("Received GPA address: %lx\n", gpa_addr);
    pr_info("Data at GPA address: %s\n", data);
}
EXPORT_SYMBOL(log_call);

static int __init log_module_init(void) {
    pr_info("log_module loaded\n");
    return 0;
}

static void __exit log_module_exit(void) {
    pr_info("log_module unloaded\n");
}

module_init(log_module_init);
module_exit(log_module_exit);

