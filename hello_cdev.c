#include "linux/fs.h"
#include "linux/gfp_types.h"
#include "linux/init.h"
#include "linux/kern_levels.h"
#include "linux/printk.h"
#include "linux/slab.h"
#include "linux/types.h"
#include "linux/uaccess.h"
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

#define DEVICE_NAME "hello_cdev"
#define BUFFER_SIZE 2048

static int major = 0;
uint8_t *kernel_buffer = NULL;


static int my_release(struct inode *inode, struct file *file) {
  kfree(kernel_buffer);
  printk(KERN_INFO "Called release, successfully deallocated memory of the kernel buffer");
  return 0;
}

static int my_open(struct inode *inode, struct file *file) {
  kernel_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
  if (NULL == kernel_buffer) {
    printk(KERN_ERR "bad alloc :(");
    return -1;
  }
  printk(KERN_INFO "Called open, successfully allocated %d bytes for the kernel buffer", BUFFER_SIZE);
  return 0;
}

static ssize_t my_write(struct file *file, const char __user *buffer, size_t n, loff_t *l) {
  if (0 != copy_from_user(kernel_buffer, buffer, n)) {
    printk(KERN_ERR "Error writing data to kernel buffer");
    return -1;
  }
  printk(KERN_INFO "Called write, successfully wrote %zu bytes", n);
  return 0;
}

static ssize_t my_read(struct file *file, char __user *buffer, size_t n, loff_t *l) {
  if (0 != copy_to_user(buffer, kernel_buffer, n)) {
    printk(KERN_ERR "Error fetching data from kernel buffer");
    return -1;
  }
  printk("Called read, successfully read %zu bytes", n);
  return 0;
}

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = my_open,
  .release = my_release,
  .read = my_read,
  .write = my_write
};

static int __init hello_mod_init(void) {
  major = register_chrdev(0, DEVICE_NAME, &fops);
  if (major < 0) {
    printk("Error while registering char device");
    return major;
  }
  printk(KERN_INFO "char device registered successfully with major %d", major);
  return 0;
}

static void __exit hello_mod_exit(void) {
  unregister_chrdev(major, DEVICE_NAME);
  printk(KERN_INFO "Exiting chrdev module from kernel !!!\n");
}

module_init(hello_mod_init);
module_exit(hello_mod_exit);
