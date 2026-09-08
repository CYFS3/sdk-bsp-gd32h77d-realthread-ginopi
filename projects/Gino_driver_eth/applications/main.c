/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-01     CYFS         add the Gino Studio example entry
 */

#include <rtdevice.h>
#include <rtthread.h>
#include <board.h>

#ifndef GINO_PROJECT_NAME
#define GINO_PROJECT_NAME "Gino_driver_eth"
#endif

#define GINO_LED_PIN GET_PIN(C, 4)

int main(void)
{
    rt_pin_mode(GINO_LED_PIN, PIN_MODE_OUTPUT);
    rt_kprintf("\n%s is running.\n", GINO_PROJECT_NAME);
    rt_kprintf("Use list_device and gino_device_probe from the MSH console.\n");

    while (1)
    {
        rt_pin_write(GINO_LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(GINO_LED_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}

