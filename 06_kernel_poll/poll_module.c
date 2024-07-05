#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/err.h>

#define FIRST_MINOR             0
#define NUM_MINOR               1
#define MAX_BUF_LEN             128
#define DEV_DRIVER_NAME         "poll_dev"
#define DEV_DRIVER_CLASS        "poll_class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robert W. Oliver II");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

typedef unsigned int uint;

static int  poll_open(struct inode *inode, struct file *file);
static int  poll_release(struct inode *inode, struct file *file);
static uint poll_poll(struct file *filp, struct poll_table_struct *wait);
static ssize_t poll_read(struct file *f, char __user *buf,
                                    size_t len, loff_t *off);
static ssize_t poll_write(struct file *f, const char __user *buf,
                                    size_t len, loff_t *off);

typedef enum {
    CL_CDEV_ADD,
    CL_CLASS_CREATE,
    CL_DEVICE_CREATE,
    CL_COMPLETE
} cleanup_point_t;

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = poll_open,
    .release = poll_release,
    .poll    = poll_poll,
    .write   = poll_write,
    .read    = poll_read
};

static dev_t             dev         = 0;
static struct cdev       poll_cdev;
static struct class     *dev_class;
static struct device    *dev_device;
static char              buffer[MAX_BUF_LEN];
static int               read_pending = false;
#if 0
static int               write_ready = false;
#endif

DECLARE_WAIT_QUEUE_HEAD(wait_queue);

static int  poll_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}

static int  poll_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}

static uint poll_poll(struct file *filp, struct poll_table_struct *wait)
{
    __poll_t mask = 0;

    poll_wait(filp, &wait_queue, wait);
    pr_info("POLL function called\n");

    if(read_pending) {
        pr_info("Set read poll flag in kernel to FALSE\n");
        read_pending = false;
        mask |= (POLLIN | POLLRDNORM);
    }
#if 0
    if(write_ready) {
        write_ready = false;
        mask |= (POLLOUT | POLLWRNORM);
    }
#endif
    return mask;
}

static ssize_t poll_read(struct file *f, char __user *buf,
                                    size_t len, loff_t *off)
{
    size_t data_len = strlen(buffer);
    int pending = copy_to_user(buf, buffer, data_len);
    pr_info("calling read len: %lu, offset: %llu\n", len, *off);
    memset(buffer, 0, sizeof(buffer));
    return data_len - pending;
}

static ssize_t poll_write(struct file *f, const char __user *buf,
                                    size_t len, loff_t *off)
{
    int pending;
    if(len >= MAX_BUF_LEN) return 0;
    pending = copy_from_user(buffer, buf, len);
    *off += (len - pending);
    read_pending = true;
    wake_up(&wait_queue);
    return (len - pending);
}

static void cleanup_driver(cleanup_point_t cl)
{
    switch(cl) {
        case CL_COMPLETE:
            device_destroy(dev_class, dev);

        __attribute__((__fallthrough__));
        case CL_DEVICE_CREATE:
            class_destroy(dev_class);

        __attribute__((__fallthrough__));
        case CL_CLASS_CREATE:
            cdev_del(&poll_cdev);

        __attribute__((__fallthrough__));
        case CL_CDEV_ADD:
            unregister_chrdev_region(dev, 1);
    }
}

static int init_driver(void)
{
    if(alloc_chrdev_region(&dev, FIRST_MINOR, NUM_MINOR, "poll_dev") < 0) {
        pr_err("Allocating major number failed\n");
        return -1;
    }

    pr_info("Device number allocated - Major: %d; Minor: %d\n",
            MAJOR(dev), MINOR(dev));

    cdev_init(&poll_cdev, &fops);
    poll_cdev.owner = THIS_MODULE;
    if((cdev_add(&poll_cdev, dev, NUM_MINOR)) < 0) {
        pr_err("Failed to add the device\n");
        cleanup_driver(CL_CDEV_ADD);
        return -1;
    }

    dev_class = class_create(THIS_MODULE, DEV_DRIVER_CLASS);
    if(IS_ERR(dev_class)) {
        pr_err("Dev class create failed\n");
        cleanup_driver(CL_CLASS_CREATE);
        return -1;
    }

    dev_device = device_create(dev_class, NULL, dev, NULL, DEV_DRIVER_NAME);
    if(IS_ERR(dev_device)) {
        pr_err("Device create failed\n");
        cleanup_driver(CL_DEVICE_CREATE);
        return -1;
    }

    return 0;
}

static int __init hello_start(void)
{
    init_driver();
    printk(KERN_INFO "Hello world\n");
    return 0;
}

static void __exit hello_end(void)
{
    cleanup_driver(CL_COMPLETE);
    printk(KERN_INFO "Goodbye Mr.\n");
}

module_init(hello_start);
module_exit(hello_end);
