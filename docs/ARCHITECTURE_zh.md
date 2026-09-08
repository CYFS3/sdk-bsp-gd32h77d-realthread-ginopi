# GD32H77D Gino BSP 架构与工作原理

[English](ARCHITECTURE.md) | [工程使用指南](PROJECT_GUIDE_zh.md) | [SDK 概览](../README_zh.md)

本文面向应用开发，介绍源码组织、启动流程、配置构建、设备接口和外设数据路径。

## 1. 源码组织

SDK 提供 16 个工程，共享 RT-Thread、GD32 库、板级驱动和离线软件包。

| 目录 | 职责 |
| --- | --- |
| `projects/<name>/applications` | 示例入口、应用线程和组件适配 |
| `projects/<name>/board` | 工程 Kconfig、板级构建入口和链接脚本 |
| `libraries/Board_Drivers` | 共享板级初始化、LCD、触摸、摄像头、以太网和文件系统集成 |
| `libraries/gd32_drivers` | RT-Thread 外设驱动和通用外设配置 |
| `libraries/gd32-arm-cmsis-latest` | CMSIS、启动代码和系统初始化 |
| `libraries/gd32-arm-series-latest` | GD32 外设固件库 |
| `rt-thread` | 内核、设备框架、组件和构建工具 |
| `packages` | 离线 LVGL、GT911、at_device 和 kawaii-mqtt 软件包 |

每个工程拥有自己的 `.config`、生成的 `rtconfig.h`、应用源码、Studio 元数据和 MDK 文件。工程的 `board/SConscript` 引入共享的 `libraries/Board_Drivers/SConscript`。二次开发可从 `Gino_template` 或最接近需求的外设示例开始；`Gino_factory` 将板级功能组合为综合应用。

## 2. 启动与内存

启动汇编调用 `SystemInit`，准备 C 运行环境并进入 RT-Thread。应用向量表位于 `0x08010000`，`0x08000000` 的启动入口用于兼容不同芯片版本。内存映射和下载格式见工程使用指南。

共享的 `libraries/Board_Drivers/board.c` 按以下顺序完成板级初始化：

1. 设置应用向量表，开启 I-Cache。
2. 配置 SysTick 和 UART，执行板级初始化，包括已启用的 SDRAM。
3. 设置控制台；启用 LCD 时配置显示内存 MPU 区域。
4. 开启 D-Cache，初始化内部 heap。

进入板级初始化时要求 D-Cache 处于关闭状态。通过 bootloader 跳转时，由 bootloader 在交接前清理并关闭 D-Cache。LCD 的使用依赖 SDRAM 成功初始化。

RT-Thread 初始化调度器和系统线程，随后在主线程中执行组件初始化。`INIT_*_EXPORT()` 将初始化入口放入 `.rti_fn.<level>` 段：板级为 `1`，设备为 `3`，组件为 `4`，环境为 `5`。设备注册先于依赖它的应用操作、文件系统挂载和 LVGL 访问。

## 3. 配置与构建

构建使用以下输入：

- `Kconfig` 和 `.config`：选择功能。
- `rtconfig.h`：为 C 和汇编源码生成配置宏。
- `SConstruct` 和 `SConscript`：组织源码分组、依赖、包含路径和编译选项。
- `board/linker_scripts/link.ld`：GCC 内存与段布局。
- `board/linker_scripts/link.sct`：MDK 内存与段布局。
- `template.uvprojx` 和 `template.uvoptx`：uVision 工程及下载默认配置。

通过 RT-Thread Settings 或 `menuconfig` 修改配置，然后重新生成 `rtconfig.h`。`scons --target=eclipse` 导出 Studio/Eclipse 元数据；`scons --target=mdk5` 将当前构建图导出为 uVision 工程。生成的源码分组应通过 SCons 输入维护。

GCC 启动源码为 `startup_gd32h77x_78x.S`，大写 `.S` 表示汇编前需要预处理，可引用 `rtconfig.h`。Eclipse 元数据为 C、C++ 和汇编分别提供包含路径，并通过 `rtconfig_preinc.h` 提供 RT-Thread 内部宏定义。

## 4. 设备接口

应用通过名称查找 RT-Thread 注册设备，使用 `rt_device_open` 打开，并按设备类别调用读、写或控制接口。

| 功能 | 设备或接口 |
| --- | --- |
| 控制台 | `uart1` |
| I2C 示例 | `hwi2c1` |
| SPI 示例 | `spi3` |
| CAN | `can1` |
| LCD 与触摸 | `lcd`、`gt911` |
| 摄像头 | `ov7670` |
| 以太网与 Wi-Fi | `e0`、`wifi0` 网络设备和 SAL socket |
| 存储 | `qspi_flash0`、`sd0` 和 DFS 挂载点 |

LVGL RT-Thread port 创建独立的 `LVGL` 线程，初始化 LVGL、显示、输入和应用界面，并循环调用 `lv_timer_handler`。LVGL 对象操作应在该线程内完成，或遵循 port 的同步规则。中断处理用于通知线程或完成外设传输，应用处理在线程上下文中执行。

## 5. Cache、DMA 与缓冲

Cortex-M7 CPU 与 DMA 共享内存，缓冲需要按 Cache line 对齐，并明确使用权：

- CPU 写入的数据交给硬件读取前，清理对应 Cache line。
- DMA 采集前准备目标缓冲，CPU 读取采集结果前使对应 Cache line 失效。
- 硬件传输或显示读取完成后，才允许复用缓冲。

