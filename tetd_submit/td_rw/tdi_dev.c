#include <linux/module.h>
MODULE_LICENSE("GPL");

#define DEV_ID 233
#define DEVNAME "tdi_dev"

extern void write_kernel_to_buffer(void);
extern int read_from_buffer(void);

// open dev
static int tdi_dev_open(struct inode *inode, struct file *filp) 
{   
    return 0; 
} 
// release dev
static int tdi_dev_release(struct inode *inode, struct file *filp) 
{ 
    return 0; 
} 
// read dev
static ssize_t tdi_dev_read(struct file *filp, char __user *buf, size_t size, loff_t *offset) 
{    
    read_from_buffer();
    return 0;
} 

// write dev
static ssize_t tdi_dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset) 
{  
    write_kernel_to_buffer();
    return 0; 
} 

static struct file_operations fops = {
    .owner = THIS_MODULE, 
    .open = tdi_dev_open, 
    .release= tdi_dev_release, 
    .read = tdi_dev_read, 
    .write = tdi_dev_write, 
}; 

static int __init tdi_dev_init(void) 
{   
    int ret;
    ret = register_chrdev(DEV_ID,DEVNAME,&fops);
    if (ret < 0) {
        printk(KERN_EMERG DEVNAME "can't register major number.\n");
        return ret;
    }
    
    return 0;
}

static void __exit tdi_dev_exit(void)
{
    unregister_chrdev(DEV_ID,DEVNAME);
}

module_init(tdi_dev_init);
module_exit(tdi_dev_exit);
