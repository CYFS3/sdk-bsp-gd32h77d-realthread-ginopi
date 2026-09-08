# Gino_driver_i2c

## 1. 简介

本工程是 GD32H77D Gino 开发板的 I2C1 硬件总线参考工程，用于单独学习、配置和验证 I2C 多主从总线协议。独立工程只启用该功能需要的驱动和组件，可作为应用开发和功能集成的参考。

主参考设备：`hwi2c1`。

所有示例均保留 `uart1` 作为 FinSH/MSH 控制台，串口参数为 115200-8-N-1；PC4 LED 用于运行指示。

当前工程包含以下 MSH 命令：

| 命令                  | 作用                                     | 示例/备注                                                 |
| --------------------- | ---------------------------------------- | --------------------------------------------------------- |
| `gino_device_probe` | 检查当前示例使用的主要设备是否已注册。   | 默认检查`hwi2c1`。                                      |
| `i2c scan hwi2c1`   | 扫描 I2C1 总线并打印有响应的从设备地址。 | 由`i2c-tools` 软件包提供，连接已供电的 I2C 目标后使用。 |

工程已启用 `i2c-tools v1.0.0`（`PKG_USING_I2C_TOOLS`），使用硬件总线模式。打开工程前，Windows 执行 `mklinks.bat`，Linux 执行 `sh mklinks.sh`，建立仓库内软件包的目录链接。执行 `i2c` 可查看扫描、读写命令用法。

## 2. I2C 多主从总线协议详解

I2C 使用开漏 SDA/SCL 和上拉电阻形成可共享总线。控制器产生 START、地址、ACK、数据和 STOP；`i2c scan` 使用零数据长度的写传输，通过地址阶段是否收到 ACK 判断器件存在，不验证器件寄存器功能。

一次完整的数据路径如下：

```text
MSH i2c scan -> i2c-tools -> hwi2c1 -> I2C1 控制器 -> PH4/PB11 -> 从设备 ACK
```

协议层只定义通信或数据处理规则，最终仍需要控制器时钟、引脚复用、中断/DMA 和上层状态机共同工作。扫描到设备、注册了总线或编译通过，都不能替代完整的数据传输验证。

## 3. GD32H77D 的 I2C1 特性

工程使用 GD32H77D 硬件 I2C1，PH4/PB11 复用为 AF4。控制器负责 START/STOP、地址、ACK/NACK 和字节时序；SDA/SCL 仍必须依靠外部或板载上拉形成高电平。

## 4. RT-Thread I2C 总线设备接口

RT-Thread 把控制器注册为 `hwi2c1` 总线。器件驱动可调用 `rt_i2c_transfer`，或使用 `rt_i2c_master_send`/`rt_i2c_master_recv`；`i2c scan hwi2c1` 通过地址 ACK 扫描器件。

主参考设备：`hwi2c1`。设备是否已注册可先通过 `list_device` 和 `gino_device_probe` 判断；更上层的文件系统、网络或 GUI 组件还需继续验证其挂载、链路或刷新状态。

## 5. 硬件说明

I2C1 默认使用 PH4/PB11（AF4）。外部总线需要合适的上拉电阻。

默认控制台：UART1，PA2/PA3，AF7，115200-8-N-1。

外部总线需要合适的上拉电阻，并与目标设备共地。

![1788863921749](figures/1788863921749.png)

## 6. 工程示例说明

以下源码路径以 SDK 仓库中的工程目录为基准：

- `applications/main.c`
- `applications/device_probe.c`
- `../../libraries/gd32_drivers/drv_hard_i2c.c`
- `../../libraries/gd32_drivers/config/i2c_config.h`
- `../../packages/i2c-tools-v1.0.0/src/i2c_tools.c`
- `../../packages/i2c-tools-v1.0.0/src/i2c_utils.c`

建议先阅读 `applications/main.c` 和 `applications/device_probe.c`，再沿数据路径进入对应驱动、组件或软件包。示例保留 MSH 命令，便于在不改动应用代码的情况下观察设备注册和运行状态。

### 6.1 运行命令

- `gino_device_probe`
- `i2c scan hwi2c1 08 78`

扫描范围按十六进制解析，结束地址不包含在内：`08 78` 扫描 `0x08-0x77`，跳过保留地址。不指定范围时，软件包扫描 `0x00-0x7F`。`gino_device_probe` 只检查总线注册，不探测从设备。

### 6.2 运行步骤

1. 检查供电、接线、外部模块和接口电平。
2. 复位开发板，确认 UART1 控制台可用且 PC4 LED 正常闪烁。
3. 执行 `list_device`，确认依赖总线和目标设备均已注册。
4. 按顺序执行上述命令，并同时观察返回值、外部波形、网络状态或显示结果。

## 7. 运行效果

### 7.1 预期现象

- `list_device` 中出现 `hwi2c1`。
- 接入已知地址器件后，`i2c scan hwi2c1 08 78` 能列出响应地址。
- 逻辑分析仪可看到 START、7 位地址、ACK 和 STOP。
