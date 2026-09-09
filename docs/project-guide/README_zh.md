# GD32H77D Gino SDK 使用指南

[English](README.md) | [SDK 概览](https://github.com/RT-Thread-Studio/sdk-bsp-gd32h77d-realthread-ginopi/blob/main/README_zh.md)

本文介绍 SDK 目录、工程选择、硬件连接、工程创建、编译下载和示例使用方法。

## 1. 平台与开发环境

- 开发板：GD32H77D Gino；MCU：GD32H77DIW，Arm Cortex-M7，最高 600 MHz。
- RT-Thread：5.3.0。
- RT-Thread Studio 工具链：GNU Arm Embedded 13.3。
- Env：2.0 或更新版本，并配置 GNU Arm Embedded 工具链。
- MDK：Keil MDK 5.43 或兼容版本、Arm Compiler 6、GigaDevice `GD32H77x_78x_DFP` 0.6.1 和 DAP-Link（CMSIS-DAP）。
- 下载接口：SWD；Studio 使用 DAP-Link/PyOCD，MDK 使用 DAP-Link（CMSIS-DAP）。
- 控制台：UART1，PA2/PA3，AF7，115200-8-N-1。
- 工程数量：16 个，包括 `Gino_template` 和 15 个示例。

### SDK 目录

各示例共享 RT-Thread 内核、GD32 库、板级驱动和离线软件包。以下路径相对于 SDK 根目录。

| 目录 | 内容 |
| --- | --- |
| `projects/<name>/applications` | 应用入口与示例代码 |
| `projects/<name>/board` | 工程板级配置与链接脚本 |
| `libraries/Board_Drivers` | 共享板级初始化与板载外设驱动 |
| `libraries/gd32_drivers` | GD32 外设的 RT-Thread 驱动 |
| `libraries/gd32-arm-*` | GD32 CMSIS、启动代码与外设固件库 |
| `rt-thread` | RT-Thread 内核、组件与构建工具 |
| `packages` | 示例使用的离线软件包 |

每个工程拥有独立的 `.config`、`rtconfig.h`、Studio 元数据和 MDK 工程文件。功能配置在所选工程中修改；共享驱动的修改会影响使用该驱动的其他工程。

首次编译或新建应用建议从 [Gino_template](../../projects/Gino_template/README_zh.md) 开始。验证外设时，按第 6 节选择对应的独立示例；体验开发板综合功能时，使用 [Gino_factory](../../projects/Gino_factory/README_zh.md)。

## 2. 硬件映射

| 外设 | 引脚或资源 | 说明 |
| --- | --- | --- |
| 运行 LED | PC4 | 每 500 ms 翻转 |
| UART1 | PA2/PA3，AF7 | 控制台 |
| UART4 | PB12/PB13，AF14 | GD32VW553 AT 模块 |
| I2C1 | PH4/PB11，AF4 | 独立 I2C 示例 |
| I2C2 | PA8/PC9，AF4 | OV7670 控制 |
| I2C3 | PD12/PD13，AF4 | GT911 触摸 |
| SPI3 | PE12/PF0/PF1 | SCK/MISO/MOSI |
| CAN1 | PB4/PB5 | 需要外接 CAN 收发器和终端电阻 |
| LCD 背光 | TIMER1 CH0，PA0 | `pwm1` 通道 1 |
| PWM 输出 | PA7 与 PG7 | `pwm2` 通道 2 和 `pwm30` 通道 3 |
| GT911 | reset PH3，interrupt PH5 | 地址 `0x5D` |
| OV7670 | XCLK PE9，I2C2，DCI + DMA1 通道 7 | QVGA RGB565 |
| QSPI Flash | CS PC11，CLK PF10，D0-D3 PF8/PF9/PF7/PA1 | 8 MiB GD25Q64E，连接 OSPI0 |
| USB Device | USBHS0，PA11/PA12 | Type-C |
| USB Host | USBHS1，PB14/PB15，VBUS PF2 | USB-A |
| 以太网 | ENET1 RMII | PHY 地址 2，PC12 输入 50 MHz REF_CLK |
| SDRAM | `0xC0000000`，32 MiB | 显示缓冲和扩展 heap |

外接模块的供电、共地和接口电平要求，以对应工程 README 的硬件说明为准。

## 3. 内存与下载布局

| 区域 | 地址 | 大小 | 用途 |
| --- | --- | --- | --- |
| CNVM | `0x08010000` | 1984 KiB | 应用向量表、启动代码和主程序 |
| ECNVM | `0x08200000` | 7680 KiB | 扩展程序 Flash，由链接脚本分配 |
| ITCMRAM | `0x00000000` | 128 KiB | 按链接脚本分配 |
| DTCMRAM | `0x20000000` | 256 KiB | 按链接脚本分配 |
| AXISRAM | `0x24000000` | 768 KiB | data、BSS、栈、内部 heap 和摄像头缓冲 |
| SRAM1 | `0x30004000` | 16 KiB | 以太网 DMA 描述符和缓冲 |
| SDRAM | `0xC0000000` | 32 MiB | 显示缓冲和扩展 heap |

应用向量表位于 `0x08010000`，`0x08000000` 的启动入口用于兼容不同芯片版本。下载时使用 ELF/AXF 或 Intel HEX，以保留镜像的加载地址。

启用 `BSP_USING_LCD_MIPI` 时，SDRAM 前 3 MiB 为预留区，扩展 heap 从 `0xC0300000` 开始；未启用 LCD 时，SDRAM heap 从 `0xC0000000` 开始。摄像头采集和预览缓冲使用内部 AXI SRAM，与 SDRAM 显示区域分开管理。

## 4. RT-Thread Studio

1. 安装 `sdk-bsp-gd32h77d-gino.yaml` 描述的 GD32H77D-Gino BSP **v1.0.0**。
2. 新建基于 `GD32H77D-Gino` 开发板的 RT-Thread 工程。
3. 选择 `Gino_template` 或 15 个示例中的一个。
4. 在 RT-Thread Settings 中配置需要的外设和软件包，使用 GNU Arm Embedded 13.3 构建。
5. 连接 UART1，设置为 115200-8-N-1，并连接 SWD 下载器。
6. 选择 DAP-Link、目标 `GD32H77DIW` 和 ELF 模式，下载 `rtthread.elf`；也可使用 Intel HEX。
7. 复位开发板，按下文运行对应示例。

Studio 创建出的工程包含该示例需要的共享源码目录。

## 5. Env 与 MDK

### 5.1 Env 与 GCC

在 Env PowerShell 环境中，从 SDK 根目录进入目标工程。以模板为例：

```powershell
cd projects\Gino_template
.\mklinks.bat
$env:RTT_CC = "gcc"
$env:RTT_EXEC_PATH = "<gnu-arm-toolchain>\bin"
scons --pyconfig-silent
scons -j8
```

将工具链路径替换为包含 `arm-none-eabi-gcc` 的目录。`mklinks.bat` 为共享的 `rt-thread` 和 `libraries` 创建目录链接。SCons 构建生成 `rt-thread.elf`、`rtthread.hex` 和 `rtthread.bin`，下载时使用 ELF 或 HEX。

通过 `menuconfig` 修改功能后，使用 `scons --pyconfig-silent` 重新生成 `rtconfig.h`。需要重新生成 Eclipse 工程元数据时，执行 `scons --target=eclipse`。

### 5.2 MDK5 与 Arm Compiler 6

安装第 1 节列出的 MDK 依赖，在 Env PowerShell 环境中从 SDK 根目录进入目标工程：

```powershell
cd projects\Gino_template
.\mklinks.bat
$env:RTT_CC = "keil"
$env:RTT_EXEC_PATH = "C:\Keil_v5"
scons --pyconfig-silent
scons --target=mdk5 --project-name=project
```

将 `RTT_EXEC_PATH` 调整为实际 MDK 安装根目录。打开 `project.uvprojx`，构建与工程目录同名的 target。在 `Options for Target -> Debug` 中选择 `CMSIS-DAP Debugger`，在 `Settings` 中选择连接的 DAP-Link 和 SWD 接口，通过 DAP-Link 下载 `Objects/rt-thread.axf` 或 `Objects/rt-thread.hex`。工程使用 `board/linker_scripts/link.sct`，以及 `GD32H77x_78x_CNVM_2M.FLM` 和 `GD32H77x_78x_ECNVM_7M_512K.FLM` 两个 Flash 算法。

`template.uvprojx` 和 `template.uvoptx` 保存 uVision 默认配置。SCons 根据当前配置生成源码分组、包含路径和宏定义。配置或源码选择变更后需要重新生成 uVision 工程，生成的源码分组会被覆盖。

## 6. 示例使用

各示例的接线、配置、命令和预期现象见侧栏中的对应工程文档。下表汇总运行前置条件和首次操作。

| 工程 | 前置条件 | 操作或预期行为 |
| --- | --- | --- |
| `Gino_template` | 开发板供电 | PC4 闪烁，UART1 控制台可用 |
| `Gino_driver_i2c` | 上拉电阻和 I2C 目标设备 | `i2c scan hwi2c1 08 78` |
| `Gino_driver_spi` | SPI 目标设备及应用配置的 CS | 使用 `spi3` 总线，在应用中配置从设备协议 |
| `Gino_driver_qspi_flash` | 板载 GD25Q64E | `ls /flash` 访问文件 |
| `Gino_driver_sdcard` | FAT/FAT32 SD 卡 | `ls /sd` |
| `Gino_driver_can` | CAN 收发器、终端电阻和通信对端 | 使用 `can1`，波特率和帧格式与对端一致 |
| `Gino_driver_pwm` | 按需连接输出端 | `pwm_dual_test start`，PA7/PG7 输出 PWM |
| `Gino_driver_rtc` | RTC 时钟源 | `date` |
| `Gino_driver_sdram` | 板载 SDRAM | 外部内存加入 heap；`free`、`list_memheap` |
| `Gino_driver_usb_device` | 支持数据传输的 USB 线 | 主机枚举配置的 USB class |
| `Gino_driver_usb_host` | FAT U 盘 | `ls /udisk` |
| `Gino_driver_eth` | 网线及可用网络 | `ifconfig`、`ping <gateway>` |
| `Gino_display_lvgl` | LCD、触摸和 SDRAM | 自动启动 LVGL demo，通过触摸操作 |
| `Gino_display_camera` | OV7670、LCD 和 SDRAM | 自动预览；`ov7670_preview start` / `ov7670_preview stop` |
| `Gino_component_mqtt` | 已配置 Wi-Fi 和 broker | `gd32vw553_mqtt_start`、`gd32vw553_mqtt_pub <topic> <message>` |
| `Gino_factory` | 已启用的板级外设 | 使用综合界面及摄像头、存储和网络示例 |

### Flash 文件系统

`filesystem` FAL 分区挂载到 `/flash`。对于需要格式化的新分区：

```text
mkfs -t elm filesystem
mount filesystem /flash elm
ls /flash
```

格式化会清除分区原有数据。FatFs 需要允许最大 4096 字节扇区。

### Wi-Fi 与 MQTT

在 RT-Thread Settings 或 `menuconfig` 中配置 Wi-Fi SSID/password、broker 地址/端口、topic、client ID 和可选认证信息。真实凭据保存在本地配置中。当模块固件不支持 `AT+CIPDOMAIN` DNS 查询时，使用数字 IPv4 broker 地址。

### 显示与摄像头

`Gino_display_lvgl` 自动启动 LVGL demo。`Gino_display_camera` 在 LVGL 初始化后请求预览，显示 QVGA RGB565 图像。使用 `ov7670_preview stop` 和 `ov7670_preview start` 控制预览。LCD 帧缓冲位于 SDRAM 预留区，应用动态内存应从配置的 heap 分配。
