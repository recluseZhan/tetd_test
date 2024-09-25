#include <asm/processor.h>
#include <asm/alternative.h>
#include <linux/interrupt.h>
#include <uapi/asm/kvm_para.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/current.h>
#include <linux/sched.h>

#include <asm/uaccess.h>
#include <linux/ctype.h>
#include <linux/smp.h>

#include <asm/segment.h>
#include <linux/buffer_head.h>
#include <asm/processor.h>
#include <linux/sched.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define DEV_ID 233
#define DEVNAME "log_dev" 

__attribute__((aligned(4096))) uint8_t log_store[4096] = {0x0};

unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}


//
// open dev
static int log_dev_open(struct inode *inode, struct file *filp) 
{   
    return 0; 
} 
// release dev
static int log_dev_release(struct inode *inode, struct file *filp) 
{ 
    return 0; 
} 
// read dev
static ssize_t log_dev_read(struct file *filp, char __user *buf, size_t size, loff_t *offset) 
{
    copy_from_user(log_store,buf,size);
    printk("log_store=%lx\n",log_store);  
    printk("log_store=%s\n",log_store);   
    return 0;
} 
// write dev
static ssize_t log_dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset) 
{   
    
    return 0; 
} 

static struct file_operations fops = {
    .owner = THIS_MODULE, 
    .open = log_dev_open, 
    .release= log_dev_release, 
    .read = log_dev_read, 
    .write = log_dev_write, 
}; 

static int __init log_dev_init(void) 
{   
    int ret;
    
    ret = register_chrdev(DEV_ID,DEVNAME,&fops);
 
    if (ret < 0) {
        printk(KERN_EMERG DEVNAME "can't register major number.\n");
        return ret;
    }
    return 0;
}

static void __exit log_dev_exit(void)
{
    unregister_chrdev(DEV_ID,DEVNAME);
}

module_init(log_dev_init);
module_exit(log_dev_exit);
