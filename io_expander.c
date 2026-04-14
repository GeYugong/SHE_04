/**
 *    Copyright (c) 2024/12/18  DengWenjie@OurEDA, Dalian Univ of Tech
 *    Modified 2026 for Lab 3
 */

#include "pinctrl.h"
#include "common_def.h"
#include "soc_osal.h"
#include "i2c.h"
#include "app_init.h"
#include "gpio.h"
#include "hal_gpio.h"
#include "watchdog.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "io_expander.h"

int LED = 0;
int BUT = 0;

uint8_t io_expander_i2c_lock = 1;

void gpio_callback_func(pin_t pin, uintptr_t param)
{
    UNUSED(pin);
    UNUSED(param);

    /* 保存进入 ISR 前的状态, 用于判断是否有变化 */
    int initialLEDValue = LED;
    int initialBUTValue = BUT;

    uint8_t tx3_buff[2] = {0x1C, 0x08};
    uint8_t rx3_buff[2] = {0};
    i2c_data_t pca5 = {0};
    pca5.send_buf = tx3_buff;
    pca5.send_len = 2;
    pca5.receive_buf = rx3_buff;
    pca5.receive_len = 2;

    uapi_i2c_master_read(1, I2C_PCA_ADDR, &pca5);

    osal_printk("rx=%X %X LED=%d BUT=%d\r\n",
                *rx3_buff, *(rx3_buff + 1), LED, BUT);

    /* ---- 旋转编码器 ---- */
    if ((*rx3_buff & 0x80) && (*rx3_buff & 0x40)) {
        /* 右转 */
        LED++;
        if (LED >= 3) {
            LED = 0;
        }
        ssd1306_ClearOLED();
        ssd1306_printf("ENCODER turn right!");
    }
    if ((*rx3_buff & 0x80) && (!(*rx3_buff & 0x40))) {
        /* 左转 */
        LED--;
        if (LED < 0) {
            LED = 2;
        }
        ssd1306_ClearOLED();
        ssd1306_printf("ENCODER turn left!");
    }

    /* ---- 编码器按下 ---- */
    if (!(*(rx3_buff + 1) & 0x04)) {
        BUT = 1;
        ssd1306_ClearOLED();
        ssd1306_printf("Press ENCODER!");
    }

    /* ---- BUTTON1 ---- */
    if (!(*(rx3_buff + 1) & 0x08)) {
        BUT = 2;
        ssd1306_ClearOLED();
        ssd1306_printf("Press BUTTON1!");
    }

    /* ---- BUTTON2 ---- */
    if (!(*(rx3_buff + 1) & 0x10)) {
        BUT = 3;
        ssd1306_ClearOLED();
        ssd1306_printf("Press BUTTON2!");
    }

    /* ---- 根据 LED 值切换三色灯 + 显示颜色名 ---- */
    if (initialLEDValue != LED) {
        switch (LED) {
            case 0:
                uapi_gpio_set_val(LEDG, GPIO_LEVEL_HIGH);
                uapi_gpio_set_val(LEDR, GPIO_LEVEL_LOW);
                uapi_gpio_set_val(LEDB, GPIO_LEVEL_LOW);
                ssd1306_printf("GreenLed open!");
                break;
            case 1:
                uapi_gpio_set_val(LEDB, GPIO_LEVEL_HIGH);
                uapi_gpio_set_val(LEDG, GPIO_LEVEL_LOW);
                uapi_gpio_set_val(LEDR, GPIO_LEVEL_LOW);
                ssd1306_printf("BlueLed open!");
                break;
            case 2:
                uapi_gpio_set_val(LEDR, GPIO_LEVEL_HIGH);
                uapi_gpio_set_val(LEDG, GPIO_LEVEL_LOW);
                uapi_gpio_set_val(LEDB, GPIO_LEVEL_LOW);
                ssd1306_printf("RedLed open!");
                break;
        }
    }

    /* ---- 根据 BUT 值做按键指示 (不做 msleep blink, 会卡死 ISR) ---- */
    if (initialBUTValue != BUT) {
        switch (BUT) {
            case 1:
                /* 编码器按下 -> 红灯 */
                uapi_gpio_set_val(LEDR, GPIO_LEVEL_HIGH);
                uapi_gpio_set_val(LEDG, GPIO_LEVEL_LOW);
                uapi_gpio_set_val(LEDB, GPIO_LEVEL_LOW);
                ssd1306_printf("RedLed open!");
                break;
            case 2:
                /* BUTTON1 -> 绿灯 */
                uapi_gpio_set_val(LEDR, GPIO_LEVEL_LOW);
                uapi_gpio_set_val(LEDG, GPIO_LEVEL_HIGH);
                uapi_gpio_set_val(LEDB, GPIO_LEVEL_LOW);
                ssd1306_printf("GreenLed open!");
                break;
            case 3:
                /* BUTTON2 -> 蓝灯 */
                uapi_gpio_set_val(LEDR, GPIO_LEVEL_LOW);
                uapi_gpio_set_val(LEDG, GPIO_LEVEL_LOW);
                uapi_gpio_set_val(LEDB, GPIO_LEVEL_HIGH);
                ssd1306_printf("BlueLed open!");
                break;
        }
    }
    BUT = 0;
}

