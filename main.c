#include "pinctrl.h"
#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"
#include "i2c.h"
#include "bh1750.h"

static int blinky_task(const char *arg)
{
    unused(arg);
    uapi_pin_set_mode(CONFIG_I2C_SCL_MASTER_PIN, CONFIG_I2C_MASTER_PIN_MODE);
    uapi_pin_set_mode(CONFIG_I2C_SDA_MASTER_PIN, CONFIG_I2C_MASTER_PIN_MODE);
    uint32_t baudrate = I2C_SET_BANDRATE;
    uint32_t hscode = I2C_MASTER_ADDR;
    errcode_t ret = uapi_i2c_master_init(1, baudrate, hscode);
    if (ret != 0) {
        printf("i2c init failed, ret = %0x\r\n", ret);
    }
    bh1750_init();
    while (1) {
        osal_msleep(10000);
        uint16_t lightness = bh1750_GetLightIntensity();
        osal_printk("BH1750:%05d\r\n", lightness);
    }
    return 0;
}