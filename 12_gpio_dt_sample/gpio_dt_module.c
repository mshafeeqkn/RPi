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
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robert W. Oliver II");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

static struct gpio_desc *gpio4;

static int gpio_dt_remove(struct platform_device *pdev);
static int gpio_dt_probe(struct platform_device *pdev);

static struct of_device_id gpio_dev_ids[] = {
    {
        .compatible = "gpio4,customization",
    },
    {}
};
MODULE_DEVICE_TABLE(of, gpio_dev_ids);

static struct platform_driver gpio_driver = {
    .probe = gpio_dt_probe,
    .remove = gpio_dt_remove,
    .driver = {
        .name = "shafeeque_driver",
        .of_match_table = gpio_dev_ids,
    },
};

static int gpio_dt_probe(struct platform_device *pdev)
{

    pr_info("Inside dev_tree_probe\n");
    gpio4 = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_HIGH);
    if (IS_ERR(gpio4)) {
        dev_err(&pdev->dev, "Failed to get GPIO 4\n");
        return PTR_ERR(gpio4);
    }

    gpiod_set_value(gpio4, 1);
    return 0;
}

static int gpio_dt_remove(struct platform_device *pdev)
{
    pr_info("Inside dev_tree_remove\n");
    gpiod_set_value(gpio4, 0);
    return 0;
}

module_platform_driver(gpio_driver);
