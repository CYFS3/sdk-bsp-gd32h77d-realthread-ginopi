# Gino_driver_qspi_flash

## 1. 简介

本工程是 GD32H77D Gino 开发板的 QSPI Flash 参考工程，用于验证通过 OSPI0 连接的板载 GD25Q64E。独立工程只启用该功能需要的驱动和组件，可作为应用开发和功能集成的参考。

主参考设备：`qspi_flash0`。

所有示例均保留 `uart1` 作为 FinSH/MSH 控制台，串口参数为 115200-8-N-1；PC4 LED 用于运行指示。

当前工程包含以下 MSH 命令：

| 命令 | 作用 | 示例/备注 |
| --- | --- | --- |
| `gino_device_probe` | 检查当前示例使用的主要设备是否已注册。 | 默认检查 `qspi_flash0`。 |
| `fal probe filesystem` | 探测 FAL `filesystem` 分区。 | 访问文件系统前确认分区信息。 |
| `fal read 0 16` | 从当前 FAL 设备读取数据。 | 用于基础 flash 读取检查。 |

## 2. QSPI NOR Flash 协议与文件系统详解

OSPI0 以四线数据阶段访问 GD25Q64E NOR Flash。底层驱动提供读、页编程和扇区擦除，FAL 再把 Flash 划分为 `filesystem` 分区并转换成块设备，DFS/FatFs 最终挂载为 `/flash`。NOR Flash 写入前必须满足擦除和忙状态约束。

一次完整的数据路径如下：

```text
GD25Q64E -> OSPI0/MDMA -> qspi_flash0 -> FAL filesystem -> 块设备 -> FatFs -> /flash
```

协议层只定义通信或数据处理规则，最终仍需要控制器时钟、引脚复用、中断/DMA 和上层状态机共同工作。扫描到设备、注册了总线或编译通过，都不能替代完整的数据传输验证。

## 3. GD32H77D 的 OSPI0 与 MDMA 特性

GD32H77D OSPI0 可在命令、地址和数据阶段使用多线传输，并由 MDMA 减少大块搬运的 CPU 开销。板载 GD25Q64E 容量为 8 MiB，典型擦除粒度 4 KiB、页编程粒度 256 B。

## 4. RT-Thread FAL、块设备与 DFS 设备接口

底层注册 `qspi_flash0`，FAL 通过 `fal_init` 和 `fal_partition_find` 找到 `filesystem` 分区，再由 `fal_blk_device_create` 暴露块设备，DFS/FatFs 将其挂载到 `/flash`。

主参考设备：`qspi_flash0`。设备是否已注册可先通过 `list_device` 和 `gino_device_probe` 判断；更上层的文件系统、网络或 GUI 组件还需继续验证其挂载、链路或刷新状态。

## 5. 硬件说明

板载 8 MiB GD25Q64E 注册为 qspi_flash0，FAL 分区 filesystem 挂载到 /flash。首次使用可能需要格式化。

默认控制台：UART1，PA2/PA3，AF7，115200-8-N-1。

未格式化分区可能需要先格式化才能挂载 `/flash`；格式化会清除已有内容。

## 6. 工程示例说明

以下源码路径以 SDK 仓库中的工程目录为基准：

- `../../libraries/Board_Drivers/drv_qspi_flash.c`
- `../../libraries/Board_Drivers/drv_filesystem.c`
- `../../libraries/Board_Drivers/fal_cfg.h`
- `../../libraries/gd32_drivers/drv_ospi.c`

建议先阅读 `applications/main.c` 和 `applications/device_probe.c`，再沿数据路径进入对应驱动、组件或软件包。示例保留 MSH 命令，便于在不改动应用代码的情况下观察设备注册和运行状态。

### 6.1 运行命令

- `gino_device_probe`
- `fal probe filesystem`
- `fal read 0 16`

### 6.2 运行步骤

1. 检查供电、接线、外部模块和接口电平。
2. 复位开发板，确认 UART1 控制台可用且 PC4 LED 正常闪烁。
3. 执行 `list_device`，确认依赖总线和目标设备均已注册。
4. 按顺序执行上述命令，并同时观察返回值、外部波形、网络状态或显示结果。

## 7. 运行效果

### 7.1 预期现象

- `gino_device_probe` 能找到 `qspi_flash0`。
- `fal probe filesystem` 显示分区地址和长度。
- `fal read 0 16` 能稳定读取分区开头；格式化并挂载后可访问 `/flash`。
