#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define SHM_SIZE 1024  // 定义共享内存大小

static char *shared_mem = NULL;  // 指向共享内存
static int major_number;

// ioctl 命令
#define WR_SHM _IOW('a', 'a', char *)

static long shm_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case WR_SHM:
            // 将用户传入的数据拷贝到共享内存
            if (copy_from_user(shared_mem, (char *)arg, SHM_SIZE)) {
                printk(KERN_ERR "Failed to write data to shared memory\n");
                return -EFAULT;
            }
            printk(KERN_INFO "Data written to shared memory: %s\n", shared_mem);
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .unlocked_ioctl = shm_ioctl,
};

static int __init shm_writer_init(void) {
    // 分配共享内存
    shared_mem = vmalloc(SHM_SIZE);
    if (!shared_mem) {
        printk(KERN_ERR "Failed to allocate shared memory\n");
        return -ENOMEM;
    }

    // 注册字符设备
    major_number = register_chrdev(0, "shm_writer", &fops);
    if (major_number < 0) {
        vfree(shared_mem);
        printk(KERN_ERR "Failed to register character device\n");
        return major_number;
    }

    printk(KERN_INFO "Shared memory module loaded, major number: %d\n", major_number);
    return 0;
}

static void __exit shm_writer_exit(void) {
    unregister_chrdev(major_number, "shm_writer");
    vfree(shared_mem);
    printk(KERN_INFO "Shared memory module unloaded\n");
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Kernel module for writing to shared memory");

