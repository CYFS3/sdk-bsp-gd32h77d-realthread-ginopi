/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-08     CYFS         add the command-driven CAN and CAN FD demo with ulog output
 */

#include <rtdevice.h>
#include <rtthread.h>
#include <board.h>

#define LOG_TAG "can.demo"
#define LOG_LVL LOG_LVL_INFO
#include <ulog.h>

#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

#ifndef GINO_PROJECT_NAME
#define GINO_PROJECT_NAME "Gino_driver_can"
#endif

#define GINO_LED_PIN GET_PIN(C, 4)

#ifndef GINO_DEVICE_NAME
#define GINO_DEVICE_NAME "can1"
#endif

static rt_device_t can_device;
static struct rt_semaphore can_rx_sem;
static rt_bool_t can_demo_ready;

static void can_print_message(const char *direction, const struct rt_can_msg *message)
{
    char payload[sizeof(message->data) * 3U + 1U] = {0};
    const char *format = "CAN";
    rt_uint32_t brs = 0U;
    rt_size_t length = 0U;

#ifdef RT_CAN_USING_CANFD
    format = message->fd_frame ? "CANFD" : "CAN";
    brs = message->brs;
#endif
    if (message->rtr == RT_CAN_DTR)
    {
        for (rt_uint32_t index = 0U; index < message->len && index < sizeof(message->data); index++)
        {
            length += rt_snprintf(payload + length, sizeof(payload) - length,
                                  "%s%02X", index == 0U ? "" : " ", message->data[index]);
        }
    }
    LOG_I("[%s] %s %s %s ID=0x%08X RTR=%u BRS=%u LEN=%u DATA=%s",
          direction, GINO_DEVICE_NAME, format, message->ide == RT_CAN_STDID ? "STD" : "EXT",
          message->id, message->rtr, brs, message->len, length == 0U ? "-" : payload);
}

static rt_err_t can_rx_indicate(rt_device_t device, rt_size_t size)
{
    RT_UNUSED(device);
    RT_UNUSED(size);
    return rt_sem_release(&can_rx_sem);
}

static void can_rx_entry(void *parameter)
{
    struct rt_can_msg message;
    RT_UNUSED(parameter);

    while (1)
    {
        if (rt_sem_take(&can_rx_sem, RT_WAITING_FOREVER) != RT_EOK)
        {
            continue;
        }
        while (1)
        {
            rt_memset(&message, 0, sizeof(message));
            message.hdr_index = -1;
            if (rt_device_read(can_device, 0, &message, sizeof(message)) != sizeof(message))
            {
                break;
            }
            can_print_message("RX", &message);
        }
    }
}

static rt_err_t can_demo_init(void)
{
    struct can_configure config;
    rt_thread_t rx_thread;
    rt_err_t result;

    can_device = rt_device_find(GINO_DEVICE_NAME);
    if (can_device == RT_NULL)
    {
        return -RT_ENOSYS;
    }
    if (can_device->ref_count != 0U)
    {
        return -RT_EBUSY;
    }
    result = rt_device_init(can_device);
    if (result != RT_EOK)
    {
        return result;
    }

    config = ((struct rt_can_device *)can_device)->config;
    config.mode = RT_CAN_MODE_NORMAL;
    config.privmode = RT_CAN_MODE_NOPRIV;
    config.baud_rate = CAN500kBaud;
#ifdef RT_CAN_USING_CANFD
    config.enable_canfd = 1U;
    config.baud_rate_fd = 2000000U;
    config.use_bit_timing = 0U;
#endif
    result = rt_device_control(can_device, RT_DEVICE_CTRL_CONFIG, &config);
    if (result != RT_EOK)
    {
        return result;
    }
    result = rt_sem_init(&can_rx_sem, "canrx", 0, RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        return result;
    }
    rx_thread = rt_thread_create("canrx", can_rx_entry, RT_NULL, 2048, 11, 10);
    if (rx_thread == RT_NULL)
    {
        rt_sem_detach(&can_rx_sem);
        return -RT_ENOMEM;
    }
    rt_device_set_rx_indicate(can_device, can_rx_indicate);
    result = rt_device_open(can_device, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
    if (result != RT_EOK)
    {
        goto cleanup;
    }
    result = rt_thread_startup(rx_thread);
    if (result != RT_EOK)
    {
        rt_device_set_rx_indicate(can_device, RT_NULL);
        rt_device_close(can_device);
        goto cleanup;
    }

#ifdef RT_CAN_USING_CANFD
    LOG_I("%s normal mode: arbitration=500000 bit/s, ISO CAN FD data=2000000 bit/s", GINO_DEVICE_NAME);
#else
    LOG_I("%s normal mode: arbitration=500000 bit/s", GINO_DEVICE_NAME);
#endif
    return RT_EOK;

cleanup:
    rt_device_set_rx_indicate(can_device, RT_NULL);
    rt_thread_delete(rx_thread);
    rt_sem_detach(&can_rx_sem);
    return result;
}

#ifdef RT_USING_FINSH
static rt_err_t can_send_frame(rt_bool_t fd_frame)
{
    struct rt_can_msg message = {0};
    rt_ssize_t written;

    message.id = 0x123U;
    message.ide = RT_CAN_STDID;
    message.rtr = RT_CAN_DTR;
    message.len = 8U;
#ifdef RT_CAN_USING_CANFD
    if (fd_frame)
    {
        message.id = 0x124U;
        message.fd_frame = 1U;
        message.brs = 1U;
        message.len = 64U;
    }
#else
    RT_UNUSED(fd_frame);
#endif
    for (rt_uint32_t index = 0U; index < message.len; index++)
    {
        message.data[index] = (rt_uint8_t)index;
    }
    written = rt_device_write(can_device, 0, &message, sizeof(message));
    if (written == sizeof(message))
    {
        can_print_message("TX", &message);
        return RT_EOK;
    }
    else
    {
        LOG_E("[TX FAIL] %s ID=0x%08X LEN=%u result=%d",
              GINO_DEVICE_NAME, message.id, message.len, (int)written);
        return -RT_ERROR;
    }
}

static int gino_can_send(void)
{
    rt_err_t result;

    if (!can_demo_ready)
    {
        LOG_E("%s is not ready", GINO_DEVICE_NAME);
        return -RT_ERROR;
    }

    result = can_send_frame(RT_FALSE);
#ifdef RT_CAN_USING_CANFD
    rt_thread_mdelay(100);
    if (can_send_frame(RT_TRUE) != RT_EOK)
    {
        result = -RT_ERROR;
    }
#endif
    return result;
}
MSH_CMD_EXPORT(gino_can_send, send one CAN frame then one CAN FD frame when enabled);
#endif

int main(void)
{
    rt_err_t result;

    rt_pin_mode(GINO_LED_PIN, PIN_MODE_OUTPUT);
    LOG_I("%s is running.", GINO_PROJECT_NAME);
    result = can_demo_init();
    if (result != RT_EOK)
    {
        LOG_E("%s initialization failed: %d", GINO_DEVICE_NAME, result);
    }
    else
    {
        can_demo_ready = RT_TRUE;
        LOG_I("RX ready. Run gino_can_send to send one set of frames.");
    }

    while (1)
    {
        rt_pin_write(GINO_LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(GINO_LED_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}

