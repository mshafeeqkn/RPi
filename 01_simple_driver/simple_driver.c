#include <linux/module.h>

static int __init simple_drv_readtest_init(void)
{
    printk("Hello kernel\n");
    return 0;
}

static void __exit simple_drv_readtest_exit(void)
{
    printk("Bye kernel\n");
	return;
}

module_init(simple_drv_readtest_init);
module_exit(simple_drv_readtest_exit);

MODULE_DESCRIPTION("This is a very basic sample kernel code");
MODULE_AUTHOR("Mohammed Shafeeque");
MODULE_LICENSE("GPL");
