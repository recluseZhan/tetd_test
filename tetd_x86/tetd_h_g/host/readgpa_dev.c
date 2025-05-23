#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
#define DEV_ID 232
#define DEVNAME "readgpa_dev"

extern void read_func(void);
extern void write_func(uint64_t gva);

// open dev
static int readgpa_dev_open(struct inode *inode, struct file *filp) 
{   
    return 0; 
} 
// release dev
static int readgpa_dev_release(struct inode *inode, struct file *filp) 
{ 
    return 0; 
} 
// read dev
static ssize_t readgpa_dev_read(struct file *filp, char __user *buf, size_t size, loff_t *offset) 
{
    read_func();
    return 0;
} 
// write dev
static ssize_t readgpa_dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset) 
{  
    uint64_t write_buffer[1];
    uint64_t gva;
    copy_from_user(write_buffer, buf, size);
    gva = write_buffer[0];
    write_func(gva);
    return 0; 
} 

static struct file_operations fops = {
    .owner = THIS_MODULE, 
    .open = readgpa_dev_open, 
    .release= readgpa_dev_release, 
    .read = readgpa_dev_read, 
    .write = readgpa_dev_write, 
}; 

static int __init readgpa_dev_init(void) 
{   
    int ret;
    ret = register_chrdev(DEV_ID,DEVNAME,&fops);
 
    if (ret < 0) {
        printk(KERN_EMERG DEVNAME "can't register major number.\n");
        return ret;
    }
    return 0;
}

static void __exit readgpa_dev_exit(void)
{
    unregister_chrdev(DEV_ID,DEVNAME);
}

module_init(readgpa_dev_init);
module_exit(readgpa_dev_exit);


