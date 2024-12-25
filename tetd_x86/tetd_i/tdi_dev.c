#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/io.h>
MODULE_LICENSE("GPL");
#define DEV_ID 233
#define DEVNAME "tdi_dev"

extern void tdi_run(u8 *data, uint64_t data_size);
extern uint64_t read_func(void);

static unsigned long gva_page;

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
    /*unsigned long gva, gpa_0;
    gva = __get_free_page(GFP_KERNEL);
    if (!gva) {
        pr_err("Failed to allocate a 4KB page\n");
        return -ENOMEM;
    }
    pr_info("Allocated physical page at 0x%lx\n", gva);
   
    char *page_ptr = (char *)gva;

    snprintf(page_ptr, PAGE_SIZE, "Hello, this is some data in the allocated page!\n");

    printk(KERN_INFO "Data in page: %s\n", page_ptr);

    gpa_0 = virt_to_phys((void*)gva);
    printk(KERN_INFO "gpa=%lx\n",gpa_0);*/
    uint64_t gva;
    gva = read_func();
    return gva;
} 

// write dev
static ssize_t tdi_dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset) 
{  
    uint64_t write_buffer[2];
    uint64_t gva;
    uint64_t data_size;
    u8 *data;

    copy_from_user(write_buffer, buf, size);
    gva = write_buffer[0];
    data_size = write_buffer[1];
    
    data = (u8 *)gva_page;
    memset(data,30,data_size);
    //printk("%u\n",*data);

    tdi_run(data, data_size);
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

    
    gva_page = get_zeroed_page(GFP_KERNEL);
    if(!gva_page){
        pr_err("failed to allocate 4kb page\n");
	return -ENOMEM;
    }
    printk(KERN_INFO "successfully allocated 4kb page 0x%lx\n", gva_page);
    
    return 0;
}

static void __exit tdi_dev_exit(void)
{
    if(gva_page){
        free_page(gva_page);
        printk(KERN_INFO "4kb page 0x%lx has been freed\n", gva_page);
    }
    unregister_chrdev(DEV_ID,DEVNAME);
}

module_init(tdi_dev_init);
module_exit(tdi_dev_exit);


