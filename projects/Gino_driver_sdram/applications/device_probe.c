/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-01     CYFS         add a common peripheral probe command
 */

#include <rtdevice.h>
#include <rtthread.h>

#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

#ifndef GINO_DEVICE_NAME
#define GINO_DEVICE_NAME ""
#endif

static int gino_device_probe(void)
{
    rt_device_t device;

    if (GINO_DEVICE_NAME[0] == '\0')
    {
        rt_kprintf("This example uses GPIO directly; run pin list for details.\n");
        return RT_EOK;
    }

    device = rt_device_find(GINO_DEVICE_NAME);
    if (device == RT_NULL)
    {
        rt_kprintf("device %s was not found\n", GINO_DEVICE_NAME);
        return -RT_ENOSYS;
    }

    rt_kprintf("device %s is ready: type=%d, flag=0x%x, open_flag=0x%x\n",
               GINO_DEVICE_NAME, device->type, device->flag, device->open_flag);
    return RT_EOK;
}
MSH_CMD_EXPORT(gino_device_probe, probe the primary device used by this example);

