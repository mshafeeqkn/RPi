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
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robert W. Oliver II");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

static int dev_tree_probe(struct platform_device *pdev);
static int dev_tree_remove(struct platform_device *pdev);

static struct of_device_id my_dev_ids[] = {
    {
        .compatible = "mydevice,sample",
    },
    {}
};
MODULE_DEVICE_TABLE(of, my_dev_ids);

static struct platform_driver my_driver = {
    .probe = dev_tree_probe,
    .remove = dev_tree_remove,
    .driver = {
        .name = "shafeeque_driver",
        .of_match_table = my_dev_ids,
    },
};

static int dev_tree_probe(struct platform_device *pdev)
{
    int ret;
    const char *label;
    struct device *dev = &pdev->dev;

    pr_info("Inside dev_tree_probe\n");

    if(device_property_present(dev, "label") == 0) {
        pr_err("Label property not found in the .dts file\n");
        return -1;
    }

    ret = device_property_read_string(dev, "label", &label);
    if(ret != 0) {
        pr_err("Unable to read the label data\n");
        return -1;
    }

    pr_info("YEZZZZZ.. The data read from DTS: %s\n", label);

    return 0;
}

static int dev_tree_remove(struct platform_device *pdev)
{
    pr_info("Inside dev_tree_remove\n");
    return 0;
}

static int __init dev_tree_start(void)
{
    pr_info("dev_tree_start: starting the kernel driver\n");
    if(platform_driver_register(&my_driver) != 0) {
        pr_err("dev_tree_start: Could not load the driver\n");
        return -1;
    }
    pr_info("dev_tree_start: Loaded into the kernel\n");
    return 0;
}

static void __exit dev_tree_end(void)
{
    platform_driver_unregister(&my_driver);
    pr_info("dev_tree_probe: Goodbye Mr.\n");
}

module_init(dev_tree_start);
module_exit(dev_tree_end);
