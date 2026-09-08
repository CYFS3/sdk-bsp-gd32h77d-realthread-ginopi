/*
 * Copyright (c) 2006-2026 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-07     CYFS         run the LVGL music demo with dual-buffer IPA flushing
 */

#include <rtconfig.h>

#ifdef PKG_USING_LVGL

#include <lvgl.h>
#include <rtdevice.h>
#include <board.h>

#include "gd32h77x_78x_ipa.h"
#include "gd32h77x_78x_tli.h"

#define DBG_TAG                         "lvgl.disp"
#define DBG_LVL                         DBG_INFO
#include <rtdbg.h>

#define LCD_DEVICE_NAME                 "lcd"
#define TOUCH_DEVICE_NAME               "gt911"
#define TOUCH_POINT_COUNT               5U

#define LCD_BYTES_PER_PIXEL             2U
#define LCD_DRAW_BUFFER_LINES           200U
#define LCD_SCAN_GUARD_LINES            96U
#define LCD_SCAN_WAIT_TIMEOUT_MS        18U

#if LV_USE_DEMO_SMARTWATCH && !defined(BSP_USING_LVGL_BOARD_DEMO)
#define LVGL_DISPLAY_WIDTH              384U
#define LVGL_DISPLAY_HEIGHT             384U
#define LVGL_USE_DIRECT_FRAMEBUFFER     0
#else
#define LVGL_DISPLAY_WIDTH              LV_HOR_RES_MAX
#define LVGL_DISPLAY_HEIGHT             LV_VER_RES_MAX
#define LVGL_USE_DIRECT_FRAMEBUFFER     0
#endif

#if !LVGL_USE_DIRECT_FRAMEBUFFER
#define LCD_DRAW_BUFFER_SIZE            (LVGL_DISPLAY_WIDTH * LCD_DRAW_BUFFER_LINES * LCD_BYTES_PER_PIXEL)
#endif

static rt_device_t lcd_device;
static struct rt_device_graphic_info lcd_info;
static lv_display_t *lvgl_display;
#if !LVGL_USE_DIRECT_FRAMEBUFFER
rt_align(32) static uint8_t lcd_draw_buffer_1[LCD_DRAW_BUFFER_SIZE];
rt_align(32) static uint8_t lcd_draw_buffer_2[LCD_DRAW_BUFFER_SIZE];
#endif
static rt_uint16_t lcd_viewport_x;
static rt_uint16_t lcd_viewport_y;
static rt_bool_t display_ready;
#if LVGL_USE_DIRECT_FRAMEBUFFER
static rt_bool_t flush_error_reported;
#else
static lv_display_t * volatile g_flushing_display;
#endif

#ifdef BSP_USING_TOUCH_GT911
int rt_hw_gt911_port_init(void);

static rt_device_t touch_device;
static lv_coord_t touch_x;
static lv_coord_t touch_y;
static rt_bool_t touch_pressed;
#endif

#if !LVGL_USE_DIRECT_FRAMEBUFFER
static void lcd_clean_dcache(void *address, rt_size_t size)
{
#if __CORTEX_M >= 0x07
    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, address, size);
#else
    RT_UNUSED(address);
    RT_UNUSED(size);
#endif
}

static void lcd_ipa_init(void)
{
    rcu_periph_clock_enable(RCU_IPA);
    ipa_interval_clock_num_config(255U);
    ipa_inter_timer_config(IPA_INTER_TIMER_ENABLE);
}

static rt_uint16_t lcd_mod_line(rt_int32_t line, rt_uint16_t total_lines)
{
    while (line < 0)
    {
        line += total_lines;
    }
    return (rt_uint16_t)((rt_uint32_t)line % total_lines);
}

static rt_bool_t lcd_tli_line_in_range(rt_uint16_t line,
                                       rt_int32_t start,
                                       rt_int32_t end,
                                       rt_uint16_t total_lines)
{
    rt_uint32_t window_lines;
    rt_uint16_t wrapped_start;
    rt_uint16_t wrapped_end;

    if (total_lines == 0U)
    {
        return RT_FALSE;
    }

    window_lines = (rt_uint32_t)(end - start + 1);
    if (window_lines >= total_lines)
    {
        return RT_TRUE;
    }

    wrapped_start = lcd_mod_line(start, total_lines);
    wrapped_end = lcd_mod_line(start + (rt_int32_t)window_lines - 1,
                               total_lines);
    if (wrapped_start <= wrapped_end)
    {
        return (line >= wrapped_start) && (line <= wrapped_end);
    }

    return (line >= wrapped_start) || (line <= wrapped_end);
}

