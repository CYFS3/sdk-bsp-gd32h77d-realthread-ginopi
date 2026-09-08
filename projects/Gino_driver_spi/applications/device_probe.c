/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-08     CYFS         add SPI3 loopback and peripheral probe commands with ulog output
 */

#include <rtdevice.h>
#include <rtthread.h>

#define LOG_TAG "spi.test"
#define LOG_LVL LOG_LVL_INFO
#include <ulog.h>

#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

#ifndef GINO_DEVICE_NAME
#define GINO_DEVICE_NAME "spi3"
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

#if defined(RT_USING_SPI) && defined(BSP_USING_SPI3) && defined(RT_USING_FINSH)
#include <stdlib.h>

#define SPI_LOOPBACK_BUS_NAME    "spi3"
#define SPI_LOOPBACK_DEVICE_NAME "spi3_loop"
#define SPI_LOOPBACK_MAX_LENGTH  256
#define SPI_LOOPBACK_MAX_COUNT   1000

static struct rt_spi_device loopback_device;

static rt_err_t spi_loopback_parse_number(const char *text, long maximum, rt_size_t *value)
{
    char *end;
    long number = strtol(text, &end, 10);

    if (end == text || *end != '\0' || number < 1 || number > maximum)
    {
        return -RT_EINVAL;
    }

    *value = (rt_size_t)number;
    return RT_EOK;
}

static int spi_loopback(int argc, char **argv)
{
    struct rt_spi_configuration configuration = {0};
    rt_uint8_t tx_buffer[SPI_LOOPBACK_MAX_LENGTH];
    rt_uint8_t rx_buffer[SPI_LOOPBACK_MAX_LENGTH];
    rt_size_t length = 32;
    rt_size_t count = 10;
    rt_size_t round;
    rt_size_t index;
    rt_ssize_t transferred;
    rt_device_t device;
    rt_err_t result;
    rt_bool_t help = argc == 2 &&
                    (!rt_strcmp(argv[1], "-h") || !rt_strcmp(argv[1], "--help"));

    if (help || argc > 3 ||
        (argc > 1 && spi_loopback_parse_number(argv[1], SPI_LOOPBACK_MAX_LENGTH, &length) != RT_EOK) ||
        (argc > 2 && spi_loopback_parse_number(argv[2], SPI_LOOPBACK_MAX_COUNT, &count) != RT_EOK))
    {
        if (!help)
        {
            LOG_W("Invalid arguments: length must be 1-%d and count must be 1-%d.",
                  SPI_LOOPBACK_MAX_LENGTH, SPI_LOOPBACK_MAX_COUNT);
        }
        LOG_RAW("Usage: spi_loopback [length:1-%d] [count:1-%d]\n",
                SPI_LOOPBACK_MAX_LENGTH, SPI_LOOPBACK_MAX_COUNT);
        LOG_RAW("Defaults: 32 bytes, 10 rounds; 8-bit, mode 0, MSB first, max_hz=1000000, no CS.\n");
        LOG_RAW("Connect %s (MOSI) to %s (MISO); disconnect external SPI targets first.\n",
                BSP_SPI3_MOSI_PIN, BSP_SPI3_MISO_PIN);
        return help ? RT_EOK : -RT_EINVAL;
    }

    device = rt_device_find(SPI_LOOPBACK_DEVICE_NAME);
    if (device != &loopback_device.parent)
    {
        if (device != RT_NULL)
        {
            LOG_E("SPI loopback FAIL: device name %s is already in use.",
                  SPI_LOOPBACK_DEVICE_NAME);
            return -RT_EBUSY;
        }

        result = rt_spi_bus_attach_device(&loopback_device, SPI_LOOPBACK_DEVICE_NAME,
                                          SPI_LOOPBACK_BUS_NAME, RT_NULL);
        if (result != RT_EOK)
        {
            LOG_E("SPI loopback FAIL: attach to %s failed (%d).",
                  SPI_LOOPBACK_BUS_NAME, result);
            return result;
        }
    }

    configuration.data_width = 8;
    configuration.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB | RT_SPI_NO_CS;
    configuration.max_hz = 1000000;
    result = rt_spi_configure(&loopback_device, &configuration);
    /* A different bus owner defers configuration until the transfer. */
    if (result != RT_EOK && result != -RT_EBUSY)
    {
        LOG_E("SPI loopback FAIL: configure failed (%d).", result);
        return result;
    }

    LOG_I("SPI loopback: %s, %u bytes x %u rounds, max_hz=1000000",
          SPI_LOOPBACK_BUS_NAME, (unsigned int)length, (unsigned int)count);
    LOG_I("Wiring: %s (MOSI) <-> %s (MISO), SCK=%s, no CS.",
          BSP_SPI3_MOSI_PIN, BSP_SPI3_MISO_PIN, BSP_SPI3_SCK_PIN);

    for (round = 0; round < count; round++)
    {
        for (index = 0; index < length; index++)
        {
            tx_buffer[index] = (rt_uint8_t)(0x5A + index + round);
            rx_buffer[index] = (rt_uint8_t)~tx_buffer[index];
        }

        transferred = rt_spi_transfer(&loopback_device, tx_buffer, rx_buffer, length);
        if (transferred != (rt_ssize_t)length)
        {
            LOG_E("SPI loopback FAIL: round %u, transferred %d/%u bytes.",
                  (unsigned int)(round + 1), (int)transferred, (unsigned int)length);
            return -RT_EIO;
        }

        for (index = 0; index < length; index++)
        {
            if (rx_buffer[index] != tx_buffer[index])
            {
                LOG_E("SPI loopback FAIL: round %u, offset %u, TX=0x%02X RX=0x%02X.",
                      (unsigned int)(round + 1), (unsigned int)index,
                      (unsigned int)tx_buffer[index], (unsigned int)rx_buffer[index]);
                return -RT_EIO;
            }
        }
    }

    LOG_I("SPI loopback PASS: %u rounds, %u bytes checked.",
          (unsigned int)count, (unsigned int)(length * count));
    return RT_EOK;
}
MSH_CMD_EXPORT(spi_loopback, test SPI3 with MOSI connected to MISO);
#endif

