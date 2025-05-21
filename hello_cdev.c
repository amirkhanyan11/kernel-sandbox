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
static uint32_t data_size = 0;

static int my_release(struct inode *inode, struct file *file) {
  printk(KERN_INFO "Called release");
  return 0;
}

static int my_open(struct inode *inode, struct file *file) {
  data_size = 0;
  file->f_pos = 0;
  printk(KERN_INFO "Called open");
  return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
  if (*ppos >= data_size)
    return 0;
  if (count > data_size - *ppos)
    count = data_size - *ppos;

  if (copy_to_user(buf, kernel_buffer + *ppos, count))
    return -1;
  *ppos += count;
  return count;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count,
                        loff_t *ppos) {
  if (*ppos >= BUFFER_SIZE)
    return -ENOMEM;

  if (count > BUFFER_SIZE - *ppos)
    count = BUFFER_SIZE - *ppos;

  if (copy_from_user(kernel_buffer + *ppos, buf, count))
    return -1;
  *ppos += count;
  data_size = *ppos;
  return count;
}

static struct file_operations fops = {.owner = THIS_MODULE,
                                      .open = my_open,
                                      .release = my_release,
                                      .read = my_read,
                                      .write = my_write};

static int __init hello_mod_init(void) {
  major = register_chrdev(0, DEVICE_NAME, &fops);

  kernel_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
  memset(kernel_buffer, 0, BUFFER_SIZE);
  if (NULL == kernel_buffer) {
    printk(KERN_ERR "bad alloc :(");
    return -1;
  }

  if (major < 0) {
    printk("Error while registering char device");
    return major;
  }
  printk(KERN_INFO "char device registered successfully with major %d", major);
  return 0;
}

static void __exit hello_mod_exit(void) {
  unregister_chrdev(major, DEVICE_NAME);
  kfree(kernel_buffer);
  printk(KERN_INFO "Exiting chrdev module from kernel !!!\n");
}

module_init(hello_mod_init);
module_exit(hello_mod_exit);
