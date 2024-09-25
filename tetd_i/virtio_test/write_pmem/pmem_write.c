#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>

#define FILE_PATH "/mnt/sharedfile"
#define DATA_TO_WRITE "Hello from Writer VM!\n"

static int __init writer_init(void)
{
    struct file *file;
    loff_t pos = 0;
    ssize_t ret;
    
    // 打开文件
    file = filp_open(FILE_PATH, O_WRONLY | O_CREAT, 0644);
    if (IS_ERR(file)) {
        printk(KERN_ERR "Failed to open file for writing\n");
        return PTR_ERR(file);
    }
    
    // 写入数据
    ret = kernel_write(file, DATA_TO_WRITE, strlen(DATA_TO_WRITE), &pos);
    if (ret < 0) {
        printk(KERN_ERR "Failed to write to file\n");
    } else {
        printk(KERN_INFO "Data written to file successfully\n");
    }
    
    //file->f_op->flush(file,current->files);    
    // 关闭文件
    filp_close(file, NULL);
    return 0;
}

static void __exit writer_exit(void)
{
    printk(KERN_INFO "Writer module unloaded\n");
}

module_init(writer_init);
module_exit(writer_exit);
MODULE_LICENSE("GPL");

