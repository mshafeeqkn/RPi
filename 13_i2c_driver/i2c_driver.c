#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/kernel.h>


#define     I2C_BUS_NUMBER              1
#define     STM_DEVICE_NAME             "STM32_DEV"
#define     STM_DEVICE_ADDR             0x12

static struct i2c_adapter        *stm_adapter = NULL;
static struct i2c_client         *stm_client  = NULL;

static struct i2c_board_info  stm_board_info = {
    I2C_BOARD_INFO(STM_DEVICE_NAME, STM_DEVICE_ADDR)
};

static struct i2c_driver stm_driver = {
    .driver = {
        .name  = STM_DEVICE_NAME,
        .owner = THIS_MODULE,
    },
};

static int  __init stm_driver_init(void) {
    int ret = -1;

    stm_adapter = i2c_get_adapter(I2C_BUS_NUMBER);
    if(stm_adapter == NULL) {
        pr_err("Failed to get the I2C bus\n");
        return ret;
    }

    stm_client = i2c_new_client_device(stm_adapter, &stm_board_info);
    if(stm_client == NULL) {
        pr_err("Unable to create i2c device\n");
        return ret;
    }

    i2c_add_driver(&stm_driver);
    ret = 0;

    i2c_put_adapter(stm_adapter);
    pr_info("STM32 Driver added\n");
    return ret;
}

static void __exit stm_driver_exit(void) {
    i2c_unregister_device(stm_client);
    i2c_del_driver(&stm_driver);
    pr_info("Driver removed\n");
}

module_init(stm_driver_init);
module_exit(stm_driver_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Muhammed Shafeeque K N");
MODULE_DESCRIPTION("A simple I2C driver to interface STM32F103C6T6 board");
MODULE_VERSION("1.0");
