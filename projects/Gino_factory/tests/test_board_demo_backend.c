/* SPDX-License-Identifier: Apache-2.0 */
#include <assert.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "board_demo_backend.h"

#define RT_USING_I2C
#define RT_I2C_WR 0
#define LOG_I(...) ((void)0)
#define LOG_E(...) ((void)0)

struct rt_i2c_bus_device { rt_uint32_t timeout; };
struct rt_i2c_msg
{
    rt_uint16_t addr, flags;
    rt_uint32_t len;
    rt_uint8_t *buf;
};
struct board_demo_command;

static struct board_demo_snapshot demo_state;
static rt_mutex_t demo_state_lock = (void *)1;
static rt_mq_t demo_command_queue = (void *)1;
static struct rt_i2c_bus_device test_bus;
static rt_ssize_t responses[128];
static rt_err_t lock_result, send_result;
static rt_uint32_t expected_timeout, ticks;
static unsigned transfer_count, lock_depth, dispatch_count, recv_count;
static unsigned char queued[1024];
static rt_size_t queued_size;
static int bus_missing;
static jmp_buf worker_exit;

static rt_err_t rt_mutex_take(rt_mutex_t mutex, int timeout)
{ RT_UNUSED(mutex); RT_UNUSED(timeout); return RT_EOK; }
static rt_err_t rt_mutex_release(rt_mutex_t mutex)
{ RT_UNUSED(mutex); return RT_EOK; }
static rt_tick_t rt_tick_get(void) { return ticks; }
static rt_tick_t rt_tick_from_millisecond(unsigned ms) { return ms; }
static rt_err_t rt_mq_send(rt_mq_t mq, const void *buffer, rt_size_t size)
{
    RT_UNUSED(mq);
    assert(size <= sizeof(queued));
    if (send_result == RT_EOK)
    {
        memcpy(queued, buffer, size);
        queued_size = size;
    }
    return send_result;
}
static rt_ssize_t rt_mq_recv(rt_mq_t mq, void *buffer, rt_size_t size, rt_tick_t timeout)
{
    RT_UNUSED(mq); RT_UNUSED(timeout);
    if (recv_count++ == 0)
    {
        assert(queued_size <= size);
        memcpy(buffer, queued, queued_size);
        return (rt_ssize_t)queued_size;
    }
    if (recv_count == 2) return -RT_ETIMEOUT;
    longjmp(worker_exit, 1);
}
static rt_err_t rt_i2c_bus_lock(struct rt_i2c_bus_device *bus, rt_tick_t timeout)
{
    assert(bus == &test_bus && timeout == 100);
    if (lock_result != RT_EOK) return lock_result;
    assert(lock_depth == 0);
    lock_depth++;
    return RT_EOK;
}
static rt_err_t rt_i2c_bus_unlock(struct rt_i2c_bus_device *bus)
{
    assert(bus == &test_bus && lock_depth == 1);
    lock_depth--;
    return RT_EOK;
}
static struct rt_i2c_bus_device *rt_i2c_bus_device_find(const char *name)
{ assert(strcmp(name, "hwi2c1") == 0); return bus_missing ? NULL : &test_bus; }
static rt_ssize_t rt_i2c_transfer(struct rt_i2c_bus_device *bus,
                                 struct rt_i2c_msg *message, rt_uint32_t count)
{
    assert(lock_depth == 1 && bus->timeout == expected_timeout);
    assert(count == 1 && message->len == 0 && message->flags == RT_I2C_WR);
    assert(message->addr >= 0x08 && message->addr <= 0x77);
    transfer_count++;
    ticks++;
    return responses[message->addr];
}
static void board_demo_handle_command(const struct board_demo_command *command);

/* The runner extracts these functions unchanged from the production C file. */
#include "board_demo_test_functions.h"

static void board_demo_handle_command(const struct board_demo_command *command)
{
    dispatch_count++;
    if (command->type == BOARD_DEMO_CMD_I2C_SCAN)
        board_demo_i2c_scan(command->data.i2c.bus_name);
}

static void reset_scan(void)
{
    struct board_demo_command command;
    memset(&demo_state, 0, sizeof(demo_state));
    memset(&command, 0, sizeof(command));
    for (unsigned i = 0; i < 128; i++) responses[i] = -RT_EIO;
    lock_result = send_result = RT_EOK;
    transfer_count = lock_depth = ticks = dispatch_count = recv_count = 0;
    bus_missing = 0;
    test_bus.timeout = 1000;
    expected_timeout = 20;
    command.type = BOARD_DEMO_CMD_I2C_SCAN;
    strcpy(command.data.i2c.bus_name, "hwi2c1");
    assert(board_demo_queue_bus_command(&command) == RT_EOK);
    assert(demo_state.i2c_busy);
}