void io_expander_init(void)
{
    uapi_pin_set_mode(GPIO_12, PIN_MODE_0);
    uapi_pin_set_pull(GPIO_12, PIN_PULL_TYPE_DOWN);
    uapi_gpio_set_dir(GPIO_12, 0);

    uapi_pin_set_mode(LEDG, HAL_PIO_FUNC_GPIO);
    uapi_gpio_set_dir(LEDG, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(LEDG, GPIO_LEVEL_LOW);

    uapi_pin_set_mode(LEDB, HAL_PIO_FUNC_GPIO);
    uapi_gpio_set_dir(LEDB, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(LEDB, GPIO_LEVEL_LOW);

    uapi_pin_set_mode(LEDR, HAL_PIO_FUNC_GPIO);
    uapi_gpio_set_dir(LEDR, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(LEDR, GPIO_LEVEL_LOW);

    osal_printk("io expander init start\r\n");

    /* ---- 原版初始化命令, 不改 ---- */
    uint8_t tx_buff[2] = {0x10, 0x3B};
    uint8_t rx_buff[2] = {0};
    i2c_data_t pca2 = {0};
    pca2.send_buf = tx_buff;
    pca2.send_len = 2;
    pca2.receive_buf = rx_buff;
    pca2.receive_len = 2;
    errcode_t ret;
    for (int i = 0; i <= 10; i++) {
        ret = uapi_i2c_master_write(1, I2C_PCA_ADDR, &pca2);
    }
    if (ret != 0) {
        printf("io expander init failed, ret = %0x\r\n", ret);
    }

    uint8_t tx4_buff[2] = {0x18, 0x00};
    uint8_t rx4_buff[2] = {0};
    i2c_data_t pca6 = {0};
    pca6.send_buf = tx4_buff;
    pca6.send_len = 2;
    pca6.receive_buf = rx4_buff;
    pca6.receive_len = 2;
    for (int i = 0; i <= 10; i++) {
        ret = uapi_i2c_master_write(1, I2C_PCA_ADDR, &pca6);
    }
    if (ret != 0) {
        printf("io expander init failed, ret = %0x\r\n", ret);
    }

    uint8_t tx1_buff[3] = {0x1B, 0xFF, 0x00};
    uint8_t rx1_buff[2] = {0};
    i2c_data_t pca3 = {0};
    pca3.send_buf = tx1_buff;
    pca3.send_len = 3;
    pca3.receive_buf = rx1_buff;
    pca3.receive_len = 2;
    for (int i = 0; i <= 10; i++) {
        ret = uapi_i2c_master_write(1, I2C_PCA_ADDR, &pca3);
    }
    if (ret != 0) {
        printf("io expander init failed, ret = %0x\r\n", ret);
    }

    errcode_t ASD = uapi_gpio_register_isr_func(GPIO_12, 0x00000008, gpio_callback_func);
    if (ASD != 0) {
        uapi_gpio_unregister_isr_func(GPIO_12);
        osal_printk("io expander init FAILED:%0x\r\n", ASD);
    } else {
        osal_printk("io expander init SUCC!\r\n");
    }

    io_expander_i2c_lock = 1;
}