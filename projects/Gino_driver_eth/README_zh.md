# Gino_driver_eth

## 1. 简介

本工程是 GD32H77D Gino 开发板的 ENET1 RMII 以太网参考工程，用于单独学习、配置和验证 Ethernet、RMII 与 TCP/IP 分层。独立工程只启用该功能需要的驱动和组件，可作为应用开发和功能集成的参考。

主参考设备：`e0`。

所有示例均保留 `uart1` 作为 FinSH/MSH 控制台，串口参数为 115200-8-N-1；PC4 LED 用于运行指示。

当前工程包含以下 MSH 命令：

| 命令 | 作用 | 示例/备注 |
| --- | --- | --- |
| `gino_device_probe` | 检查当前示例使用的主要设备是否已注册。 | 默认检查 `e0`。 |
| `ifconfig` | 查看网络设备的联网状态和 IP 地址。 | 确认网络已 ready。 |
| `ping 192.168.1.1` | 通过当前网络接口发送 ICMP 报文。 | 按实际网关或对端地址替换。 |

## 2. Ethernet、RMII 与 TCP/IP 分层详解

lwIP 的 pbuf 通过 RT-Thread `eth_device` 交给 ENET1。发送端复制 pbuf 链并把 DMA 描述符 ownership 交给硬件；接收中断只通知网络线程，线程复制有效帧并把描述符归还 DMA。PHY 线程负责地址检测、自动协商和链路状态。

一次完整的数据路径如下：

```text
socket/SAL -> lwIP -> e0 eth_device -> ENET DMA -> RMII -> PHY -> 网线
```

协议层只定义通信或数据处理规则，最终仍需要控制器时钟、引脚复用、中断/DMA 和上层状态机共同工作。扫描到设备、注册了总线或编译通过，都不能替代完整的数据传输验证。

## 3. GD32H77D 的 ENET1 与 RMII 特性

工程使用 GD32H77D ENET1 RMII，PHY 默认地址 2，PC12 接收 50 MHz REF_CLK。ENET DMA 使用专用描述符和缓冲区完成帧收发，MDIO/MDC 用于 PHY 寄存器管理和自动协商。

## 4. RT-Thread Ethernet、Netdev 与 SAL 设备接口

驱动注册 RT-Thread `eth_device` 和 `e0` netdev。接收中断调用 `eth_device_ready` 唤醒网络线程，lwIP 提供 IP/TCP/UDP，SAL 向应用提供统一 socket，`ifconfig` 查看网络配置，`ping` 检查网络连通性。

主参考设备：`e0`。设备是否已注册可先通过 `list_device` 和 `gino_device_probe` 判断；更上层的文件系统、网络或 GUI 组件还需继续验证其挂载、链路或刷新状态。

## 5. 硬件说明

ENET1 使用 RMII，默认 PHY 地址 2；PHY 必须向 PC12 提供 50 MHz 参考时钟。

默认控制台：UART1，PA2/PA3，AF7，115200-8-N-1。

## 6. 工程示例说明

以下源码路径以 SDK 仓库中的工程目录为基准：

- `../../libraries/Board_Drivers/drv_enet.c`
- `board/SConscript`
- `rtconfig.h`
- `applications/device_probe.c`

建议先阅读 `applications/main.c` 和 `applications/device_probe.c`，再沿数据路径进入对应驱动、组件或软件包。示例保留 MSH 命令，便于在不改动应用代码的情况下观察设备注册和运行状态。

### 6.1 运行命令

- `gino_device_probe`
- `ifconfig`
- `ping 192.168.1.1`

### 6.2 运行步骤

1. 检查供电、接线、外部模块和接口电平。
2. 复位开发板，确认 UART1 控制台可用且 PC4 LED 正常闪烁。
3. 执行 `list_device`，确认依赖总线和目标设备均已注册。
4. 按顺序执行上述命令，并同时观察返回值、外部波形、网络状态或显示结果。

## 7. 运行效果

### 7.1 预期现象

- 启动日志能找到默认地址 2 的 PHY 并完成自动协商。
- `ifconfig` 显示 `e0` link up 和有效 IP。
- `ping <网关>` 有稳定回复，持续收发时无 DMA descriptor 错误。