static void check_released(void)
{
    assert(!demo_state.i2c_busy && lock_depth == 0);
    assert(test_bus.timeout == 1000);
    assert(demo_state.i2c_addresses[0x07] == BOARD_DEMO_I2C_SKIPPED);
    assert(demo_state.i2c_addresses[0x78] == BOARD_DEMO_I2C_SKIPPED);
}

int main(void)
{
    struct board_demo_command command;

    reset_scan();
    responses[0x14] = responses[0x5D] = 1;
    if (setjmp(worker_exit) == 0) board_demo_worker_entry(NULL);
    assert(dispatch_count == 1 && transfer_count == 112);
    assert(demo_state.i2c_device_count == 2 && demo_state.i2c_result_code == RT_EOK);
    assert(demo_state.i2c_addresses[0x14] == BOARD_DEMO_I2C_ACK);
    assert(demo_state.i2c_addresses[0x5D] == BOARD_DEMO_I2C_ACK);
    assert(demo_state.i2c_addresses[0x50] == BOARD_DEMO_I2C_NACK);
    check_released();

    reset_scan();
    board_demo_i2c_scan("hwi2c1");
    assert(transfer_count == 112 && demo_state.i2c_device_count == 0);
    assert(demo_state.i2c_result_code == RT_EOK);
    check_released();

    reset_scan();
    for (unsigned i = 0x08; i <= 0x77; i++) responses[i] = 1;
    board_demo_i2c_scan("hwi2c1");
    assert(demo_state.i2c_device_count == 112);
    assert(strlen(demo_state.i2c_log) < BOARD_DEMO_BUS_LOG_MAX);
    assert(strstr(demo_state.i2c_log, "DONE hwi2c1") != NULL);
    check_released();

    reset_scan();
    responses[0x14] = 1;
    responses[0x20] = -RT_ETIMEOUT;
    board_demo_i2c_scan("hwi2c1");
    assert(transfer_count == 25 && demo_state.i2c_result_code == -RT_ETIMEOUT);
    assert(demo_state.i2c_device_count == 1);
    assert(demo_state.i2c_addresses[0x20] == BOARD_DEMO_I2C_ERROR);
    assert(demo_state.i2c_addresses[0x21] == BOARD_DEMO_I2C_PENDING);
    assert(strstr(demo_state.i2c_log, "STOPPED") != NULL);
    check_released();

    reset_scan();
    lock_result = -RT_ETIMEOUT;
    board_demo_i2c_scan("hwi2c1");
    assert(transfer_count == 0 && demo_state.i2c_result_code == -RT_EBUSY);
    check_released();

    reset_scan();
    bus_missing = 1;
    board_demo_i2c_scan("hwi2c1");
    assert(transfer_count == 0 && demo_state.i2c_result_code == -RT_ENOENT);
    check_released();

    reset_scan();
    test_bus.timeout = expected_timeout = 5;
    assert(board_demo_i2c_probe(&test_bus, 0x08) == -RT_EIO);
    assert(test_bus.timeout == 5 && lock_depth == 0);

    reset_scan();
    responses[0x08] = -RT_ERROR;
    board_demo_i2c_scan("hwi2c1");
    assert(transfer_count == 1 && demo_state.i2c_result_code == -RT_ERROR);
    check_released();

    reset_scan();
    memset(&command, 0, sizeof(command));
    command.type = BOARD_DEMO_CMD_I2C_SCAN;
    strcpy(command.data.i2c.bus_name, "hwi2c1");
    assert(board_demo_queue_bus_command(&command) == -RT_EBUSY);
    demo_state.i2c_busy = RT_FALSE;
    send_result = -RT_EFULL;
    assert(board_demo_queue_bus_command(&command) == -RT_EFULL);
    assert(demo_state.i2c_result_code == -RT_EFULL && !demo_state.i2c_busy);

    puts("PASS: queue dispatch, ACK/NACK map, empty/dense scans, timeout abort,");
    puts("      lock contention, missing bus, timeout restoration, transfer/queue errors");
    return 0;
}
