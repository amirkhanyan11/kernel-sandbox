#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>


MODULE_LICENSE("GPL");

static int custom_init(void) {
  printk(KERN_INFO "Hello world!");
  return 0;
}


static void custom_exit(void) {
  printk(KERN_INFO "current process is %s (pid: %i)", current->comm, current->pid);
  printk(KERN_INFO "Goodbye cruel world.....");
}


module_init(custom_init);
module_exit(custom_exit);