static void lcd_wait_scanline_away(const lv_area_t *area)
{
    rt_uint32_t layer_vpos = TLI_LXVPOS(LAYER0);
    rt_uint16_t layer_top = (rt_uint16_t)GET_BITS(layer_vpos, 0U, 11U);
    rt_uint16_t layer_bottom = (rt_uint16_t)GET_BITS(layer_vpos, 16U, 27U);
    rt_uint16_t total_lines = (rt_uint16_t)(GET_BITS(TLI_TSZ, 0U, 11U) + 1U);
    rt_int32_t flush_top = (rt_int32_t)layer_top +
                           (rt_int32_t)lcd_viewport_y + area->y1;
    rt_int32_t flush_bottom = (rt_int32_t)layer_top +
                              (rt_int32_t)lcd_viewport_y + area->y2;
    rt_int32_t danger_start = flush_top - (rt_int32_t)LCD_SCAN_GUARD_LINES;
    rt_int32_t danger_end = flush_bottom + (rt_int32_t)LCD_SCAN_GUARD_LINES;
    rt_uint32_t danger_lines = (rt_uint32_t)(danger_end - danger_start + 1);
    rt_tick_t start_tick;
    rt_tick_t timeout;

    if ((total_lines == 0U) || (layer_bottom <= layer_top))
    {
        return;
    }
    if (danger_lines >= total_lines)
    {
        return;
    }

    start_tick = rt_tick_get();
    timeout = rt_tick_from_millisecond(LCD_SCAN_WAIT_TIMEOUT_MS);
    if (timeout == 0U)
    {
        timeout = 1U;
    }
    do
    {
        rt_uint16_t current_line = (rt_uint16_t)GET_BITS(TLI_CPPOS, 0U, 15U);

        if (!lcd_tli_line_in_range(current_line, danger_start, danger_end,
                                   total_lines))
        {
            return;
        }
    }
    while ((rt_tick_get() - start_tick) < timeout);
}

static void lcd_flush(lv_display_t *display,
                      const lv_area_t *area,
                      uint8_t *color_p)
{
    rt_uint32_t width;
    rt_uint32_t height;
    rt_uint8_t *destination;

    width = (rt_uint32_t)lv_area_get_width(area);
    height = (rt_uint32_t)lv_area_get_height(area);

    while (g_flushing_display != RT_NULL)
    {
    }
    destination = (rt_uint8_t *)lcd_info.framebuffer +
                  LCD_BYTES_PER_PIXEL *
                  ((rt_uint32_t)lcd_info.width *
                   ((rt_uint32_t)area->y1 + lcd_viewport_y) +
                   (rt_uint32_t)area->x1 + lcd_viewport_x);

    g_flushing_display = display;

    /* Finish cache maintenance before sampling the scanline safety window. */
    lcd_clean_dcache(color_p, (rt_size_t)width * height * LCD_BYTES_PER_PIXEL);
    lcd_wait_scanline_away(area);

    IPA_CTL = IPA_FGTODE | IPA_CTL_FTFIE;
    IPA_FMADDR = (rt_uint32_t)color_p;
    IPA_DMADDR = (rt_uint32_t)destination;
    IPA_FLOFF = 0U;
    IPA_DLOFF = (rt_uint32_t)lcd_info.width - width;
    IPA_FPCTL = FOREGROUND_PPF_RGB565;
    IPA_IMS = (width << 16U) | (height & 0xFFFFU);
    IPA_CTL |= IPA_CTL_TEN;
}

void IPA_IRQHandler(void)
{
    rt_interrupt_enter();

    if (SET == ipa_interrupt_flag_get(IPA_INT_FLAG_FTF))
    {
        ipa_interrupt_flag_clear(IPA_INT_FLAG_FTF);
        if (g_flushing_display != RT_NULL)
        {
            lv_display_flush_ready(g_flushing_display);
            g_flushing_display = RT_NULL;
        }
    }

    rt_interrupt_leave();
}
#else
static void lcd_mark_dirty_area(const lv_area_t *area)
{
    struct rt_device_rect_info rect;

    rect.x = (rt_uint16_t)area->x1;
    rect.y = (rt_uint16_t)area->y1;
    rect.width = (rt_uint16_t)lv_area_get_width(area);
    rect.height = (rt_uint16_t)lv_area_get_height(area);
    if (rt_device_control(lcd_device, RTGRAPHIC_CTRL_RECT_UPDATE,
                          &rect) != RT_EOK)
    {
        (void)rt_device_control(lcd_device, RTGRAPHIC_CTRL_RECT_UPDATE,
                                RT_NULL);
    }
}

