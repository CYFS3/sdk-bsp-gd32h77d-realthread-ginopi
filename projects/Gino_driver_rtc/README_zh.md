# Gino_driver_rtc

## 1. 简介

本工程是 GD32H77D Gino 开发板的 片上 RTC 与 Alarm 参考工程，用于单独学习、配置和验证 RTC 实时时钟与 Alarm。独立工程只启用该功能需要的驱动和组件，可作为应用开发和功能集成的参考。

主参考设备：`rtc`。

所有示例均保留 `uart1` 作为 FinSH/MSH 控制台，串口参数为 115200-8-N-1；PC4 LED 用于运行指示。

当前工程包含以下 MSH 命令：

| 命令 | 作用 | 示例/备注 |
| --- | --- | --- |
| `gino_device_probe` | 检查当前示例使用的主要设备是否已注册。 | 默认检查 `rtc`。 |
| `date` | 通过 MSH date 命令读取或设置 RTC 时间。 | 重复执行可确认秒计数递增。 |
| `rtc_alarm [秒数]` | 设置一次性 RTC 闹钟。 | 默认 5 秒，范围 1..86400 秒；再次设置会替换本示例的旧闹钟。 |
| `rtc_alarm status` | 查看本示例闹钟状态和目标时间。 | 时间按本地时区显示。 |
| `rtc_alarm stop` | 取消并删除本示例闹钟。 | 不删除其他应用创建的闹钟。 |

## 2. RTC 实时时钟与 Alarm 详解

RTC 使用低速时钟和备份域独立计时，RT-Thread 将其注册为 `rtc`，`date` 通过统一 RTC 接口读取或设置时间。Alarm 由 RTC 比较逻辑产生唤醒/中断事件，不依赖主循环持续轮询。

一次完整的数据路径如下：

```text
低速时钟/备份域 -> 片上 RTC -> rt_device rtc -> date/Alarm
```

协议层只定义通信或数据处理规则，最终仍需要控制器时钟、引脚复用、中断/DMA 和上层状态机共同工作。扫描到设备、注册了总线或编译通过，都不能替代完整的数据传输验证。

## 3. GD32H77D 的 RTC 与备份域特性

GD32H77D 片上 RTC 位于低速时钟和备份域，可在主 CPU 停止时继续计时。Alarm 比较单元可以产生中断/唤醒；复位或掉电后的保持能力取决于备份域和 VBAT 条件。

## 4. RT-Thread RTC 与 Alarm 设备接口

RT-Thread 注册 `rtc`，时间读写通过 RTC device control 接口完成，POSIX 时间函数和 MSH `date` 命令建立在该设备之上。Alarm 组件管理闹钟对象并由 RTC 中断触发回调。

主参考设备：`rtc`。设备是否已注册可先通过 `list_device` 和 `gino_device_probe` 判断；更上层的文件系统、网络或 GUI 组件还需继续验证其挂载、链路或刷新状态。

## 5. 硬件说明

验证片上 RTC 与 Alarm；掉电保持能力取决于 RTC 时钟源、备份域和后备电源。

默认控制台：UART1，PA2/PA3，AF7，115200-8-N-1。

RTC 保持能力取决于低速时钟、备份域和 VBAT 条件。

## 6. 工程示例说明

以下源码路径以 SDK 仓库中的工程目录为基准：

- `applications/device_probe.c`
- `applications/rtc_alarm_example.c`
- `../../libraries/gd32_drivers/drv_rtc.c`
- `board/Kconfig`

建议先阅读 `applications/main.c` 和 `applications/device_probe.c`，再沿数据路径进入对应驱动、组件或软件包。示例保留 MSH 命令，便于在不改动应用代码的情况下观察设备注册和运行状态。

### 6.1 运行命令

- `gino_device_probe`
- `date`
- `rtc_alarm 5`
- `rtc_alarm status`
- `rtc_alarm stop`

```text
date 2026 09 08 12 00 00
rtc_alarm 5
rtc_alarm status
```

约 5 秒后，ulog 输出一次带本地日期时间的 `[I/rtc.alarm] Triggered:` 日志。不带参数的 `rtc_alarm` 同样设置 5 秒后的闹钟，`rtc_alarm help` 查看用法。测试取消功能时，先执行 `rtc_alarm 30`，再在到期前执行 `rtc_alarm stop`。

命令通过 `rt_alarm_create()` 和 `rt_alarm_start()` 创建、启动一次性闹钟。RTC Alarm0 中断通知 RT-Thread Alarm 服务，由服务线程执行回调并打印日志。工程默认启用 INFO 级别的 ulog 控制台后端，Alarm 服务线程栈调整为 2048 字节。应用启动和设备探测日志也使用 ulog；系统自带的 `date`、`list_device` 保持标准命令行输出。

闹钟截止时间基于 RTC 时间计算，设置闹钟后应避免再用 `date` 调整时间。示例闹钟对象保存在 RAM 中，复位后需要重新设置，即使 RTC 时间仍然保持。

### 6.2 运行步骤

1. 检查供电、接线、外部模块和接口电平。
2. 复位开发板，确认 UART1 控制台可用且 PC4 LED 正常闪烁。
3. 执行 `list_device`，确认依赖总线和目标设备均已注册。
4. 按顺序执行上述命令，并同时观察返回值、外部波形、网络状态或显示结果。

## 7. 运行效果

### 7.1 预期现象

- `gino_device_probe` 能打开 `rtc`。
- 连续执行 `date` 时秒值单调增加。
- `rtc_alarm 5` 到期后仅输出一次 ulog 触发日志，取消后的闹钟不再触发。
- 设置时间后复位，是否保持应与备份域和后备电源设计一致。
