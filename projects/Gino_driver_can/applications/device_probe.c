/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-08     CYFS         add the peripheral probe command with ulog output
 */

#include <rtdevice.h>
#include <rtthread.h>

#define LOG_TAG "can.probe"
#define LOG_LVL LOG_LVL_INFO
#include <ulog.h>

#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

#ifndef GINO_DEVICE_NAME
#define GINO_DEVICE_NAME "can1"
#endif

static int gino_device_probe(void)
{
    rt_device_t device;

    if (GINO_DEVICE_NAME[0] == '\0')
    {
        LOG_I("This example uses GPIO directly; run pin list for details.");
        return RT_EOK;
    }

    device = rt_device_find(GINO_DEVICE_NAME);
    if (device == RT_NULL)
    {
        LOG_E("device %s was not found", GINO_DEVICE_NAME);
        return -RT_ENOSYS;
    }

    LOG_I("device %s is ready: type=%d, flag=0x%x, open_flag=0x%x",
          GINO_DEVICE_NAME, device->type, device->flag, device->open_flag);
    return RT_EOK;
}
MSH_CMD_EXPORT(gino_device_probe, probe the primary device used by this example);

