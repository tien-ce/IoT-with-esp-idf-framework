/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdint.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define GPIO_BASE_ADDR 0x60004000
// For GPIO 0-31
#define GPIO_OUT_W1TS_OFFSET 0x0008
#define GPIO_OUT_W1TC_OFFSET 0x000C
#define GPIO_ENABLE_W1TS_REG 0x0024
#define GPIO_ENABLE_W1TC_REG 0x0028

// For GPIO 32-48
#define GPIO_OUT1_W1TS_OFFSET 0x0014
#define GPIO_OUT1_W1TC_OFFSET 0x0018
#define GPIO_ENABLE1_W1TS_REG 0x0030
#define GPIO_ENABLE1_W1TC_REG 0x0034
#define LED_GPIO_NUM 48
#define DELAY_TIME_MS 1000
void app_main(void)
{
    volatile uint32_t* gpio_out1_w1ts_reg = (uint32_t*) (GPIO_BASE_ADDR + GPIO_OUT1_W1TS_OFFSET);
    volatile uint32_t* gpio_out1_w1tc_reg = (uint32_t*) (GPIO_BASE_ADDR + GPIO_OUT1_W1TC_OFFSET);
    volatile uint32_t* gpio_enable1_w1ts_reg = (uint32_t*) (GPIO_BASE_ADDR + GPIO_ENABLE1_W1TS_REG);
    // volatile uint32_t* gpio_enable1_w1tc_reg = (uint32_t*) (GPIO_BASE_ADDR + GPIO_ENABLE1_W1TC_REG);
    // Enable the LED GPIO as output
    *(gpio_enable1_w1ts_reg) = (1 << (LED_GPIO_NUM - 32));

    while (1) {
         // GPIO 32-48 correspond to bits 0-16 in OUT1 register
        // Turn on the LED
        *(gpio_out1_w1ts_reg) = (1 << (LED_GPIO_NUM - 32));
        vTaskDelay (pdMS_TO_TICKS(DELAY_TIME_MS));
        printf ("LED ON\n");
        // Turn off the LED
        *(gpio_out1_w1tc_reg) = (1 << (LED_GPIO_NUM - 32));
        printf ("LED OFF\n");
        vTaskDelay (pdMS_TO_TICKS(DELAY_TIME_MS));
    }
}