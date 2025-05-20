// memchardev.c - simple in-memory character device
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DEVICE_NAME "memchardev"
#define BUFFER_SIZE 1024

static dev_t dev_number;
static struct class *memchardev_class = NULL;
static struct cdev memchardev_cdev;
static char *kernel_buffer = NULL;
static int device_open_count = 0;
static size_t data_size = 0;

/* File open: allow only single access (non-concurrent) */
static int memchardev_open(struct inode *inode, struct file *file) {
    if (device_open_count > 0)
        return -EBUSY;  // another open in progress
    device_open_count++;
    data_size = 0;
    file->f_pos = 0;
    printk(KERN_INFO "memchardev: device opened\n");
    return 0;
}

/* File release (close) */
static int memchardev_release(struct inode *inode, struct file *file) {
    device_open_count--;
    printk(KERN_INFO "memchardev: device closed\n");
    return 0;
}

/* File read: copy data from kernel buffer to user space */
static ssize_t memchardev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    ssize_t retval = 0;
    /* No data to read beyond written data */
    if (*ppos >= data_size)
        return 0;  // EOF
    if (count > data_size - *ppos)
        count = data_size - *ppos;
    /* Copy to user and check for errors */
    if (copy_to_user(buf, kernel_buffer + *ppos, count))
        return -EFAULT;
    *ppos += count;
    retval = count;
    return retval;
}

/* File write: copy data from user space to kernel buffer */
static ssize_t memchardev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    ssize_t retval = -ENOMEM;
    if (*ppos >= BUFFER_SIZE)
        return -ENOMEM;  // no space left
    if (count > BUFFER_SIZE - *ppos)
        count = BUFFER_SIZE - *ppos;
    /* Copy from user and check for errors */
    if (copy_from_user(kernel_buffer + *ppos, buf, count))
        return -EFAULT;
    *ppos += count;
    data_size = *ppos;
    retval = count;
    return retval;
}

/* File operations structure */
static const struct file_operations memchardev_fops = {
    .owner   = THIS_MODULE,
    .open    = memchardev_open,
    .read    = memchardev_read,
    .write   = memchardev_write,
    .release = memchardev_release,
};

/* Module initialization */
static int __init memchardev_init(void) {
    int ret;
    printk(KERN_INFO "memchardev: initializing module\n");
    /* Allocate kernel memory for the buffer */
    kernel_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!kernel_buffer) {
        printk(KERN_ERR "memchardev: failed to allocate memory\n");
        return -ENOMEM;
    }
    memset(kernel_buffer, 0, BUFFER_SIZE);

    /* Dynamically allocate a major number */
    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "memchardev: alloc_chrdev_region failed\n");
        kfree(kernel_buffer);
        return ret;
    }
    printk(KERN_INFO "memchardev: major number %d\n", MAJOR(dev_number));

    /* Initialize and add the character device */
    cdev_init(&memchardev_cdev, &memchardev_fops);
    memchardev_cdev.owner = THIS_MODULE;
    ret = cdev_add(&memchardev_cdev, dev_number, 1);
    if (ret) {
        printk(KERN_ERR "memchardev: cdev_add failed\n");
        unregister_chrdev_region(dev_number, 1);
        kfree(kernel_buffer);
        return ret;
    }

    /* Create device class and device node (e.g., /dev/memchardev) */
    memchardev_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(memchardev_class)) {
        printk(KERN_ERR "memchardev: class_create failed\n");
        cdev_del(&memchardev_cdev);
        unregister_chrdev_region(dev_number, 1);
        kfree(kernel_buffer);
        return PTR_ERR(memchardev_class);
    }
    if (device_create(memchardev_class, NULL, dev_number, NULL, DEVICE_NAME) == NULL) {
        printk(KERN_ERR "memchardev: device_create failed\n");
        class_destroy(memchardev_class);
        cdev_del(&memchardev_cdev);
        unregister_chrdev_region(dev_number, 1);
        kfree(kernel_buffer);
        return -1;
    }

    return 0;
}

/* Module cleanup */
static void __exit memchardev_exit(void) {
    dev_t dev = dev_number;
    device_destroy(memchardev_class, dev);
    class_destroy(memchardev_class);
    cdev_del(&memchardev_cdev);
    unregister_chrdev_region(dev, 1);
    kfree(kernel_buffer);
    printk(KERN_INFO "memchardev: module unloaded\n");
}

module_init(memchardev_init);
module_exit(memchardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Simple in-memory character device");




//
// // test_app.c - user-space test for memchardev
// #include <stdio.h>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <string.h>
// #include <errno.h>
//
// #define DEVICE_FILE "/dev/memchardev"
// #define BUFFER_SIZE 1024
//
// int main() {
//     int fd = open(DEVICE_FILE, O_RDWR);
//     if (fd < 0) {
//         perror("open");
//         return 1;
//     }
//
//     const char *msg = "Hello from user space!";
//     ssize_t written = write(fd, msg, strlen(msg));
//     if (written < 0) {
//         perror("write");
//         close(fd);
//         return 1;
//     }
//     printf("Wrote %zd bytes to %s\n", written, DEVICE_FILE);
//
//     /* Reset file offset to beginning */
//     if (lseek(fd, 0, SEEK_SET) < 0) {
//         perror("lseek");
//         close(fd);
//         return 1;
//     }
//
//     char buffer[BUFFER_SIZE] = {0};
//     ssize_t read_bytes = read(fd, buffer, BUFFER_SIZE);
//     if (read_bytes < 0) {
//         perror("read");
//         close(fd);
//         return 1;
//     }
//     printf("Read %zd bytes from %s: '%s'\n", read_bytes, DEVICE_FILE, buffer);
//
//     close(fd);
//     return 0;
// }
//
