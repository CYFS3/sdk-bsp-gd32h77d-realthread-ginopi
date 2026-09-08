# Gino_display_lvgl

## 1. 简介

本工程是 GD32H77D Gino 开发板的 MIPI DSI LCD、GT911 与 LVGL 图形参考工程，用于单独学习、配置和验证 LVGL 渲染、输入与双缓冲。独立工程只启用该功能需要的驱动和组件，可作为应用开发和功能集成的参考。

主参考设备：`lcd`。

所有示例均保留 `uart1` 作为 FinSH/MSH 控制台，串口参数为 115200-8-N-1；PC4 LED 用于运行指示。

当前工程包含以下 MSH 命令：

| 命令                  | 作用                                   | 示例/备注         |
| --------------------- | -------------------------------------- | ----------------- |
| `gino_device_probe` | 检查当前示例使用的主要设备是否已注册。 | 默认检查`lcd`。 |

## 2. LVGL 渲染、输入与双缓冲详解

LVGL 独立线程初始化对象系统、显示和触摸，并周期调用 `lv_timer_handler` 推进动画、输入和刷新。本工程使用两个位于内部 AXI SRAM 的 200 行局部绘制缓冲，由 IPA 异步复制到 SDRAM 中的单个扫描帧缓冲，传输完成后才通知 LVGL 复用绘制缓冲。

一次完整的数据路径如下：

```text
GT911 输入 -> LVGL 对象/事件 -> RGB565 缓冲 -> Cache flush/IPA -> TLI/DSI -> LCD
```

协议层只定义通信或数据处理规则，最终仍需要控制器时钟、引脚复用、中断/DMA 和上层状态机共同工作。扫描到设备、注册了总线或编译通过，都不能替代完整的数据传输验证。

## 3. GD32H77D 的 SDRAM、TLI/DSI 与 I2C3 特性

本工程组合 GD32H77D SDRAM、TLI、DSI、IPA、I2C3 和定时器 PWM。TLI/DSI 持续扫描帧缓冲，GT911 提供触摸，IPA 可加速局部复制，Cortex-M7 D-Cache 需要在显示 DMA/TLI 读取前清理。

板级初始化使用 MPU region 0 保护未使用的外部地址窗口，由更高优先级的区域放行有效 SDRAM。若使用 OSPI 内存映射，需要先为其配置独立的可访问 MPU 区域。

D-Cache 在 SDRAM 初始化和显示内存 MPU 配置完成后才开启，LCD 使用板级初始化配置的 MPU 和 Cache 属性。若由 bootloader 跳转启动，需要由 bootloader 在交接前清理并关闭 D-Cache。

## 4. RT-Thread LVGL、Graphic 与 Touch 设备接口

LVGL RT-Thread port 创建独立 `LVGL` 线程，调用 `lv_timer_handler`。显示端使用 `lcd` 图形设备，输入端使用 `gt911` touch device；`lv_display_set_buffers` 和 flush callback 建立 LVGL 与 BSP 的缓冲交接。

主参考设备：`lcd`。设备是否已注册可先通过 `list_device` 和 `gino_device_probe` 判断；更上层的文件系统、网络或 GUI 组件还需继续验证其挂载、链路或刷新状态。

## 5. 硬件说明

整合 LCD、GT911、SDRAM 和 LVGL 离线包。

默认控制台：UART1，PA2/PA3，AF7，115200-8-N-1。

操作屏幕，确认动画和触摸响应持续正常。

## 6. 工程示例说明

以下源码路径以 SDK 仓库中的工程目录为基准：

- `applications/lv_port.c`
- `applications/lv_conf.h`
- `../../libraries/Board_Drivers/drv_lcd.c`
- `../../packages/LVGL-latest/env_support/rt-thread/lv_rt_thread_port.c`

建议先阅读 `applications/main.c` 和 `applications/device_probe.c`，再沿数据路径进入对应驱动、组件或软件包。示例保留 MSH 命令，便于在不改动应用代码的情况下观察设备注册和运行状态。

### 6.1 运行命令

- `gino_device_probe`

### 6.2 运行步骤

1. 检查供电、接线、外部模块和接口电平。
2. 复位开发板，确认 UART1 控制台可用且 PC4 LED 正常闪烁。
3. 执行 `list_device`，确认依赖总线和目标设备均已注册。
4. 按顺序执行上述命令，并同时观察返回值、外部波形、网络状态或显示结果。

## 7. 运行效果

### 7.1 预期现象

- 启动后显示 LVGL demo，触摸可驱动界面控件。
- 绘制缓冲交替使用，IPA 持续将界面更新到 LCD。
