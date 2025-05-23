#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");
#define DEV_ID 232
#define DEVNAME "readgpa_dev"

static unsigned long gva_0,gpa_0;

extern void read_func(void);
extern void write_func(uint64_t gva);
void bind_pa(unsigned long gpa, unsigned long hpa);
void target_dump(unsigned long hpa, unsigned long gpa, unsigned long target_gva, unsigned long flag);
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
    unsigned long read_buffer[4];
    unsigned long hpa, gpa, target_gva, flag;

    copy_from_user(read_buffer, buf, size);
    hpa = read_buffer[0];
    //gpa = read_buffer[1];
    gpa = gpa_0;
    target_gva = read_buffer[2];
    flag = read_buffer[3];

    target_dump(hpa, gpa, target_gva, flag);
    u8 *gva_ptr = (u8*)gva_0;
    u8 gva_value = *gva_ptr;  
    printk(KERN_INFO "Value at GVA: %u\n", gva_value);
    //read_func();
    return 0;
} 
// write dev
static ssize_t readgpa_dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset) 
{  
    unsigned long write_buffer[1];
    unsigned long gva;
    unsigned long gpa;
    copy_from_user(write_buffer, buf, size);
    gva = write_buffer[0];
    gpa = virt_to_phys((void*)gva);
    
    
    //write_func(gva);
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
    gva_0 = __get_free_page(GFP_KERNEL);
    if (!gva_0) {
        pr_err("Failed to allocate a 4KB page\n");
        return -ENOMEM;
    }
    pr_info("Allocated gva at 0x%lx\n", gva_0);
    memset((void*)gva_0,0,PAGE_SIZE);
    gpa_0 = virt_to_phys((void*)gva_0);
    pr_info("Allocated at gva 0x%lx,gpa 0x%lx\n", gva_0,gpa_0);

    return 0;
}

static void __exit readgpa_dev_exit(void)
{
    free_page(gva_0);
    unregister_chrdev(DEV_ID,DEVNAME);
}

module_init(readgpa_dev_init);
module_exit(readgpa_dev_exit);


