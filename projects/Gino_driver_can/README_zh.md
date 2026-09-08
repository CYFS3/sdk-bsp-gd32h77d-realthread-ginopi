# Gino_driver_can

## 1. 简介

本工程是 GD32H77D Gino 开发板的 CAN1 外部总线收发示例。上电只初始化设备并启动接收，不自动发送。每次执行 `gino_can_send` 命令发送一帧经典 CAN；启用 CAN FD 时，再发送一帧 CAN FD。命令可以反复调用，外部节点发来的数据通过彩色 ulog 打印到串口终端。

主参考设备：`can1`。

所有示例均保留 `uart1` 作为 FinSH/MSH 控制台，串口参数为 115200-8-N-1；PC4 LED 用于运行指示。

当前工程包含以下 MSH 命令：

| 命令 | 作用 | 示例/备注 |
| --- | --- | --- |
| `gino_can_send` | 发送一帧经典 CAN，启用 CAN FD 时再发送一帧 CAN FD。 | 每次调用只发送一组，可反复执行。 |
| `gino_device_probe` | 检查当前示例使用的主要设备是否已注册。 | 默认检查 `can1`。 |
| `list_device` | 查看系统已注册设备。 | 确认 UART、PIN、总线和目标设备存在。 |

## 2. CAN 总线、仲裁与错误处理详解

CAN1 控制器负责位时序、ID 仲裁、ACK、错误计数和重发，PB4/PB5 仍需通过外部 CAN 收发器连接差分 CANH/CANL。低数值 ID 在仲裁中通常具有更高优先级，应用需要自行定义 ID 和负载协议。

一次完整的数据路径如下：

```text
应用 CAN 帧 -> RT-Thread can1 -> CAN1 控制器 -> PB4/PB5 -> 收发器 -> CANH/CANL
```

协议层只定义通信或数据处理规则，最终仍需要控制器时钟、引脚复用、中断/DMA 和上层状态机共同工作。扫描到设备、注册了总线或编译通过，都不能替代完整的数据传输验证。

## 3. GD32H77D 的 CAN1 特性

工程启用 GD32H77D CAN1，PB4/PB5 连接控制器 TX/RX。控制器提供位时序、过滤器、发送邮箱、接收 FIFO、错误计数和自动重发，但物理差分层必须由外部 CAN 收发器完成。

## 4. RT-Thread CAN 设备接口

RT-Thread 将 CAN1 注册为 `can1`。应用通过 `rt_device_open` 配置中断收发模式，通过 `rt_device_control` 设置波特率/模式，使用 `struct rt_can_msg` 配合 `rt_device_read`/`rt_device_write` 收发帧。

主参考设备：`can1`。应用在正常模式下打开设备，MSH 命令按顺序发送帧，接收回调通知接收线程读取队列并打印数据。

## 5. 硬件说明

CAN1 默认使用 PB4/AF4 发送、PB5/AF9 接收。通过外部收发器连接对端的 CANH/CANL，并连接参考地，总线两端各配置 120 Ω 终端电阻。启用 CAN FD 时，收发器和对端都必须支持 ISO CAN FD 及 2 Mbit/s 数据段速率。

默认控制台：UART1，PA2/PA3，AF7，115200-8-N-1。

必须外接 CAN 收发器并正确配置终端电阻；MCU 引脚不能直接连接 CANH/CANL。

## 6. 工程示例说明

上电只启动接收。需要发送时，在 MSH 终端执行：

```text
msh /> gino_can_send
```

每次调用只发送一组，执行结束后返回命令行；再次输入同一命令即可再次发送。接收线程始终运行。

- `applications/main.c`：初始化 CAN、顺序发送经典 CAN/CAN FD 帧、接收并打印外部数据。
- `applications/device_probe.c`
- `../../libraries/gd32_drivers/drv_can.c`
- `../../libraries/gd32_drivers/config/can_config.h`

### 6.1 命令发送的数据

默认仲裁段为 **500 kbit/s**。定义 `RT_CAN_USING_CANFD` 时，应用自动启用 ISO CAN FD，数据段为 **2 Mbit/s**，并保留经典 CAN 帧的收发能力。

| 发送顺序 | 帧类型 | 标准 ID | 长度 | BRS | 数据 |
| --- | --- | --- | --- | --- | --- |
| 1 | 经典 CAN 数据帧 | `0x123` | 8 字节 | 0 | `00 01 02 03 04 05 06 07` |
| 2 | CAN FD 数据帧，仅启用 CAN FD 时发送 | `0x124` | 64 字节 | 1 | `00` 至 `3F`，每字节递增 1 |

每次执行 `gino_can_send` 先发送一帧经典 CAN，启用 CAN FD 时等待 100 ms 后再发送一帧 CAN FD，随后结束，不循环发送。关闭 `RT_CAN_USING_CANFD` 后，每次命令只发送第一帧。发送成功通过 `LOG_I` 打印绿色 `[TX]`，失败或等待完成超时通过 `LOG_E` 打印红色 `[TX FAIL]`，应用不会自动重试该命令。

### 6.2 接收并打印到终端

接收线程持续读取外部节点发送的帧，每收到一帧就通过 `LOG_I` 输出一行绿色 `[RX]`，包含帧类型、标准/扩展 ID、RTR、BRS、字节长度及全部十六进制数据。打印操作在接收线程中执行，中断回调只负责通知。