static void lcd_flush(lv_display_t *display,
                      const lv_area_t *area,
                      uint8_t *color_p)
{
    rt_err_t result;

    lcd_mark_dirty_area(area);

    if (!lv_display_flush_is_last(display))
    {
        lv_display_flush_ready(display);
        return;
    }

    /* Direct mode also synchronizes untouched areas between framebuffers. */
    result = rt_device_control(lcd_device, RTGRAPHIC_CTRL_PAN_DISPLAY,
                               color_p);
    if (result == RT_EOK)
    {
        result = rt_device_control(lcd_device, RTGRAPHIC_CTRL_WAIT_VSYNC,
                                   RT_NULL);
    }
    if (result == RT_EOK)
    {
        flush_error_reported = RT_FALSE;
    }
    else if (!flush_error_reported)
    {
        LOG_E("framebuffer swap failed: %d", result);
        flush_error_reported = RT_TRUE;
    }

    lv_display_flush_ready(display);
}
#endif

void lv_port_disp_init(void)
{
    rt_err_t result;
    rt_uint32_t framebuffer_size;
#if LVGL_USE_DIRECT_FRAMEBUFFER
    rt_uint8_t *back_framebuffer;
#endif

    lcd_device = rt_device_find(LCD_DEVICE_NAME);
    if (lcd_device == RT_NULL)
    {
        LOG_E("cannot find %s device", LCD_DEVICE_NAME);
        return;
    }

    result = rt_device_open(lcd_device, RT_DEVICE_OFLAG_RDWR);
    if (result != RT_EOK)
    {
        LOG_E("cannot open %s device: %d", LCD_DEVICE_NAME, result);
        return;
    }

    result = rt_device_control(lcd_device, RTGRAPHIC_CTRL_GET_INFO, &lcd_info);
    framebuffer_size = (rt_uint32_t)lcd_info.width * lcd_info.height *
                       LCD_BYTES_PER_PIXEL;
    if ((result != RT_EOK) ||
        (lcd_info.width != LV_HOR_RES_MAX) ||
        (lcd_info.height != LV_VER_RES_MAX) ||
        (lcd_info.pixel_format != RTGRAPHIC_PIXEL_FORMAT_RGB565) ||
        (lcd_info.bits_per_pixel != 16U) ||
        (lcd_info.framebuffer == RT_NULL) ||
        (lcd_info.smem_len < framebuffer_size *
                             (LVGL_USE_DIRECT_FRAMEBUFFER ? 2U : 1U)))
    {
        LOG_E("invalid %s framebuffer", LCD_DEVICE_NAME);
        rt_device_close(lcd_device);
        return;
    }

    lcd_viewport_x = (rt_uint16_t)((lcd_info.width - LVGL_DISPLAY_WIDTH) / 2U);
    lcd_viewport_y = (rt_uint16_t)((lcd_info.height - LVGL_DISPLAY_HEIGHT) / 2U);

    lvgl_display = lv_display_create(LVGL_DISPLAY_WIDTH, LVGL_DISPLAY_HEIGHT);
    if (lvgl_display == RT_NULL)
    {
        LOG_E("cannot create LVGL display");
        rt_device_close(lcd_device);
        return;
    }

    lv_display_set_color_format(lvgl_display, LV_COLOR_FORMAT_RGB565);
#if LVGL_USE_DIRECT_FRAMEBUFFER
    back_framebuffer = (rt_uint8_t *)lcd_info.framebuffer + framebuffer_size;
    lv_display_set_buffers(lvgl_display,
                           back_framebuffer,
                           lcd_info.framebuffer,
                           framebuffer_size,
                           LV_DISPLAY_RENDER_MODE_DIRECT);
#else
    lv_display_set_buffers(lvgl_display,
                           lcd_draw_buffer_1,
                           lcd_draw_buffer_2,
                           sizeof(lcd_draw_buffer_1),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif
    lv_display_set_flush_cb(lvgl_display, lcd_flush);
    lv_display_set_default(lvgl_display);

#if !LVGL_USE_DIRECT_FRAMEBUFFER
    lcd_ipa_init();
    nvic_irq_enable(IPA_IRQn, 2U, 0U);
#endif
    display_ready = RT_TRUE;

#if LVGL_USE_DIRECT_FRAMEBUFFER
    LOG_I("%ux%u RGB565 direct framebuffers at %p/%p",
          lcd_info.width, lcd_info.height,
          lcd_info.framebuffer, back_framebuffer);
    LOG_I("vertical blank framebuffer swapping enabled");
#else
    LOG_I("%ux%u LVGL viewport centered on %ux%u RGB565 panel",
          LVGL_DISPLAY_WIDTH, LVGL_DISPLAY_HEIGHT,
          lcd_info.width, lcd_info.height);
    LOG_I("partial buffers (%u lines) at %p/%p",
          LCD_DRAW_BUFFER_LINES,
          lcd_draw_buffer_1, lcd_draw_buffer_2);
    LOG_I("single framebuffer IPA flush strategy enabled");
#endif
}

#ifdef BSP_USING_TOUCH_GT911
static void touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    struct rt_touch_data touch_data[TOUCH_POINT_COUNT];
    rt_size_t point_index;
    rt_bool_t active_point_found = RT_FALSE;

    RT_UNUSED(indev);

    rt_memset(touch_data, 0, sizeof(touch_data));
    if (rt_device_read(touch_device, 0, touch_data, TOUCH_POINT_COUNT) == TOUCH_POINT_COUNT)
    {
        for (point_index = 0; point_index < TOUCH_POINT_COUNT; point_index++)
        {
            if ((touch_data[point_index].event == RT_TOUCH_EVENT_DOWN) ||
                (touch_data[point_index].event == RT_TOUCH_EVENT_MOVE))
            {
                if ((touch_data[point_index].x_coordinate >= lcd_viewport_x) &&
                    (touch_data[point_index].x_coordinate < lcd_viewport_x + LVGL_DISPLAY_WIDTH) &&
                    (touch_data[point_index].y_coordinate >= lcd_viewport_y) &&
                    (touch_data[point_index].y_coordinate < lcd_viewport_y + LVGL_DISPLAY_HEIGHT))
                {
                    touch_x = (lv_coord_t)(touch_data[point_index].x_coordinate - lcd_viewport_x);
                    touch_y = (lv_coord_t)(touch_data[point_index].y_coordinate - lcd_viewport_y);
                    touch_pressed = RT_TRUE;
                }
                else
                {
                    touch_pressed = RT_FALSE;
                }

                active_point_found = RT_TRUE;
                break;
            }
        }

        if (!active_point_found)
        {
            for (point_index = 0; point_index < TOUCH_POINT_COUNT; point_index++)
            {
                if (touch_data[point_index].event == RT_TOUCH_EVENT_UP)
                {
                    touch_pressed = RT_FALSE;
                    break;
                }
            }
        }
    }

    data->point.x = touch_x;
    data->point.y = touch_y;
    data->state = touch_pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}
