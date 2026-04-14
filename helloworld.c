/*
 * Lab 3 - OLED + Encoder + Buttons
 */
#include "pinctrl.h"
#include "common_def.h"
#include "soc_osal.h"
#include "i2c.h"
#include "osal_debug.h"
#include "ssd1306_fonts.h"
#include "ssd1306.h"
#include "io_expander.h"
#include "app_init.h"

#define CONFIG_I2C_SCL_MASTER_PIN 15
#define CONFIG_I2C_SDA_MASTER_PIN 16
#define CONFIG_I2C_MASTER_PIN_MODE 2
#define I2C_MASTER_ADDR 0x0
#define I2C_SET_BANDRATE 400000
#define I2C_TASK_STACK_SIZE 0x1000
#define I2C_TASK_PRIO 17

void app_i2c_init_pin(void)
{
    uapi_pin_set_mode(CONFIG_I2C_SCL_MASTER_PIN, CONFIG_I2C_MASTER_PIN_MODE);
    uapi_pin_set_mode(CONFIG_I2C_SDA_MASTER_PIN, CONFIG_I2C_MASTER_PIN_MODE);
}

void OledTask(void)
{
    uint32_t baudrate = I2C_SET_BANDRATE;
    uint32_t hscode = I2C_MASTER_ADDR;
    app_i2c_init_pin();
    errcode_t ret = uapi_i2c_master_init(1, baudrate, hscode);
    if (ret != 0) {
        printf("i2c init failed, ret = %0x\r\n", ret);
    }

    ssd1306_Init();
    ssd1306_Fill(Black);

    /* ===== Demo A: 验证显示规则 ===== */
    ssd1306_printf("%05dLx %02dC %02dRH", 66666, (uint16_t)10, 1000);
    ssd1306_printf("%s", "Hello Claude!");
    osal_msleep(8000);   /* 停 8 秒看效果拍照 */

    /* ===== Demo B: 验证显示限制 ===== */
    ssd1306_ClearOLED();
    ssd1306_printf("%05dLx %02dC %02dRH", 66666, (uint16_t)10, 10000);   /* 19 字符 -> 丢最后一个 */
    ssd1306_printf("%05dLx %02dC %02dRH", 66666, (uint16_t)10, 114514);  /* 20+ 字符 -> 空行 */
    ssd1306_printf("%s", "Man!瓦特 can I say?");                         /* 含中文 -> 空行 */
    ssd1306_printf("%05dLx %02dC %02dRH", 66666, (uint16_t)10, 1000);    /* 正常 18 字符 */
    ssd1306_printf("%s", "Man!What can I say?");                         /* 第 5 行超出屏幕 */
    osal_msleep(8000);

    /* ===== Demo C: 带 ClearOLED 的循环 ===== */
    ssd1306_ClearOLED();
    for (int i = 1; i < 200; i++) {
        ssd1306_printf("%d", i);
        ssd1306_ClearOLED();
        osal_msleep(50);
    }
    osal_msleep(3000);

    /* ===== Demo D: 不带 ClearOLED 的循环 ===== */
    ssd1306_ClearOLED();
    for (int i = 1; i < 200; i++) {
        ssd1306_printf("%d", i);
        osal_msleep(50);
    }

    while (1) {
        osal_msleep(1000);
    }
}
void oled_entry(void)
{
    uint32_t ret;
    osal_task *taskid;
    osal_kthread_lock();
    taskid = osal_kthread_create((osal_kthread_handler)OledTask, NULL, "OledTask", I2C_TASK_STACK_SIZE);
    ret = osal_kthread_set_priority(taskid, I2C_TASK_PRIO);
    if (ret != OSAL_SUCCESS) {
        printf("create task1 failed .\n");
    }
    osal_kthread_unlock();
}

app_run(oled_entry);