工程已启用 `RT_USING_ULOG`、`ULOG_BACKEND_USING_CONSOLE` 和 `ULOG_USING_COLOR`。日志包含 tick 时间、等级和 `can.demo` 标签；`ULOG_LINE_BUF_SIZE=512` 可完整容纳 64 字节 FD 数据。串口终端需支持 ANSI 颜色显示。

这是正常模式下的外部收发，开发板不会将自己发送的帧作为接收数据打印。要同时观察对应的 `[TX]` 和 `[RX]`，请将对端配置为收到数据后原样回发，或在 CAN 工具中手动发送同样的 ID 和数据。仅由对端提供 ACK 不会产生应用层接收帧。开发板只打印收到的帧，不自动回发接收数据。

执行命令且对端原样回发时，可观察到如下日志正文（省略 ulog 时间、等级和标签前缀）。实际 TX/RX 日志的先后顺序取决于对端响应时机：

```text
can1 normal mode: arbitration=500000 bit/s, ISO CAN FD data=2000000 bit/s
[TX] can1 CAN STD ID=0x00000123 RTR=0 BRS=0 LEN=8 DATA=00 01 02 03 04 05 06 07
[RX] can1 CAN STD ID=0x00000123 RTR=0 BRS=0 LEN=8 DATA=00 01 02 03 04 05 06 07
[TX] can1 CANFD STD ID=0x00000124 RTR=0 BRS=1 LEN=64 DATA=00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
[RX] can1 CANFD STD ID=0x00000124 RTR=0 BRS=1 LEN=64 DATA=00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
```

### 6.3 CAN FD 配置说明

本工程已启用 `RT_CAN_USING_CANFD`，非阻塞发送缓冲区为 1152 字节，可存放 16 个 `rt_can_msg`；本示例使用阻塞发送保证发送顺序。其他工程可在 `RT-Thread Components -> Device Drivers -> Using CAN device drivers` 中启用 `Enable CAN-FD support`。

`can_demo_init()` 根据编译选项配置 `enable_canfd`，再打开正常模式下的中断收发。发送经典 CAN 时 `fd_frame=0`，随后发送 FD 帧时设置 `fd_frame=1`、`brs=1`，两种帧之间不需要重新初始化控制器。

- `message.len` 表示字节数，64 字节填写 `64`，不是 DLC 编码 `15`。合法长度为 0 至 8、12、16、20、24、32、48、64；其他 9 至 63 字节长度会补零至下一个合法长度，接收端返回补齐后的长度。
- `fd_frame=0` 发送经典 CAN 帧，最多 8 字节；`fd_frame=1, brs=0` 发送不切换波特率的 FD 帧；`fd_frame=1, brs=1` 启用数据段速率切换。CAN FD 不支持远程帧。
- 接收仍使用 `rt_device_read`，返回的 `fd_frame`、`brs` 和 `len` 描述实际帧。启用硬件过滤宏时，读取普通接收队列前设置 `hdr_index=-1`。
- 数据段最高 8 Mbit/s，驱动根据实际 APB2 时钟选择仲裁段和数据段共用的预分频，约 80% 采样点。默认 300 MHz APB2 下支持 500 kbit/s 仲裁配合 2、4、5 Mbit/s 数据段；8 Mbit/s 无法由该时钟整除，会返回 `-RT_EINVAL`。自定义位时序接口暂不支持，返回 `-RT_ENOSYS`。
- CAN FD 的 64 字节邮箱占用 72 字节，512 字节消息 RAM 最多容纳 7 个邮箱。启用编译选项后固定使用邮箱 0 至 2 接收、3 至 6 发送，`RT_CANSND_BOX_NUM` 允许 1 至 3，至少保留一个邮箱用于非阻塞发送。关闭编译选项后保留经典 CAN 的 16 接收、16 发送布局。
- 用 `RT_CAN_CMD_SET_CANFD` 和 `(void *)0U` 切回经典 CAN。修改配置前停止发送并等待发送队列排空，硬件发送尚未完成时返回 `-RT_EBUSY`。

### 6.4 验证步骤

1. 连接收发器、CANH/CANL、参考地和终端电阻，打开 UART1 串口终端，设置为 115200-8-N-1。
2. 将外部 CAN 工具或另一块开发板设置为正常模式、仲裁段 500 kbit/s；本工程启用 CAN FD 时，对端同时设置 ISO CAN FD、数据段 2 Mbit/s。
3. 复位开发板，此时不发送。执行 `gino_can_send`，观察终端依次打印经典 CAN 和 CAN FD 的发送数据，使用对端确认收到 `0x123` 和 `0x124`；再次执行命令可再次发送一组。
4. 从对端发送对应帧或开启对端回发功能，确认开发板打印完整的 `[RX]` 数据。接收也支持其他 ID，无需限定为示例发送 ID。
5. 可执行 `list_device` 和 `gino_device_probe` 检查设备状态。若出现 `[TX FAIL]`，检查对端是否在线并提供 ACK、速率是否一致以及接线和终端电阻。

## 7. 运行效果

### 7.1 预期现象

- `list_device` 和 `gino_device_probe` 能找到 `can1`。
- 上电不发送。每次执行 `gino_can_send` 先发送一帧 `0x123` 经典 CAN，启用 CAN FD 时再发送一帧 `0x124` CAN FD；命令可以反复调用。
- 对端发送或回发数据后，终端逐帧打印 `[RX]` 和完整数据。
- 总线空闲时 CANH/CANL 处于正确的隐性电平。
