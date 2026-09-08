# Gino_driver_usb_device

## 1. 简介

本工程是 GD32H77D Gino 开发板的 USBHS0 Device 参考工程，用于单独学习、配置和验证 USB Device 枚举与 CDC 类。独立工程只启用该功能需要的驱动和组件，可作为应用开发和功能集成的参考。

主参考设备：`usbd`。

CDC 数据设备：`vcom`。电脑打开 USB 虚拟串口并启用 DTR 后，应用每秒发送一条带序号的测试信息，并将收到的数据原样回显。

所有示例均保留 `uart1` 作为 FinSH/MSH 控制台，串口参数为 115200-8-N-1；PC4 LED 用于运行指示。

当前工程包含以下 MSH 命令：

| 命令 | 作用 | 示例/备注 |
| --- | --- | --- |
| `gino_device_probe` | 检查当前示例使用的主要设备是否已注册。 | 默认检查 `usbd`。 |
| `list_device` | 查看系统已注册设备。 | 确认 UART、PIN、总线和目标设备存在。 |

## 2. USB Device 枚举与 CDC 类详解

USBHS0 工作在 Device 角色，连接 PC 后响应总线复位和枚举请求，返回设备、配置、接口和端点描述符。当前配置启用 RT-Thread USB Device 与 CDC 组合类，PC 根据描述符加载相应驱动。

一次完整的数据路径如下：

```text
PC Host -> Type-C D+/D- -> USBHS0 Device -> RT-Thread USB Device/CDC -> 应用端点
```

协议层只定义通信或数据处理规则，最终仍需要控制器时钟、引脚复用、中断/DMA 和上层状态机共同工作。扫描到设备、注册了总线或编译通过，都不能替代完整的数据传输验证。

## 3. GD32H77D 的 USBHS0 Device 特性

工程将 GD32H77D USBHS0 配置为 Device，使用 Type-C 接口 PA11/PA12。控制器响应 Host 的复位和端点事务；当前 RT-Thread 配置启用 USB Device、Composite 和 CDC 类。

## 4. RT-Thread USB Device 设备接口

RT-Thread USB Device 栈通过描述符定义设备、配置、接口和端点，并将类请求分派给 CDC 功能。该栈不是普通字符 `rt_device` 的简单替代，应用通常通过 USB 类回调或类设备接口收发数据。

底层控制器注册为 `usbd`，CDC 串口接口注册为 `vcom`。应用通过 `rt_device_open()` 打开 `vcom`，使用 `rt_device_read()` / `rt_device_write()` 接收和回显数据，并在发送前通过 `RT_USBD_CLASS_CTRL_CONNECTED` 检查连接状态。该状态由电脑串口工具设置的 DTR 信号决定。`rt_kprintf()` 和 FinSH/MSH 仍使用 `uart1`。

## 5. 硬件说明

USBHS0 Device 使用 Type-C 接口和 PA11/PA12。请使用支持数据传输的 USB 线。

默认控制台：UART1，PA2/PA3，AF7，115200-8-N-1。

使用可传输数据的 USB 线，并确认 PC 发生枚举，而不是只给开发板供电。

## 6. 工程示例说明

以下源码路径以 SDK 仓库中的工程目录为基准：

- `../../libraries/gd32_drivers/usb/drv_otghs_dev.c`
- `../../libraries/gd32_drivers/usb/drv_usb_dev.c`
- `../../libraries/gd32_drivers/usb/gd32h7_usb_hw.c`
- `rtconfig.h`

建议先阅读 `applications/main.c` 和 `applications/device_probe.c`，再沿数据路径进入对应驱动、组件或软件包。示例保留 MSH 命令，便于在不改动应用代码的情况下观察设备注册和运行状态。

### 6.1 运行命令

- `gino_device_probe`
- `list_device`

### 6.2 运行步骤

1. 检查供电、接线、外部模块和接口电平。
2. 复位开发板，确认 UART1 控制台可用且 PC4 LED 正常闪烁。
3. 在 UART1 控制台执行 `list_device`，确认 `usbd` 和 `vcom` 均已注册。`gino_device_probe` 检查的是 `usbd` 控制器。
4. 将 USBHS0 Type-C 接口连接电脑，在串口工具中打开枚举出的 USB 虚拟串口，参数选 115200-8-N-1，并启用 DTR。
5. 确认每秒收到一条带序号的测试信息。发送 `hello\r\n` 等短字符串，确认收到原样回显；关闭电脑串口工具的本地回显，避免把本地显示误认为设备回复。
6. 关闭并重新打开 USB 虚拟串口，启用 DTR 后确认周期数据恢复。本示例用于低速通信验证，接收缓冲区为 128 字节，回显测试请使用短数据包。

## 7. 运行效果

### 7.1 预期现象

- 连接支持数据的 USB 线后，PC 能检测到新 USB/CDC 设备。
- 设备管理器中无反复连接、断开或未知设备。
- UART1 控制台保持可用，可用于观察枚举日志。
- 打开 USB 虚拟串口并启用 DTR 后，每秒收到一条数据：

```text
Gino USB CDC test: 0
Gino USB CDC test: 1
Gino USB CDC test: 2
```

- 电脑发送的数据会原样回显，周期测试信息与回显数据共用同一个串口。
- DTR 关闭或 USB 断开时暂停发送，重新打开串口并启用 DTR 后恢复。
