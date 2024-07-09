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
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robert W. Oliver II");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

static struct kobject *sysfs_kobj;

static ssize_t sysfs_show(struct kobject *kobj,
                struct kobj_attribute *attr, char *buffer)
{
    return sprintf(buffer, "sysfs_read: /sys/kernel/%s/%s\n",
                        kobj->name, attr->attr.name);
}

static ssize_t sysfs_store(struct kobject *kobj,
            struct kobj_attribute *attr,
            const char *buffer, size_t count)
{
    pr_info("sysfs_read: you wrote: %s into /sys/kernel/%s/%s\n",
                buffer, kobj->name, attr->attr.name);
    return count;
}

static struct kobj_attribute sysfs_attr = __ATTR(shafeeque,
                                    0644, sysfs_show, sysfs_store);

static int __init sysfs_start(void)
{
    sysfs_kobj = kobject_create_and_add("sysfs", kernel_kobj);
    if(!sysfs_kobj) {
        pr_err("Error on creating /sys/kernel/hello/dummy\n");
        return -ENOMEM;
    }

    if(sysfs_create_file(sysfs_kobj, &sysfs_attr.attr)) {
        pr_err("Error on creating sysfs file\n");
        kobject_put(sysfs_kobj);
        return -ENOMEM;
    }

    pr_info("sysfs_module: Loaded into the kernel\n");
    return 0;
}

static void __exit sysfs_end(void)
{
    sysfs_remove_file(sysfs_kobj, &sysfs_attr.attr);
    kobject_put(sysfs_kobj);
    pr_info("sysfs_end: Goodbye Mr.\n");
}

module_init(sysfs_start);
module_exit(sysfs_end);
