# Gino_driver_sdram

## 1. 简介

本工程是 GD32H77D Gino 开发板的 32 MiB 外部 SDRAM 参考工程，用于单独学习、配置和验证 SDRAM 时序、刷新与内存扩展。独立工程只启用该功能需要的驱动和组件，可作为应用开发和功能集成的参考。

本工程没有独立的主设备节点。

所有示例均保留 `uart1` 作为 FinSH/MSH 控制台，串口参数为 115200-8-N-1；PC4 LED 用于运行指示。

当前工程包含以下 MSH 命令：

| 命令 | 作用 | 示例/备注 |
| --- | --- | --- |
| `gino_device_probe` | 检查当前示例使用的主要设备是否已注册。 | 默认检查主参考设备。 |
| `free` | 打印 RT-Thread 堆使用情况。 | 确认 SDRAM 已扩展可用 heap。 |
| `sdram_test` | 测试 SDRAM 读写速度并校验数据，显示耗时和 MiB/s。 | 从 SDRAM heap 申请 1 MiB 缓冲区，读写各 8 轮，测试后释放。 |

## 2. SDRAM 时序、刷新与内存扩展详解

SDRAM 控制器必须按器件参数配置 bank、行列地址、CAS 延迟和刷新周期。初始化成功后，32 MiB 空间从 `0xC0000000` 开始并加入系统 heap；它没有独立 `rt_device`，因此用内存统计和压力访问验证。

一次完整的数据路径如下：

```text
系统时钟 -> EXMC/SDRAM 时序与刷新 -> 0xC0000000 内存 -> RT-Thread heap
```

协议层只定义通信或数据处理规则，最终仍需要控制器时钟、引脚复用、中断/DMA 和上层状态机共同工作。扫描到设备、注册了总线或编译通过，都不能替代完整的数据传输验证。

## 3. GD32H77D 的 EXMC 与 SDRAM 特性

GD32H77D 通过外部存储控制器配置 SDRAM 的 bank、行列地址、CAS 延迟和刷新周期。板载 32 MiB SDRAM 从 `0xC0000000` 开始，访问性能和稳定性受系统时钟、总线竞争与器件时序影响。

## 4. RT-Thread 内存管理接口

SDRAM 不注册独立 `rt_device`。BSP 初始化成功后调用内存管理接口扩展 heap，应用通过 `rt_malloc`、`rt_calloc` 和 `rt_free` 间接使用；`free` 用于查看总量和剩余空间。

本工程没有独立的主设备节点。 设备是否已注册可先通过 `list_device` 和 `gino_device_probe` 判断；更上层的文件系统、网络或 GUI 组件还需继续验证其挂载、链路或刷新状态。

## 5. 硬件说明

初始化 0xC0000000 起始的 32 MiB SDRAM 并扩展系统 heap；本工程没有独立设备节点。

默认控制台：UART1，PA2/PA3，AF7，115200-8-N-1。

后续若启用显示或摄像头功能，应避免把 framebuffer/camera 预留区纳入 heap。

## 6. 工程示例说明

以下源码路径以 SDK 仓库中的工程目录为基准：

- `../../libraries/Board_Drivers/drv_sdram_port.c`
- `../../libraries/gd32_drivers/drv_sdram.c`
- `../../libraries/Board_Drivers/board.h`
- `board/SConscript`

建议先阅读 `applications/main.c` 和 `applications/device_probe.c`，再沿数据路径进入对应驱动、组件或软件包。示例保留 MSH 命令，便于在不改动应用代码的情况下观察设备注册和运行状态。

### 6.1 运行命令

- `gino_device_probe`
- `free`
- `sdram_test`

`sdram_test` 无需参数，只访问从 SDRAM heap 申请的测试缓冲区，避开堆管理信息、已分配内存和显示/摄像头预留区；空间不足或 SDRAM 初始化失败时会报错退出。

输出表格中的 `Write`、`Read` 分别表示顺序写入、读取速度，`Total KiB` 为各阶段累计访问量，`Time ms` 为耗时，`MiB/s` 为每秒传输的 MiB（1 MiB = 1024 × 1024 字节）。读写各累计访问 8 MiB，数据校验单独执行并输出 `Verify: PASS` 或首个错误的地址、期望值和实际值。

测速使用当前 CPU、Cache 和 SDRAM 时序配置，写入计时包含每轮 D-Cache clean，读取计时包含每轮 invalidate，避免只读取写入后留在缓存中的数据。结果包含循环和系统调度开销，表示 CPU 顺序访问的有效吞吐率；不是 SDRAM 总线理论带宽。计时不足一个系统 tick 时显示 `N/A`。

### 6.2 运行步骤

1. 检查供电、接线、外部模块和接口电平。
2. 复位开发板，确认 UART1 控制台可用且 PC4 LED 正常闪烁。
3. 执行 `list_device`，确认依赖总线和目标设备均已注册。
4. 按顺序执行上述命令，并同时观察返回值、外部波形、网络状态或显示结果。

## 7. 运行效果

### 7.1 预期现象

- 启动日志显示 SDRAM 初始化成功。
- `free` 显示的可用 heap 明显大于仅使用内部 RAM 的工程。
- `sdram_test` 显示 `Write`、`Read` 的耗时和 MiB/s，并输出 `Verify: PASS`。
- 持续分配、写入、校验和释放大块内存时数据保持一致。
