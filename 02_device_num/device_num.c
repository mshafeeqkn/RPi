// This driver code is to register and unregister a major number 
// and minor number in the kernel but the associated device file
// won't be created. They way how to create the device file is
// covered in the next example.

#include <linux/module.h>

#define DEVICE_NAME     "drv_device_num"
#define CLASS_NAME      "device_num_class"
#define STATIC_MAJOR    64

static int            major_num;

static struct file_operations fops = {
    // TODO: fill the file operations
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
    return 0;
}

static void __exit simple_drv_readtest_exit(void) {
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "Bye kernel\n");
	return;
}

module_init(simple_drv_readtest_init);
module_exit(simple_drv_readtest_exit);

MODULE_DESCRIPTION("This is a very basic sample kernel code");
MODULE_AUTHOR("Mohammed Shafeeque");
MODULE_LICENSE("GPL");
