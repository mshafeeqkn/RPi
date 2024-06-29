#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define DEVICE_NAME     "drv_dev_ops"
#define CLASS_NAME      "drv_dev_ops_class"
#define STATIC_MAJOR    64
#define MESSAGE_LEN     24

static int              major_num;
static dev_t            dev_file;

static struct class    *char_class = NULL;
static struct device   *char_device = NULL;
static char             msg[MESSAGE_LEN] = "This is sample message\n";


static ssize_t drv_read(struct file *f, char *buffer, size_t count, loff_t *offs) {
    int pending;

    printk(KERN_INFO "offset: %lld; count: %lu\n", *offs, count);
    if(*offs >= MESSAGE_LEN) return 0;

    if (count > MESSAGE_LEN - *offs) {
        count = MESSAGE_LEN - *offs;
    }

    pending = copy_to_user(buffer, msg + *offs, count);
    if(pending > 0) {
        count -= pending;
    }

    *offs += count;
    return count;
}

static int drv_open(struct inode *device_file, struct file *instance) {
    printk(KERN_INFO "device file %s opened\n", DEVICE_NAME);
    return 0;
}

static ssize_t drv_write(struct file *f, const char *buffer, size_t count, loff_t *offs) {
    size_t msg_len = count;

    if( count > MESSAGE_LEN) {
        msg_len = MESSAGE_LEN;
    }

    memset(msg, 0, sizeof(msg));
    int pending = copy_from_user(msg, buffer, msg_len);

    msg_len -= pending;

    printk(KERN_INFO "Written %lu bytes\n", msg_len);
    return msg_len;
}

static struct file_operations fops = {
    .open  = drv_open,
    .read  = drv_read,
    .write = drv_write
};

static int __init simple_drv_readtest_init(void) {
    // use below line to allocate a major number dynamically
    // major_num = register_chrdev(0, DEVICE_NAME, &fops);
    major_num = register_chrdev(STATIC_MAJOR, DEVICE_NAME, &fops);
    if( major_num < 0 ) {
        printk(KERN_ALERT "Failed to register the device number\n");
        return major_num;
    } else if (major_num == 0) {
        printk(KERN_INFO "Registered device with device number: %d\n", STATIC_MAJOR);
        major_num = STATIC_MAJOR;
    } else {
        printk(KERN_INFO "Device %s registered with major: %d; minor: %d\n",
                DEVICE_NAME, major_num >> 20, major_num & 0xFFFFF);
    }

    // Register class driver
    char_class = class_create(CLASS_NAME);
    if( IS_ERR(char_class)) {
        unregister_chrdev(major_num, DEVICE_NAME);
        return PTR_ERR(char_class);
    }

    printk(KERN_INFO "Device class registered successfully; major number: %d\n", major_num);

    // create the device file for the driver
    dev_file = MKDEV(major_num, 0);
    char_device = device_create(char_class, NULL, dev_file,
                                NULL, DEVICE_NAME);
    if( IS_ERR(char_device) ) {
        class_destroy(char_class);
        unregister_chrdev(major_num, DEVICE_NAME);
        printk( KERN_ALERT "Failed to create the device\n" );
        return PTR_ERR(char_device);
    }

    printk(KERN_INFO "Success..!!!! Device driver created successfully\n");
    return 0;
}

static void __exit simple_drv_readtest_exit(void) {
    device_destroy(char_class, dev_file);
    class_unregister(char_class);
    class_destroy(char_class);
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "Bye kernel\n");
	return;
}

module_init(simple_drv_readtest_init);
module_exit(simple_drv_readtest_exit);

MODULE_DESCRIPTION("This is a very basic sample kernel code");
MODULE_AUTHOR("Mohammed Shafeeque");
MODULE_LICENSE("GPL");
