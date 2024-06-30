#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define DEVICE_NAME     "gpio_intr"

#define GPIO_SWITCH     517

static unsigned int irq_number;

static irqreturn_t gpio_irq_handler(int irq, void *dev_id) {
    printk(KERN_INFO "gpio_intr: ISR has been invoked\n");
    return IRQ_HANDLED;
}

static int __init simple_drv_readtest_init(void) {
    int ret;

    if((ret = gpio_request(GPIO_SWITCH, "rpi-gpio-switch"))) {
        printk( KERN_ALERT "Can't alloc GPIO_SWITCH(%d); ret: %d\n", GPIO_SWITCH, ret);
        return -1;
    }

    if(gpio_direction_input(GPIO_SWITCH)) {
        printk( KERN_ALERT "Cannot set direction GPIO_SWITCH(%d)\n", GPIO_SWITCH);
        gpio_free(GPIO_SWITCH);
    }

    irq_number = gpio_to_irq(GPIO_SWITCH);
    if(request_irq(irq_number, gpio_irq_handler, IRQF_TRIGGER_RISING, "switch_irq", NULL) != 0) {
        gpio_free(GPIO_SWITCH);
        return -1;
    }

    printk(KERN_INFO "Success..!!!! Device driver created successfully\n");
    return 0;
}

static void __exit simple_drv_readtest_exit(void) {
    free_irq(irq_number, NULL);
    gpio_free(GPIO_SWITCH);
    printk(KERN_INFO "Bye kernel\n");
    return;
}

module_init(simple_drv_readtest_init);
module_exit(simple_drv_readtest_exit);

MODULE_DESCRIPTION("This is a GPIO driver code to blink an LED in a loop");
MODULE_AUTHOR("Mohammed Shafeeque");
MODULE_LICENSE("GPL");
