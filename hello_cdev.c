#include "linux/fs.h"
#include "linux/printk.h"
#include "linux/types.h"
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mickael Jackson");
MODULE_DESCRIPTION("Character driver");
MODULE_VERSION("1.4.88");

#define DEVICE_NAME "hello_cdev"

static int major = 0;

ssize_t my_read(struct file *file, char __user *user, size_t s, loff_t *l) {
  printk("Called my_read");
  return 0;
}

static struct file_operations fops = {
  .read = my_read
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
  printk(KERN_INFO "Exiting hello world module from kernel !!!\n");
}

module_init(hello_mod_init);
module_exit(hello_mod_exit);
