# Gino_template

## 1. 简介

本工程是 GD32H77D Gino 开发板的 LED 闪烁与二次开发模板，用于单独学习、配置和验证 GPIO 与 RTOS 线程调度。独立工程只启用该功能需要的驱动和组件，可作为应用开发和功能集成的参考。

本工程没有独立的主设备节点。

所有示例均保留 `uart1` 作为 FinSH/MSH 控制台，串口参数为 115200-8-N-1；PC4 LED 用于运行指示。

当前工程包含以下 MSH 命令：

| 命令 | 作用 | 示例/备注 |
| --- | --- | --- |
| `gino_device_probe` | 检查当前示例使用的运行探测状态。 | 本模板没有独立的主参考设备节点。 |
| `pin list` | 查看 PIN 设备信息。 | 确认 GPIO/PIN 注册。 |

## 2. GPIO 与 RTOS 线程调度详解

启动汇编完成数据段初始化后，RT-Thread 配置时钟、控制台、系统 tick 和调度器，再进入应用 `main`。应用通过 PIN 设备接口驱动 PC4，并用 `rt_thread_mdelay` 主动让出 CPU；所以 LED 闪烁同时验证了 GPIO、系统节拍和线程调度。

一次完整的数据路径如下：

```text
复位 -> board 初始化 -> RT-Thread 调度器 -> main 线程 -> rt_pin_write -> PC4 LED
```

协议层只定义通信或数据处理规则，最终仍需要控制器时钟、引脚复用、中断/DMA 和上层状态机共同工作。扫描到设备、注册了总线或编译通过，都不能替代完整的数据传输验证。

## 3. GD32H77D 的 GPIO 与 SysTick 特性

GD32H77D 的 GPIO 支持输入、推挽/开漏输出和复用功能。板载运行指示灯连接 PC4；BSP 在 `board.c` 中完成系统时钟、SysTick、PIN 驱动和 UART1 控制台初始化，因此 PC4 翻转也能间接验证系统节拍与主线程调度。

## 4. RT-Thread PIN 与线程接口

RT-Thread PIN 接口使用 `rt_pin_mode` 配置方向，使用 `rt_pin_write`/`rt_pin_read` 操作电平。示例在线程中调用 `rt_thread_mdelay(500)` 让出处理器，而不是用空循环阻塞 CPU。

本工程没有独立的主参考设备节点。可先通过 `list_device` 和 `gino_device_probe` 判断运行状态；更上层的文件系统、网络或 GUI 组件还需继续验证其挂载、链路或刷新状态。

## 5. 硬件说明

无需额外硬件，使用 PC4 LED 验证系统节拍、GPIO 和基础启动。

默认控制台：UART1，PA2/PA3，AF7，115200-8-N-1。

## 6. 工程示例说明

以下源码路径以 SDK 仓库中的工程目录为基准：

- `applications/main.c`
- `applications/device_probe.c`
- `../../libraries/Board_Drivers/board.c`
- `board/SConscript`

建议先阅读 `applications/main.c` 和 `applications/device_probe.c`，再沿数据路径进入对应驱动、组件或软件包。示例保留 MSH 命令，便于在不改动应用代码的情况下观察设备注册和运行状态。

### 6.1 运行命令

- `gino_device_probe`
- `pin list`

### 6.2 运行步骤

1. 检查供电、接线、外部模块和接口电平。
2. 复位开发板，确认 UART1 控制台可用且 PC4 LED 正常闪烁。
3. 执行 `list_device`，确认依赖总线和目标设备均已注册。
4. 按顺序执行上述命令，并同时观察返回值、外部波形、网络状态或显示结果。

## 7. 运行效果

### 7.1 预期现象

- UART1 输出 RT-Thread 启动信息和工程名称。
- PC4 LED 每 500 ms 翻转一次，完整亮灭周期约 1 s。
- `pin list` 能显示 PIN 设备和当前引脚信息。
