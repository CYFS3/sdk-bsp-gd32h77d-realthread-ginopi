# sdk-bsp-gd32h77d-gino

[English](README.md) | 中文

## 概览

本仓库是 GD32H77D Gino 开发板的 RT-Thread Studio 板级支持包，基于 RT-Thread 5.3.0。SDK 共享 `rt-thread`、GD32 库和离线软件包，并提供 16 个独立工程，用于外设示例和应用演示。

![GD32H77D Gino](figures/board.png)

工程覆盖基础模板、UART、I2C、SPI、QSPI Flash、SD 卡、CAN、PWM、RTC、SDRAM、USB Device/Host、LVGL 显示触摸、Wi-Fi、以太网、摄像头、MQTT 和综合出厂演示，同时保留 Studio 与 MDK5 工程支持。

## 开发板资源

| 项目       | 配置                                             |
| ---------- | ------------------------------------------------ |
| MCU        | GD32H77DIW，Arm Cortex-M7，最高 600 MHz          |
| 时钟源     | 25 MHz HXTAL                                     |
| 控制台     | UART1，PA2/PA3，AF7，115200-8-N-1                |
| 运行指示灯 | PC4，默认每 500 ms 翻转                          |
| 外部 SDRAM | 32 MiB，地址`0xC0000000`                       |
| 板载 Flash | 8 MiB GD25Q64E QSPI Flash，连接 OSPI0            |
| 显示       | 720 x 720 MIPI DSI LCD，FL7707N，RGB565          |
| 触摸       | GT911，I2C3                                      |
| 摄像头     | OV7670，QVGA RGB565，DCI + DMA                   |
| 以太网     | ENET1 RMII，默认 PHY 地址 2                      |
| Wi-Fi      | GD32VW553 AT 模块，UART4                         |
| 下载       | SWD；Studio 使用 DAP-Link/PyOCD，MDK 使用 DAP-Link（CMSIS-DAP） |

引脚、内存、前置条件和示例操作见 [工程使用指南](docs/PROJECT_GUIDE_zh.md)。

## 架构说明

[架构与工作原理](docs/ARCHITECTURE_zh.md) 说明从复位到 RT-Thread 启动的链路、Kconfig/SCons 生成流程、RT-Thread 设备模型、Cortex-M7 Cache/DMA ownership，以及显示、摄像头、以太网、Wi-Fi、存储和 MQTT 的数据路径。

## 工程列表

| 工程                       | 用途                          | 主要设备或挂载点            | 验证方式                         |
| -------------------------- | ----------------------------- | --------------------------- | -------------------------------- |
| `Gino_template`          | LED 闪烁与开发模板            | PC4 LED                     | `pin list`                     |
| `Gino_driver_i2c`        | I2C1 总线                     | `hwi2c1`                  | `i2c scan hwi2c1 08 78`        |
| `Gino_driver_spi`        | SPI3 总线                     | `spi3`                    | `list_device`                  |
| `Gino_driver_qspi_flash` | GD25Q64E、FAL、FatFs          | `qspi_flash0`、`/flash` | `fal probe filesystem`         |
| `Gino_driver_sdcard`     | SDIO1 与 FatFs                | `sd0`、`/sd`            | `ls /sd`                       |
| `Gino_driver_can`        | CAN1                          | `can1`                    | `gino_device_probe`            |
| `Gino_driver_pwm`        | 双路 PWM 输出                 | `pwm2`、`pwm30`         | `pwm_dual_test start`          |
| `Gino_driver_rtc`        | RTC 与 Alarm                  | `rtc`                     | `date`                         |
| `Gino_driver_sdram`      | 外部 SDRAM                    | SDRAM heap                  | `free`                         |
| `Gino_driver_usb_device` | USBHS0 Device                 | `usbd`                    | `list_device`                  |
| `Gino_driver_usb_host`   | USBHS1 Host 与 U 盘           | `usbh`、`/udisk`        | `ls /udisk`                    |
| `Gino_driver_eth`        | ENET1 RMII 以太网             | `e0`                      | `ifconfig`、`ping <gateway>` |
| `Gino_display_lvgl`      | LCD、触摸与 LVGL              | `lcd`、`gt911`          | 自动显示 LVGL demo，通过触摸操作 |
| `Gino_display_camera`    | OV7670 LCD 预览               | `ov7670`                  | `ov7670_preview start`         |
| `Gino_component_mqtt`    | Wi-Fi 上的 SAL 与 kawaii-mqtt | `wifi0`                   | `gd32vw553_mqtt_start`         |
| `Gino_factory`           | 综合开发板演示                | 多个设备                    | 综合触摸界面、`ifconfig`       |

每个工程包含独立配置、Studio 元数据、MDK5 工程文件、应用代码和 README。工程 README 介绍硬件、设备接口、示例操作和预期行为。

## RT-Thread Studio

1. 打开 RT-Thread Studio，安装 GD32H77D-Gino **v1.0.0** 开发板支持包。
2. 选择“文件 -> 新建 -> RT-Thread 项目 -> 基于开发板”，选择 `GD32H77D-Gino`，创建示例工程或模板工程。

   ![RT-Thread Studio 创建开发板工程](figures/rt-thread-studio-project.png)

3. 使用 GNU Arm Embedded 13.3 编译；连接 UART1 控制台，设置为 115200-8-N-1。
4. 连接 DAP-Link，选择目标 `GD32H77DIW` 和 ELF 模式，下载 `rtthread.elf`。

应用向量表位于 `0x08010000`，`0x08000000` 的启动入口用于兼容不同芯片版本。使用 ELF/AXF 或 Intel HEX 下载，以保留镜像的加载地址。

## MDK5 开发

需要先安装 [GigaDevice 器件支持包](tools/mdk_pack/GigaDevice.GD32H77x_78x_DFP.0.6.1.pack)。

为了避免 SDK 在持续更新中，每一个 `projects` 都创建一份 `rt-thread` 文件夹 和 `libraries` 文件夹导致的 SDK 越来越臃肿，所以这些通用文件夹被单独提取了出来。这样就会导致直接打开 `MDK` 的工程编译会提示缺少上述两个文件夹的文件，我们使用如下步骤解决这个问题：

1. 使用 [Env](https://club.rt-thread.org/ask/question/5699.html) 工具执行 mklinks.bat 命令，分别为 `rt-thread` 及 `libraries` 文件创建符号链接。
2. 查看目录下是否有 `rt-thread` 和 `libraries` 的文件夹图标。
3. 使用 [Env](https://club.rt-thread.org/ask/question/5699.html) 工具执行 scons --target=mdk5 更新 MDK5 工程文件。

Env 使用 2.0 或更新版本，[下载链接](https://www.rt-thread.org/download.html#download-rt-thread-env-tool)。MDK 使用 Arm Compiler 6，并安装 GigaDevice `GD32H77x_78x_DFP` 0.6.1。完整步骤见 [工程使用指南](docs/PROJECT_GUIDE_zh.md)。
