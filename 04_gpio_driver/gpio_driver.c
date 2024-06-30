#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define DEVICE_NAME     "drv_gpio"
#define CLASS_NAME      "drv_gpio_class"
#define STATIC_MAJOR    64
#define DATA_LEN        32

#define GPIO_LED        516
#define GPIO_SWITCH     1

static int              major_num;
static dev_t            dev_file;

static struct class    *char_class = NULL;
static struct device   *char_device = NULL;
static char             data[DATA_LEN] = "This is sample message\n";
static struct task_struct *task;
static unsigned long   delay = 1000;

// @brief thread function to turn on and off the LED
// for a particular ms in a loop
// @param data - not used
static int thread_function(void *data) {
    int led_state;
    while(!kthread_should_stop()) {
        led_state = gpio_get_value(GPIO_LED);
        led_state ^= 1;
        gpio_set_value(GPIO_LED, led_state);
        msleep(delay);
    }
    return 0;
}

// @breif take the written data from the user space, process and 
// set the corresponding delay for LED toggling
// 
// @param f - not used
// @param buffer - data sent from the user through write operation
// @param count - number of char sent
// @param offs - not used
static ssize_t drv_write(struct file *f, const char *buffer, size_t count, loff_t *offs) {
    unsigned long d;
    if(count <= 0) {
        return -1;
    }

    if((d = simple_strtol(buffer, NULL, 10))) {
        delay = d;
    }
    return count;
}

static struct file_operations fops = {
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
        goto class_create_fail;
    }

    printk(KERN_INFO "Device class registered successfully; major: %d\n", major_num);

    // create the device file for the driver
    dev_file = MKDEV(major_num, 0);
    char_device = device_create(char_class, NULL, dev_file,
                                NULL, DEVICE_NAME);
    if( IS_ERR(char_device) ) {
        printk( KERN_ALERT "Failed to create the device\n" );
        goto dev_destroy_fail;
    }

    if(gpio_is_valid(GPIO_LED) == 0) {
        printk( KERN_ALERT "GPIO is not valid\n");
        goto gpio_led_failed;
    }

    if(gpio_is_valid(GPIO_LED) == 0) {
        printk( KERN_ALERT "GPIO is not valid\n");
        goto gpio_led_failed;
    }

    struct gpio_desc *desc = gpio_to_desc(GPIO_LED);
    printk( KERN_ALERT "Descriptor ret: %p\n", desc);

    int ret;
    if((ret = gpio_request(GPIO_LED, "rpi-gpio-led"))) {
        printk( KERN_ALERT "Cannot allocate GPIO_LED(%d); ret: %d\n", GPIO_LED, ret);
        goto gpio_led_failed;
    }

    if(gpio_direction_output(GPIO_LED, 0)) {
        printk( KERN_ALERT "Cannot set direction GPIO_LED(%d)\n", GPIO_LED);
        goto gpio_led_failed;
    }

    task = kthread_run(thread_function, NULL, "gpio_thread");
    if(IS_ERR(task)) {
        printk(KERN_ALERT "GPIO thread create failed\n");
        goto gpio_thread_failed;
    }

    printk(KERN_INFO "Success..!!!! Device driver created successfully\n");
    return 0;

gpio_thread_failed:
    gpio_set_value(GPIO_LED, 0);
    gpio_free(GPIO_LED);

gpio_led_failed:
    device_destroy(char_class, dev_file);
    class_unregister(char_class);

dev_destroy_fail:
    class_destroy(char_class);

class_create_fail:
    unregister_chrdev(major_num, DEVICE_NAME);
    return ENODEV;
}

static void __exit simple_drv_readtest_exit(void) {
    kthread_stop(task);
    gpio_set_value(GPIO_LED, 0);
    gpio_free(GPIO_LED);
    device_destroy(char_class, dev_file);
    class_unregister(char_class);
    class_destroy(char_class);
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "Bye kernel\n");
	return;
}

module_init(simple_drv_readtest_init);
module_exit(simple_drv_readtest_exit);

MODULE_DESCRIPTION("This is a GPIO driver code to blink an LED in a loop");
MODULE_AUTHOR("Mohammed Shafeeque");
MODULE_LICENSE("GPL");
