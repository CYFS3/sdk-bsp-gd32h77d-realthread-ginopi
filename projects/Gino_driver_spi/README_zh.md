# Gino_driver_spi

## 1. 简介

本工程是 GD32H77D Gino 开发板的 SPI3 总线参考工程，用于单独学习、配置和验证 SPI 同步串行协议。独立工程只启用该功能需要的驱动和组件，可作为应用开发和功能集成的参考。

主参考设备：`spi3`。

所有示例均保留 `uart1` 作为 FinSH/MSH 控制台，串口参数为 115200-8-N-1；PC4 LED 用于运行指示。

当前工程包含以下 MSH 命令：

| 命令 | 作用 | 示例/备注 |
| --- | --- | --- |
| `gino_device_probe` | 检查当前示例使用的主要设备是否已注册。 | 默认检查 `spi3`。 |
| `spi_loopback [length] [count]` | 执行 SPI3 全双工回环测试，逐字节比较收发数据。 | `spi_loopback` 默认每轮 32 字节、测试 10 轮；`spi_loopback 256 100` 测试 100 轮，每轮 256 字节。 |
| `list_device` | 查看系统已注册设备。 | 确认 UART、PIN、总线和目标设备存在。 |

## 2. SPI 同步串行协议详解

SPI 由主机产生 SCK，并通过 MOSI/MISO 全双工移位；每个从设备还需要独立片选。当前工程支持验证 `spi3` 总线注册和外部回环收发。回环命令使用 8 位数据、Mode 0、MSB 优先，请求最大时钟为 1 MHz，不使用片选。

一次完整的数据路径如下：

```text
应用 -> RT-Thread SPI bus -> SPI3 -> SCK/MOSI/MISO -> 外部从设备
```

协议层只定义通信或数据处理规则，最终仍需要控制器时钟、引脚复用、中断/DMA 和上层状态机共同工作。扫描到设备、注册了总线或编译通过，都不能替代完整的数据传输验证。

## 3. GD32H77D 的 SPI3 特性

工程启用 GD32H77D SPI3，默认使用 PE12/PF0/PF1 作为时钟和数据信号。硬件控制器支持可配置的时钟分频、CPOL/CPHA 和数据宽度；片选通常由附加的 SPI device 或 GPIO 管理。

## 4. RT-Thread SPI 总线设备接口

RT-Thread 将控制器注册为 `spi3` 总线。具体从设备需通过 `rt_spi_bus_attach_device` 或 BSP 封装附加，然后使用 `rt_spi_configure`、`rt_spi_transfer`、`rt_spi_send`/`rt_spi_recv`。

主参考设备：`spi3`。设备是否已注册可先通过 `list_device` 和 `gino_device_probe` 判断；更上层的文件系统、网络或 GUI 组件还需继续验证其挂载、链路或刷新状态。

## 5. 硬件说明

SPI3 默认使用 PE12/PF0/PF1；具体从设备的片选、模式和时钟由应用配置。

外部回环测试前，先断开外部 SPI 从设备，再将 **PF1（MOSI）** 与 **PF0（MISO）** 直接短接。**PE12（SCK）** 无需短接，可接逻辑分析仪观察时钟；无需连接 CS。如果修改过 SPI3 引脚映射，请按实际配置接线。

默认控制台：UART1，PA2/PA3，AF7，115200-8-N-1。

SPI 总线注册后空闲时不会产生波形，需要具体 SPI 设备发起传输。

## 6. 工程示例说明

以下源码路径以 SDK 仓库中的工程目录为基准：

- `applications/main.c`
- `applications/device_probe.c`
- `../../libraries/gd32_drivers/drv_spi.c`
- `../../libraries/gd32_drivers/config/spi_config.h`

建议先阅读 `applications/main.c` 和 `applications/device_probe.c`，再沿数据路径进入对应驱动、组件或软件包。示例保留 MSH 命令，便于在不改动应用代码的情况下观察设备注册和运行状态。

### 6.1 运行命令

- `gino_device_probe`
- `list_device`
- `spi_loopback`：每轮 32 字节，测试 10 轮。
- `spi_loopback 256 100`：每轮 256 字节，测试 100 轮。
- `spi_loopback --help`：查看参数范围和接线说明。

`length` 范围为 1-256 字节，`count` 范围为 1-1000 轮，均使用十进制。命令首次运行时将 `spi3_loop` 设备挂载到 `spi3`，后续运行复用该设备。每轮发送变化的数据并同步接收，检查实际传输长度，再逐字节比较。当前 BSP 使用轮询收发，实际 SCK 频率由外设时钟和分频系数决定。

### 6.2 运行步骤

1. 断开外部 SPI 从设备，将 PF1（MOSI）与 PF0（MISO）短接。
2. 复位开发板，确认 UART1 控制台可用且 PC4 LED 正常闪烁。
3. 执行 `list_device`，确认依赖总线和目标设备均已注册。
4. 执行 `gino_device_probe`，再运行 `spi_loopback`。默认测试通过时输出 `SPI loopback PASS: 10 rounds, 320 bytes checked.`。
5. 执行 `spi_loopback 256 100` 进行多轮测试。失败时根据输出的传输长度，或轮次、从 0 开始的字节偏移及 TX/RX 值，检查接线和引脚配置。

## 7. 运行效果

### 7.1 预期现象

运行日志使用 ulog，显示时间、级别和标签：启动日志为 `spi.demo`，设备探测和回环测试为 `spi.test`。支持 ANSI 颜色的串口终端中，INFO/PASS 显示绿色，参数错误的 WARN 显示黄色，ERROR/FAIL 显示红色；命令帮助通过 ulog 原样输出。

- `list_device` 中出现 `spi3`。
- `gino_device_probe` 能找到 SPI3 总线。
- `spi_loopback` 运行时产生时钟和数据波形，收发数据全部一致时输出 `PASS`。
- 未接回环线或接线错误时，通常输出 `FAIL` 和首个不匹配字节。总线注册成功不能替代收发验证；当前驱动的轮询等待尚无超时，控制器或时钟异常时命令可能阻塞。
