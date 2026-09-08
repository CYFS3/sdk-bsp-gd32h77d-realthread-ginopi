#ifndef BOARD_DEMO_TEST_RTTHREAD_H
#define BOARD_DEMO_TEST_RTTHREAD_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t rt_uint8_t;
typedef uint16_t rt_uint16_t;
typedef uint32_t rt_uint32_t;
typedef uint64_t rt_uint64_t;
typedef uint32_t rt_tick_t;
typedef size_t rt_size_t;
typedef ptrdiff_t rt_ssize_t;
typedef int rt_err_t;
typedef int rt_bool_t;
typedef void *rt_mutex_t;
typedef void *rt_mq_t;

#define RT_NAME_MAX 16
#define RT_EOK 0
#define RT_ERROR 1
#define RT_ETIMEOUT 2
#define RT_EFULL 3
#define RT_EBUSY 7
#define RT_EIO 8
#define RT_ENOENT 11
#define RT_NULL NULL
#define RT_TRUE 1
#define RT_FALSE 0
#define RT_WAITING_FOREVER (-1)
#define RT_TICK_PER_SECOND 1000
#define RT_UNUSED(x) ((void)(x))
#define rt_memset memset
#define rt_memcpy memcpy
#define rt_memmove memmove
#define rt_strlen strlen
#define rt_strncpy strncpy
#define rt_snprintf snprintf
#define rt_vsnprintf vsnprintf

#endif
