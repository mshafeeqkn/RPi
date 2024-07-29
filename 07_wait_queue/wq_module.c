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
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/err.h>
#include <linux/delay.h>

#define FIRST_MINOR             0
#define NUM_MINOR               1
#define DEV_DRIVER_NAME         "wq_dev"
#define DEV_DRIVER_CLASS        "wq_class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robert W. Oliver II");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

typedef unsigned int uint;

static int  wq_open(struct inode *inode, struct file *file);
static int  wq_release(struct inode *inode, struct file *file);
static ssize_t wq_write(struct file *f, const char __user *buf,
                                    size_t len, loff_t *off);

typedef enum {
    CL_CDEV_ADD,
    CL_CLASS_CREATE,
    CL_DEVICE_CREATE,
    CL_COMPLETE,
    CL_THREAD,
} cleanup_point_t;

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = wq_open,
    .release = wq_release,
    .write   = wq_write,
};

static dev_t             dev         = 0;
static struct cdev       wq_cdev;
static struct class     *dev_class;
static struct device    *dev_device;
static int               thread_var  = 0;

// Static declaration of wait queue
DECLARE_WAIT_QUEUE_HEAD(wait_queue_10);

// Dynamic declaration of wait queue
static wait_queue_head_t wait_queue_20;


static int  wq_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}

static int  wq_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}

static int thread_function(void *arg) {
    uint8_t data = *(uint8_t *)arg;
    pr_info("Creating thread: %d\n", data);

    if(data == 1) {
        wait_event(wait_queue_10, thread_var == 10);
    } else if(data == 2) {
        wait_event(wait_queue_20, thread_var == 20);
    }

    pr_info("thread_function: %d execution finished\n", data);
    return 0;
}

static int start_kernel_threads(void) {
    struct task_struct *kthread_1, *kthread_2;
    uint8_t t1 = 1, t2 = 2;

    kthread_1 = kthread_run(thread_function, &t1, "thread_num_1");
    if(kthread_1 == NULL) {
        pr_err("Thread 1 create failed\n");
        return -1;
    }

    kthread_2 = kthread_run(thread_function, &t2, "thread_num_2");
    if(kthread_2 == NULL) {
        pr_err("Thread 2 create failed\n");
        thread_var = 10;
        wake_up(&wait_queue_10);
        return -1;
    }

    msleep(100);
    return 0;
}

static ssize_t wq_write(struct file *f, const char __user *buf,
                                    size_t len, loff_t *off)
{
    thread_var = simple_strtol(buf, NULL, 10);

    wake_up(&wait_queue_10);
    wake_up(&wait_queue_20);

    return len;
}

static void cleanup_driver(cleanup_point_t cl)
{
    switch(cl) {
        case CL_THREAD:
            thread_var = 10;
            wake_up(&wait_queue_10);
            thread_var = 20;
            wake_up(&wait_queue_20);

        __attribute__((__fallthrough__));
        case CL_COMPLETE:
            device_destroy(dev_class, dev);

        __attribute__((__fallthrough__));
        case CL_DEVICE_CREATE:
            class_destroy(dev_class);

        __attribute__((__fallthrough__));
        case CL_CLASS_CREATE:
            cdev_del(&wq_cdev);

        __attribute__((__fallthrough__));
        case CL_CDEV_ADD:
            unregister_chrdev_region(dev, 1);
    }
}

static int init_driver(void)
{
    if(alloc_chrdev_region(&dev, FIRST_MINOR, NUM_MINOR, "wq_dev") < 0) {
        pr_err("Allocating major number failed\n");
        return -1;
    }

    pr_info("Device number allocated - Major: %d; Minor: %d\n",
            MAJOR(dev), MINOR(dev));

    cdev_init(&wq_cdev, &fops);
    wq_cdev.owner = THIS_MODULE;
    if((cdev_add(&wq_cdev, dev, NUM_MINOR)) < 0) {
        pr_err("Failed to add the device\n");
        cleanup_driver(CL_CDEV_ADD);
        return -1;
    }

    dev_class = class_create(DEV_DRIVER_CLASS);
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

    init_waitqueue_head(&wait_queue_20);
    if(-1 == start_kernel_threads()) {
        cleanup_driver(CL_COMPLETE);
        return -1;
    }

    return 0;
}

static int __init hello_start(void)
{
    int ret = init_driver();
    if(ret == 0) {
        printk(KERN_INFO "wq_module Loaded\n");
    }
    return ret;
}

static void __exit hello_end(void)
{
    cleanup_driver(CL_THREAD);
    printk(KERN_INFO "Goodbye Mr.\n");
}

module_init(hello_start);
module_exit(hello_end);
