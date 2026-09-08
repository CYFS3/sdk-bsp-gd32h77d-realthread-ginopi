# Gino_display_camera

## 1. 简介

本示例在 GD32H77D Gino 开发板上实现 OV7670 图像采集，通过 LVGL 和 MIPI LCD 实时预览。LVGL 初始化后自动启动预览。

主设备为 `ov7670`。UART1 用作 FinSH/MSH 控制台，参数为 115200-8-N-1，PC4 LED 用作运行指示。

## 2. 采集与显示

I2C2/SCCB 配置 OV7670，TIMER0 通过 PE9 提供 XCLK。DCI 接收并行像素流，DMA1 通道 7 将数据写入内部 AXI SRAM 的独立采集缓冲。完成 Cache 维护和 RGB565 数据处理后，`rt_device_read` 将一帧 QVGA 图像复制到应用提供的目标缓冲。

预览线程另外使用两个内部 AXI SRAM 缓冲，跟踪正在写入、就绪和正在显示的缓冲，保证 LVGL 图像源在渲染完成前保持稳定。LVGL 通过 32 行绘制缓冲渲染，IPA 更新 SDRAM 后台帧缓冲，LCD 驱动在垂直消隐期交换帧缓冲。

## 3. 内存要求

DMA 采集缓冲及两个预览缓冲均按 32 字节对齐。LCD 帧缓冲使用 SDRAM 预留区；启用 LCD 时，SDRAM heap 从 `0xC0300000` 开始。

板级初始化完成 SDRAM 和 MPU 配置后再开启 D-Cache。LCD 的使用依赖 SDRAM 成功初始化。通过 bootloader 跳转时，由 bootloader 在交接前清理并关闭 D-Cache。

## 4. 设备接口

`ov7670` 设备支持 `rt_device_open` 和 `rt_device_read`。输出缓冲需要容纳一帧 320 x 240 RGB565 图像，读取成功时返回帧数据的字节数。

预览使用 `lcd` 图形设备和 LVGL image API，在 LVGL 线程中更新界面。启用 GT911 时提供触摸输入。

## 5. 硬件说明

- OV7670：连接供电、共地、复位、XCLK、PCLK、VSYNC、HREF、D0-D7 和 I2C2/SCCB。
- 摄像头控制：I2C2 使用 PA8/PC9，XCLK 使用 PE9。
- 显示：MIPI LCD 和 SDRAM；GT911 触摸使用 I2C3。
- 控制台：UART1，PA2/PA3，AF7，115200-8-N-1。
- 运行 LED：PC4，每 500 ms 翻转。

断电后连接摄像头，供电和信号电平应满足模块要求。

## 6. 示例使用

以下源码路径以 SDK 仓库中的工程目录为基准：

- `applications/main.c`
- `applications/lv_port.c`
- `applications/ov7670_preview.c`
- `../../libraries/Board_Drivers/drv_ov7670.c`
- `../../libraries/Board_Drivers/ports/camera/sensors/ov7670.c`

| 命令                     | 作用                       |
| ------------------------ | -------------------------- |
| `ov7670_preview start` | 请求在 LCD 上启动实时预览  |
| `ov7670_preview stop`  | 停止实时预览并释放预览界面 |

1. 连接摄像头、LCD、电源和控制台。
2. 通过 RT-Thread Studio 或 MDK5 编译下载本工程。
3. 复位开发板，等待 LCD 自动显示摄像头画面。
4. 需要手动控制预览时，使用 start 和 stop 命令。

## 7. 预期效果

- LVGL 初始化显示设备并启动 OV7670 预览。
- LCD 持续显示 QVGA RGB565 摄像头图像。
- 可通过控制台停止并重新启动预览。
