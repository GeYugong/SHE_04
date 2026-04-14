/**
 * Copyright (c) 2024/12/18  DengWenjie@OurEDA, Dalian Univ of Tech
 * Modified 2026 for Lab 3
 */
#ifndef IO_EXPANDER_H
#define IO_EXPANDER_H

#include <stdint.h>
#include "gpio.h"

#define LEDG GPIO_11
#define LEDB GPIO_07
#define LEDR GPIO_09
#define I2C_PCA_ADDR 0x28

/* 事件类型 —— ISR 里写, 主循环读 */
typedef enum {
    IO_EVT_NONE = 0,
    IO_EVT_ENC_LEFT,
    IO_EVT_ENC_RIGHT,
    IO_EVT_ENC_PRESS,
    IO_EVT_BTN1,
    IO_EVT_BTN2,
} io_event_t;

extern volatile io_event_t g_io_event;
extern volatile uint8_t g_io_event_flag;

void io_expander_init(void);

#endif