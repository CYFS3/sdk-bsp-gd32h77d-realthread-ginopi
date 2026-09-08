/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-08     CYFS         add the RTC demo entry with ulog output
 */

#include <rtdevice.h>
#include <rtthread.h>
#include <board.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#ifndef GINO_PROJECT_NAME
#define GINO_PROJECT_NAME "Gino_driver_rtc"
#endif

#define GINO_LED_PIN GET_PIN(C, 4)

int main(void)
{
    rt_pin_mode(GINO_LED_PIN, PIN_MODE_OUTPUT);
    LOG_I("%s is running", GINO_PROJECT_NAME);
    LOG_I("RTC commands: date, rtc_alarm help, gino_device_probe");

    while (1)
    {
        rt_pin_write(GINO_LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(GINO_LED_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}

