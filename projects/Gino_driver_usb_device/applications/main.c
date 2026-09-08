/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-08     CYFS         add the USB CDC periodic transmission and echo demo
 */

#include <rtdevice.h>
#include <rtthread.h>
#include <board.h>
#include <drivers/usb_device.h>

#ifndef GINO_PROJECT_NAME
#define GINO_PROJECT_NAME "Gino_driver_usb_device"
#endif

#define GINO_LED_PIN GET_PIN(C, 4)
#define GINO_CDC_DEVICE_NAME "vcom"

static rt_device_t gino_cdc_init(void)
{
    rt_device_t device;
    rt_err_t result;

    device = rt_device_find(GINO_CDC_DEVICE_NAME);
    if (device == RT_NULL)
    {
        rt_kprintf("CDC device %s was not found.\n", GINO_CDC_DEVICE_NAME);
        return RT_NULL;
    }

    /* Polling TX copies the local buffers before rt_device_write returns. */
    result = rt_device_open(device, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    if (result != RT_EOK)
    {
        rt_kprintf("Failed to open %s: %d\n", GINO_CDC_DEVICE_NAME, result);
        return RT_NULL;
    }

    rt_kprintf("CDC ready: open the USB COM port with DTR enabled.\n");
    return device;
}

static void gino_cdc_poll(rt_device_t device)
{
    static rt_tick_t last_tx;
    static rt_uint32_t sequence;
    rt_tick_t now;
    rt_bool_t connected = RT_FALSE;
    rt_uint8_t rx_buffer[64];
    char tx_buffer[64];
    rt_ssize_t size;
    int length;

    if (device == RT_NULL)
    {
        return;
    }

    now = rt_tick_get();
    size = rt_device_read(device, 0, rx_buffer, sizeof(rx_buffer));
    if (rt_device_control(device, RT_USBD_CLASS_CTRL_CONNECTED, &connected) != RT_EOK ||
        !connected)
    {
        /* Discard inactive-session input and send immediately after DTR is set. */
        last_tx = now - RT_TICK_PER_SECOND;
        return;
    }

    if (size > 0)
    {
        if (rt_device_write(device, 0, rx_buffer, (rt_size_t)size) != size)
        {
            rt_kprintf("CDC echo write failed.\n");
        }
    }

    if ((rt_tick_t)(now - last_tx) >= RT_TICK_PER_SECOND)
    {
        last_tx = now;
        length = rt_snprintf(tx_buffer, sizeof(tx_buffer),
                             "Gino USB CDC test: %lu\r\n", (unsigned long)sequence);
        if (rt_device_write(device, 0, tx_buffer, (rt_size_t)length) == length)
        {
            sequence++;
        }
        else
        {
            rt_kprintf("CDC test write failed.\n");
        }
    }
}

int main(void)
{
    rt_device_t cdc_device;
    rt_tick_t last_led;
    rt_tick_t now;
    rt_uint8_t led_state = PIN_HIGH;

    rt_pin_mode(GINO_LED_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(GINO_LED_PIN, led_state);
    rt_kprintf("\n%s is running.\n", GINO_PROJECT_NAME);
    rt_kprintf("Use list_device and gino_device_probe from the MSH console.\n");

    cdc_device = gino_cdc_init();
    last_led = rt_tick_get();
    while (1)
    {
        gino_cdc_poll(cdc_device);
        now = rt_tick_get();
        if ((rt_tick_t)(now - last_led) >= rt_tick_from_millisecond(500))
        {
            last_led = now;
            led_state = (led_state == PIN_HIGH) ? PIN_LOW : PIN_HIGH;
            rt_pin_write(GINO_LED_PIN, led_state);
        }
        rt_thread_mdelay(10);
    }
}

