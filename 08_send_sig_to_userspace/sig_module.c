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
#include <linux/sched.h>

#define FIRST_MINOR             0
#define NUM_MINOR               1
#define DEV_DRIVER_NAME         "sig_dev"
#define DEV_DRIVER_CLASS        "sig_class"

#define REGISTER_APP            _IO('R', 'g')
#define SIG_NUM                 44

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robert W. Oliver II");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

typedef unsigned int uint;

static int  sig_open(struct inode *inode, struct file *file);
static int  sig_release(struct inode *inode, struct file *file);
static ssize_t sig_write(struct file *f, const char __user *buf,
                                    size_t len, loff_t *off);
static long int sig_ioctl(struct file *file, unsigned int cmd,
                               unsigned long arg);

typedef enum {
    CL_CDEV_ADD,
    CL_CLASS_CREATE,
    CL_DEVICE_CREATE,
    CL_COMPLETE,
} cleanup_point_t;

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = sig_open,
    .release = sig_release,
    .write   = sig_write,
    .unlocked_ioctl   = sig_ioctl,
};

static dev_t             dev         = 0;
static struct cdev       sig_cdev;
static struct class     *dev_class;
static struct device    *dev_device;
static struct task_struct *task      = NULL;

static int  sig_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}

static int  sig_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}

static ssize_t sig_write(struct file *f, const char __user *buf,
                                    size_t len, loff_t *off)
{
    struct siginfo info;
    int module_var;

    module_var = simple_strtol(buf, NULL, 10);
    if(module_var == 44) {
        if(task != NULL) {
            memset(&info, 0, sizeof(info));
            info.si_signo = SIG_NUM;
            info.si_code = SI_QUEUE;

            if(send_sig_info(SIG_NUM, (struct kernel_siginfo *)&info, task) < 0) {
                pr_err("Failed to send the signal\n");
            } else {
                pr_info("Singal sent to user application\n");
            }
        } else {
            pr_err("User application is not running\n");
        }
    }
    return len;
}

static long int sig_ioctl(struct file *file, unsigned int cmd,
                               unsigned long arg)
{
    if(cmd == REGISTER_APP) {
        task = get_current();
        pr_info("Userspace application registered: PID: %d\n", task->pid);
    }
    return 0;
}

static void cleanup_driver(cleanup_point_t cl)
{
    switch(cl) {
        case CL_COMPLETE:
            device_destroy(dev_class, dev);
            task = NULL;

        __attribute__((__fallthrough__));
        case CL_DEVICE_CREATE:
            class_destroy(dev_class);

        __attribute__((__fallthrough__));
        case CL_CLASS_CREATE:
            cdev_del(&sig_cdev);

        __attribute__((__fallthrough__));
        case CL_CDEV_ADD:
            unregister_chrdev_region(dev, 1);
    }
}

static int init_driver(void)
{
    if(alloc_chrdev_region(&dev, FIRST_MINOR, NUM_MINOR, "sig_dev") < 0) {
        pr_err("Allocating major number failed\n");
        return -1;
    }

    pr_info("Device number allocated - Major: %d; Minor: %d\n",
            MAJOR(dev), MINOR(dev));

    cdev_init(&sig_cdev, &fops);
    sig_cdev.owner = THIS_MODULE;
    if((cdev_add(&sig_cdev, dev, NUM_MINOR)) < 0) {
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

static int __init sig_start(void)
{
    int ret = init_driver();
    if(ret == 0) {
        pr_info("sig_module Loaded\n");
    }
    return ret;
}

static void __exit sig_end(void)
{
    cleanup_driver(CL_COMPLETE);
    pr_info("Goodbye Mr.\n");
}

module_init(sig_start);
module_exit(sig_end);
