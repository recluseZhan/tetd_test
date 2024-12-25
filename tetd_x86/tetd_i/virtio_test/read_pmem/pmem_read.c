#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>

#define FILE_PATH "/mnt/sharedfile"
#define BUFFER_SIZE 256

static int __init reader_init(void)
{
    struct file *file;
    loff_t pos = 0;
    ssize_t ret;
    char buffer[BUFFER_SIZE];
    
    // 打开文件
    file = filp_open(FILE_PATH, O_RDONLY, 0);
    if (IS_ERR(file)) {
        printk(KERN_ERR "Failed to open file for reading\n");
        return PTR_ERR(file);
    }
    
    // 读取数据
    ret = kernel_read(file, buffer, sizeof(buffer) - 1, &pos);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read from file\n");
    } else {
        buffer[ret] = '\0';  // 确保字符串以null结尾
        printk(KERN_INFO "Read data: %s\n", buffer);
    }
    
    // 关闭文件
    filp_close(file, NULL);
    return 0;
}

static void __exit reader_exit(void)
{
    printk(KERN_INFO "Reader module unloaded\n");
}

module_init(reader_init);
module_exit(reader_exit);
MODULE_LICENSE("GPL");