#endif

void lv_port_indev_init(void)
{
#ifdef BSP_USING_TOUCH_GT911
    rt_err_t result;
    lv_indev_t *input_device;

    touch_device = rt_device_find(TOUCH_DEVICE_NAME);
    if (touch_device == RT_NULL)
    {
        result = rt_hw_gt911_port_init();
        if (result == RT_EOK)
        {
            touch_device = rt_device_find(TOUCH_DEVICE_NAME);
        }
        if (touch_device == RT_NULL)
        {
            LOG_E("cannot initialize %s device: %d",
                  TOUCH_DEVICE_NAME, result);
            return;
        }
    }

    result = rt_device_open(touch_device, RT_DEVICE_OFLAG_RDONLY);
    if (result != RT_EOK)
    {
        LOG_E("cannot open %s device: %d", TOUCH_DEVICE_NAME, result);
        touch_device = RT_NULL;
        return;
    }

    input_device = lv_indev_create();
    if (input_device == RT_NULL)
    {
        LOG_E("cannot create LVGL input device");
        rt_device_close(touch_device);
        touch_device = RT_NULL;
        return;
    }

    lv_indev_set_type(input_device, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(input_device, touch_read);
    lv_indev_set_display(input_device, lvgl_display);

    LOG_I("%s registered as LVGL pointer", TOUCH_DEVICE_NAME);
#endif
}

void lv_user_gui_init(void)
{
    if (!display_ready)
    {
        return;
    }

#if LV_USE_DEMO_MUSIC
    extern void lv_demo_music(void);

    lv_demo_music();
    lv_refr_now(NULL);
#elif defined(BSP_USING_LVGL_BOARD_DEMO)
    extern void board_demo_init(void);

    board_demo_init();
    lv_refr_now(NULL);
#elif LV_USE_DEMO_SMARTWATCH
    extern void lv_demo_smartwatch(void);

    lv_demo_smartwatch();
    lv_refr_now(NULL);
#elif LV_USE_DEMO_BENCHMARK
    extern void lv_demo_benchmark(void);

    lv_demo_benchmark();
    lv_refr_now(NULL);
#else
    LOG_W("no LVGL demo is enabled");
    lv_refr_now(NULL);
#endif

#if defined(BSP_USING_OV7670) && !defined(BSP_USING_LVGL_BOARD_DEMO) && \
    !LV_USE_DEMO_SMARTWATCH
    extern void ov7670_preview_init(void);

    ov7670_preview_init();
#endif
}

#endif /* PKG_USING_LVGL */
