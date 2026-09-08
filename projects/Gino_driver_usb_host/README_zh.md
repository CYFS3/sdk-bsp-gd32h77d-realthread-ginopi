# Gino_driver_usb_host

## 1. 简介

本工程是 GD32H77D Gino 开发板的 USBHS1 Host 与 U 盘参考工程，用于单独学习、配置和验证 USB Host 枚举与大容量存储。独立工程只启用该功能需要的驱动和组件，可作为应用开发和功能集成的参考。

主参考设备：`usbh`。

所有示例均保留 `uart1` 作为 FinSH/MSH 控制台，串口参数为 115200-8-N-1；PC4 LED 用于运行指示。

当前工程包含以下 MSH 命令：

| 命令 | 作用 | 示例/备注 |
| --- | --- | --- |
| `gino_device_probe` | 检查当前示例使用的主要设备是否已注册。 | 默认检查 `usbh`。 |
| `list_device` | 查看系统已注册设备。 | 确认 UART、PIN、总线和目标设备存在。 |
| `ls /udisk` | 列出已挂载 USB 大容量存储文件系统中的文件。 | 需要已枚举的 FAT/FAT32 U 盘。 |

## 2. USB Host 枚举与大容量存储详解

USBHS1 工作在 Host 角色，PF2 控制 VBUS 给下游设备供电。Host 栈负责检测、复位、读取描述符和选择类驱动；大容量存储类随后注册块设备，并由 DFS/FatFs 挂载为 `/udisk`。

一次完整的数据路径如下：

```text
PF2 VBUS -> USB-A U盘 -> USBHS1 Host 枚举 -> Mass Storage -> 块设备 -> FatFs -> /udisk
```

协议层只定义通信或数据处理规则，最终仍需要控制器时钟、引脚复用、中断/DMA 和上层状态机共同工作。扫描到设备、注册了总线或编译通过，都不能替代完整的数据传输验证。

## 3. GD32H77D 的 USBHS1 Host 特性

工程将 GD32H77D USBHS1 配置为 Host，USB-A 接口由 PF2 控制 VBUS。Host 控制器负责端口复位、控制传输和各类 pipe 事务，RT-Thread Host 栈完成描述符解析和类驱动匹配。

## 4. RT-Thread USB Host 与 DFS 设备接口

大容量存储类识别 U 盘后向 RT-Thread 注册块设备，DFS/FatFs 再挂载为 `/udisk`。应用应通过 POSIX/DFS 文件接口访问文件，而不是直接操作 USB 传输 pipe。

主参考设备：`usbh`。设备是否已注册可先通过 `list_device` 和 `gino_device_probe` 判断；更上层的文件系统、网络或 GUI 组件还需继续验证其挂载、链路或刷新状态。

## 5. 硬件说明

USBHS1 Host 使用 USB-A 接口，PF2 控制 VBUS；兼容 U 盘挂载到 /udisk。

默认控制台：UART1，PA2/PA3，AF7，115200-8-N-1。

若无设备枚举，先测量 USB-A 口 VBUS。

## 6. 工程示例说明

以下源码路径以 SDK 仓库中的工程目录为基准：

- `../../libraries/gd32_drivers/usb/drv_otghs_host.c`
- `../../libraries/gd32_drivers/usb/drv_usb_host.c`
- `../../libraries/gd32_drivers/usb/gd32h7_usb_hw.c`
- `../../libraries/Board_Drivers/drv_filesystem.c`

建议先阅读 `applications/main.c` 和 `applications/device_probe.c`，再沿数据路径进入对应驱动、组件或软件包。示例保留 MSH 命令，便于在不改动应用代码的情况下观察设备注册和运行状态。

### 6.1 运行命令

- `gino_device_probe`
- `list_device`
- `ls /udisk`

### 6.2 运行步骤

1. 检查供电、接线、外部模块和接口电平。
2. 复位开发板，确认 UART1 控制台可用且 PC4 LED 正常闪烁。
3. 执行 `list_device`，确认依赖总线和目标设备均已注册。
4. 按顺序执行上述命令，并同时观察返回值、外部波形、网络状态或显示结果。

## 7. 运行效果

### 7.1 预期现象

- 插入兼容 U 盘后出现 USB 枚举和 mass-storage 日志。
- `list_device` 能看到 Host/存储相关设备。
- `ls /udisk` 能列出 FAT/FAT32 文件。