| 缓冲 | 位置与用途 |
| --- | --- |
| LCD 帧缓冲 | 从 `0xC0000000` 开始的 SDRAM 预留区 |
| LVGL 局部绘制缓冲 | 内部 AXI SRAM |
| OV7670 采集与预览缓冲 | 内部 AXI SRAM，32 字节对齐 |
| 以太网 DMA 描述符与缓冲 | SRAM1 的 `.enet_dma` 段，起始地址 `0x30004000` |
| 启用 LCD 时的扩展 heap | 从 `0xC0300000` 开始的 SDRAM |

板级初始化在开启 D-Cache 前配置 MPU 属性，保护未使用的外部地址窗口，并通过更高优先级的区域开放有效 SDRAM。OSPI0 Flash 使用间接传输；应用新增内存映射外部设备时，需要为其配置可访问的 MPU 区域。

SDRAM 前 3 MiB 预留区与 heap 分开管理。`board.c` 根据 LCD/摄像头配置选择显示内存属性，扩展应用时应保留这些区域边界。

## 6. 外设数据路径

### 显示与触摸

LVGL 生成 RGB565 像素，IPA 将绘制区域复制到 SDRAM 帧缓冲，TLI/DSI 驱动 720 x 720 FL7707N 面板。GT911 通过 I2C3 上报触摸，经 RT-Thread touch 设备传递给 LVGL。

`Gino_display_lvgl` 使用两个 200 行局部绘制缓冲，通过异步 IPA 复制到单个扫描帧缓冲。启用摄像头的显示及综合工程使用 32 行绘制缓冲，由 IPA 更新后台帧缓冲，并在垂直消隐期交换帧缓冲。flush 回调在传输完成后才释放 LVGL 缓冲。

### 摄像头

I2C2/SCCB 配置 OV7670，TIMER0 提供 XCLK。DCI 接收并行像素，DMA1 通道 7 写入内部 AXI SRAM 采集缓冲。驱动完成 Cache 维护和 RGB565 帧数据处理后，通过 `rt_device_read` 返回图像。

预览应用使用采集线程和两个内部预览缓冲，跟踪正在写入、就绪和正在显示的缓冲，保证 LVGL 在渲染完成前持有稳定的图像源。图像随后经 LCD 路径显示。`Gino_display_camera` 在 LVGL 初始化后启动预览。

### 以太网与 Wi-Fi

以太网数据经过 SAL socket、lwIP、RT-Thread 以太网设备、ENET DMA 和 RMII PHY。驱动在 lwIP pbuf 与 DMA 缓冲之间传递帧，并在中断处理函数中通知接收线程。

Wi-Fi 数据经过 SAL socket、AT socket、at_device/at_client、UART4 和 GD32VW553。模块负责 Wi-Fi 和 TCP/IP，H7 负责发送 AT 命令并处理响应和异步事件。

### 存储

OSPI0 访问 GD25Q64E NOR Flash。FAL 提供 Flash 设备及分区，块设备适配层与 DFS/FatFs 提供 `/flash` 文件系统。SD 和 USB 大容量存储提供块设备，分别挂载到 `/sd` 和 `/udisk`。文件系统挂载在设备初始化之后执行，存储介质需要使用受支持的文件系统。

### MQTT

MQTT 示例等待 `wifi0`，临时将其选为 SAL 默认网络设备，通过 kawaii-mqtt 完成连接、订阅和发布。Wi-Fi 与 broker 参数来自工程配置。示例使用 QoS 0，不提供投递确认或重试保证。

## 7. 综合应用

`Gino_factory` 组合 LVGL、摄像头、存储、以太网、Wi-Fi/MQTT 和常用外设。启用的功能共享内存、总线、中断资源和网络路由。各设备配置应与对应独立示例保持一致，保留缓冲使用权管理，并为 socket 通信选择目标网络设备。

## 8. 代码地图

以下路径相对于 SDK 根目录。通过 Studio 创建工程后，所选共享目录会复制到工程中。

| 主题 | 起点 |
| --- | --- |
| GCC 启动 | `libraries/gd32-arm-cmsis-latest/GD32H77x_78x/GD/GD32H77x_78x/Source/GCC/startup_gd32h77x_78x.S` |
| 板级初始化与内存定义 | `libraries/Board_Drivers/board.c`、`libraries/Board_Drivers/board.h` |
| RT-Thread 初始化 | `rt-thread/src/components.c`、`rt-thread/include/rtdef.h` |
| 构建 | `projects/Gino_template/SConstruct`、`rt-thread/tools/building.py` |
| LCD 与触摸 | `libraries/Board_Drivers/drv_lcd.c`、`libraries/Board_Drivers/drv_gt911.c` |
| LVGL | `projects/Gino_display_lvgl/applications/lv_port.c`、`packages/LVGL-latest/env_support/rt-thread/lv_rt_thread_port.c` |
| 摄像头 | `libraries/Board_Drivers/drv_ov7670.c`、`projects/Gino_display_camera/applications/ov7670_preview.c` |
| 以太网 | `libraries/Board_Drivers/drv_enet.c` |
| AT Wi-Fi | `projects/Gino_component_mqtt/applications/gd32vw553_at_port.c`、`packages/at_device-latest/class/gd32vw553/` |
| MQTT | `projects/Gino_component_mqtt/applications/gd32vw553_mqtt_demo.c` |
| 综合界面 | `projects/Gino_factory/applications/board_demo.c`、`projects/Gino_factory/applications/board_demo_backend.c` |
