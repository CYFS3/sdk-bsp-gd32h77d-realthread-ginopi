# Gino_driver_sdcard

## 1. 简介

本工程是 GD32H77D Gino 开发板的 SDIO1 与 SD 卡文件系统参考工程，用于单独学习、配置和验证 SD/SDIO 块设备协议。独立工程只启用该功能需要的驱动和组件，可作为应用开发和功能集成的参考。

主参考设备：`sd0`。

所有示例均保留 `uart1` 作为 FinSH/MSH 控制台，串口参数为 115200-8-N-1；PC4 LED 用于运行指示。

当前工程包含以下 MSH 命令：

| 命令 | 作用 | 示例/备注 |
| --- | --- | --- |
| `gino_device_probe` | 检查当前示例使用的主要设备是否已注册。 | 默认检查 `sd0`。 |
| `ls /sd` | 列出已挂载 SD 卡文件系统中的文件。 | 需要可用的 FAT/FAT32 卡。 |

## 2. SD/SDIO 块设备协议详解

SDIO1 完成 SD 卡识别和块读写后注册 `sd0`，DFS/FatFs 读取分区与文件系统元数据并挂载到 `/sd`。卡检测脚只说明介质是否插入，不能代替初始化、块读写和文件系统验证。

一次完整的数据路径如下：

```text
SD 卡 -> SDIO1 + 卡检测 -> sd0 块设备 -> DFS/FatFs -> /sd
```

协议层只定义通信或数据处理规则，最终仍需要控制器时钟、引脚复用、中断/DMA 和上层状态机共同工作。扫描到设备、注册了总线或编译通过，都不能替代完整的数据传输验证。

## 3. GD32H77D 的 SDIO1 特性

工程启用 GD32H77D SDIO1 及卡检测，控制器完成命令响应、数据线传输和块读写。卡检测配置为低电平有效，但卡检测只表示介质插入，不代表卡初始化和文件系统已成功。

## 4. RT-Thread SDIO、块设备与 DFS 设备接口

RT-Thread SDIO/MMCSd 层在识别卡后注册 `sd0` 块设备。DFS/elm-FatFs 使用块设备读写扇区，并按自动挂载配置将文件系统挂载到 `/sd`。

主参考设备：`sd0`。设备是否已注册可先通过 `list_device` 和 `gino_device_probe` 判断；更上层的文件系统、网络或 GUI 组件还需继续验证其挂载、链路或刷新状态。

## 5. 硬件说明

SDIO1 注册 sd0，并将 FAT/FAT32 文件系统自动挂载到 /sd。测试前插入可用 SD 卡。

默认控制台：UART1，PA2/PA3，AF7，115200-8-N-1。

测试挂载点前先插入可用的 FAT/FAT32 SD 卡。

## 6. 工程示例说明

以下源码路径以 SDK 仓库中的工程目录为基准：

- `../../libraries/Board_Drivers/drv_filesystem.c`
- `../../libraries/gd32_drivers/drv_sdio.c`
- `../../libraries/gd32_drivers/config/sdio_config.h`

建议先阅读 `applications/main.c` 和 `applications/device_probe.c`，再沿数据路径进入对应驱动、组件或软件包。示例保留 MSH 命令，便于在不改动应用代码的情况下观察设备注册和运行状态。

### 6.1 运行命令

- `gino_device_probe`
- `ls /sd`

### 6.2 运行步骤

1. 检查供电、接线、外部模块和接口电平。
2. 复位开发板，确认 UART1 控制台可用且 PC4 LED 正常闪烁。
3. 执行 `list_device`，确认依赖总线和目标设备均已注册。
4. 按顺序执行上述命令，并同时观察返回值、外部波形、网络状态或显示结果。

## 7. 运行效果

### 7.1 预期现象

- 插卡后 `list_device` 中出现 `sd0`。
- `ls /sd` 能列出 FAT/FAT32 文件和目录。
- 拔插时设备和挂载状态按配置更新，不应影响 UART1 控制台。
