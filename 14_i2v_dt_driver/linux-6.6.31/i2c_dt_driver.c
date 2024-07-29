// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define I2C_SLAVE_ADDR 0x14
#define DEVICE_NAME "my_i2c_device"
#define CLASS_NAME "i2c_class"

static uint8_t stm_data[2] = {0};

#define     STM_OFF_TIME        _IOW('i', 0, uint8_t)
#define     STM_ON_TIME         _IOW('i', 1, uint8_t)
#define     STM_START_BLINK     _IO ('i', 3)
#define     STM_GET_TIME        _IOR('i', 4, uint8_t*)

#define     ON_INDEX            0
#define     OFF_INDEX           1

static struct class*  i2c_class  = NULL;
static struct device* i2c_device = NULL;
static int    major_number;

static const struct of_device_id my_i2c_of_match[] = {
    { .compatible = "my,i2c-slave-device" },
    { }
};
MODULE_DEVICE_TABLE(of, my_i2c_of_match);

static int my_i2c_open(struct inode *inode, struct file *file)
{
    file->private_data = dev_get_drvdata(i2c_device);;
    return 0;
}

static int my_i2c_release(struct inode *inode, struct file *file)
{
    return 0;
}

static long my_i2c_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret;
    struct i2c_client *client = file->private_data;

    switch(cmd) {
        case STM_OFF_TIME:
            if (copy_from_user(&stm_data[OFF_INDEX], (uint8_t __user *)arg, 1)) {
			    return -EFAULT;
            }
            break;

        case STM_ON_TIME:
            if (copy_from_user(&stm_data[ON_INDEX], (uint8_t __user *)arg, 1)) {
			    return -EFAULT;
            }
            break;

        case STM_START_BLINK:
            ret = i2c_master_send(client, stm_data, 2);
            if(ret < 0) {
                return -EFAULT;
            }
            pr_info("Sending data to STM: on: %d; off: %d\n",
                stm_data[ON_INDEX], stm_data[OFF_INDEX]);
            break;

        case STM_GET_TIME:
            if(copy_to_user((uint8_t __user*)arg, stm_data, 2)) {
                return -EFAULT;
            }

        default:
            break;
    }
    return 0;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_i2c_open,
    .release = my_i2c_release,
    .unlocked_ioctl = my_i2c_ioctl,
};

static int my_i2c_probe(struct i2c_client *client)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        pr_err("Failed to register a major number\n");
        return major_number;
    }

    i2c_class = class_create(CLASS_NAME);
    if (IS_ERR(i2c_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        pr_err("Failed to register device class\n");
        return PTR_ERR(i2c_class);
    }

    i2c_device = device_create(i2c_class, NULL, MKDEV(major_number, 0), client, DEVICE_NAME);
    if (IS_ERR(i2c_device)) {
        class_destroy(i2c_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        pr_err("Failed to create the device\n");
        return PTR_ERR(i2c_device);
    }

    pr_info("my_i2c_driver: probe function called\n");
    return 0;
}

static void my_i2c_remove(struct i2c_client *client)
{
    device_destroy(i2c_class, MKDEV(major_number, 0));
    class_unregister(i2c_class);
    class_destroy(i2c_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("my_i2c_driver: remove function called\n");
}

static const struct i2c_device_id my_i2c_id[] = {
    { "my_i2c_device", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, my_i2c_id);

static struct i2c_driver my_i2c_driver = {
    .driver = {
        .name = "my_i2c_device",
        .of_match_table = of_match_ptr(my_i2c_of_match),
    },
    .probe = my_i2c_probe,
    .remove = my_i2c_remove,
    .id_table = my_i2c_id,
};

module_i2c_driver(my_i2c_driver);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("I2C Driver for STM32 Slave Device with IOCTL");
MODULE_LICENSE("GPL");
