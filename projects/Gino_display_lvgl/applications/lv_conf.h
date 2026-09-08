/*
 * Copyright (c) 2006-2026 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-04     CYFS         configure the LVGL music demo for LVGL v9
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_FORMAT_DEFAULT         LV_COLOR_FORMAT_RGB565
#define LV_DISABLE_ASSERT_HANDLER_INCLUDE_WARNING 1

#define LV_HOR_RES_MAX                  720
#define LV_VER_RES_MAX                  720

#define LV_USE_SYSMON                   1
#define LV_USE_PERF_MONITOR             0

#define LV_USE_DEMO_BENCHMARK           0
#define LV_USE_DEMO_SMARTWATCH          0
#define LV_USE_DEMO_MUSIC               1
#define LV_DEMO_MUSIC_AUTO_PLAY         0
#define LV_DEMO_MUSIC_LARGE             0
#define LV_DEMO_MUSIC_ROUND             0
#define LV_USE_LOTTIE                   0

#define LV_FONT_MONTSERRAT_12           1
#define LV_FONT_MONTSERRAT_16           1

#endif /* LV_CONF_H */
