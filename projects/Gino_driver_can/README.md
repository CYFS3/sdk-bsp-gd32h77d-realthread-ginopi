# Gino_driver_can

## 1. Introduction

This project demonstrates external CAN1 communication on the GD32H77D Gino board. Startup initializes the device and reception without transmitting. Each `gino_can_send` command sends one classic CAN frame, followed by one CAN FD frame when enabled. The command can be called repeatedly. Received frames are printed to the UART console using colored ulog output.

Primary device: `can1`.

All examples keep `uart1` as the FinSH/MSH console at 115200-8-N-1, and PC4 LED is used as the run indicator.

Current project provides the following MSH commands:

| Command | Function | Example/Note |
| --- | --- | --- |
| `gino_can_send` | Send one classic CAN frame, then one CAN FD frame when enabled. | Sends one set per invocation; repeat the command to send again. |
| `gino_device_probe` | Check whether the primary device used by this example is registered. | Checks `can1` by default. |
| `list_device` | List registered system devices. | Confirm that UART, PIN, buses, and target devices exist. |

## 2. CAN Arbitration and Error Handling Details

CAN1 handles bit timing, ID arbitration, ACK, error counters, and retransmission. PB4/PB5 still require an external transceiver for CANH/CANL. Lower numeric IDs normally win arbitration, while the application defines the payload protocol.

The complete data path is:

```text
application CAN frame -> RT-Thread can1 -> CAN1 -> PB4/PB5 -> transceiver -> CANH/CANL
```

The protocol layer only defines communication and data processing rules. Final validation still depends on controller clocks, pin multiplexing, interrupt/DMA handling, and upper-layer state machines. Device discovery, bus registration, or a successful build cannot replace a complete data transfer test.

## 3. GD32H77D CAN1 Features

The project enables GD32H77D CAN1 on PB4/PB5. The controller provides bit timing, filters, TX mailboxes, RX FIFOs, error counters, and retransmission; an external transceiver supplies the differential physical layer.

## 4. RT-Thread CAN Device Interface

RT-Thread registers CAN1 as `can1`. Applications use `rt_device_open` to configure interrupt-driven TX/RX, `rt_device_control` to set bitrate and mode, and `rt_device_read`/`rt_device_write` to exchange `struct rt_can_msg` objects.

Primary device: `can1`. The application opens it in normal mode. The MSH command sends frames in order; the RX callback wakes a receive thread that drains the queue and prints each frame.

## 5. Hardware

CAN1 uses PB4/AF4 for TX and PB5/AF9 for RX. Connect the external transceiver to the peer's CANH/CANL and reference ground, with 120-ohm termination at each end of the bus. With CAN FD enabled, both the transceiver and peer must support ISO CAN FD at a 2 Mbit/s data rate.

Default console: UART1, PA2/PA3, AF7, 115200-8-N-1.

A CAN transceiver and proper termination are mandatory; MCU pins cannot connect directly to CANH/CANL.

## 6. Example

Reception starts after reset without transmitting. Run the following MSH command to send one set of frames:

```text
msh /> gino_can_send
```

The command returns after sending once and can be invoked again. Reception remains active.

- `applications/main.c`: CAN initialization, ordered classic CAN/CAN FD transmission, and RX printing.
- `applications/device_probe.c`
- `../../libraries/gd32_drivers/drv_can.c`
- `../../libraries/gd32_drivers/config/can_config.h`

### 6.1 Send Command

The arbitration bitrate is **500 kbit/s**. With `RT_CAN_USING_CANFD` defined, the application enables ISO CAN FD at **2 Mbit/s** for the data phase while also supporting classic frames.

| Order | Format | Standard ID | Length | BRS | Payload |
| --- | --- | --- | --- | --- | --- |
| 1 | Classic CAN data frame | `0x123` | 8 bytes | 0 | `00 01 02 03 04 05 06 07` |
| 2 | CAN FD data frame, only when enabled | `0x124` | 64 bytes | 1 | Incrementing bytes from `00` through `3F` |

Each `gino_can_send` invocation sends one classic frame, waits 100 ms when CAN FD is enabled, then sends one FD frame and returns. Without CAN FD, only the classic frame is sent. Successful writes produce green `[TX]` logs through `LOG_I`; failed or timed-out writes produce red `[TX FAIL]` logs through `LOG_E`. The application does not automatically retry the command.

### 6.2 Receive Output

The receive thread prints each external frame as one green `[RX]` line through `LOG_I`, including CAN/CANFD format, standard/extended ID, RTR, BRS, byte count, and the complete hexadecimal payload. The interrupt callback only signals the thread.

The project enables `RT_USING_ULOG`, `ULOG_BACKEND_USING_CONSOLE`, and `ULOG_USING_COLOR`. Logs include tick time, level, and the `can.demo` tag. `ULOG_LINE_BUF_SIZE=512` accommodates a complete 64-byte FD payload. Use a serial terminal with ANSI color support.

The board runs in normal mode and does not receive its own transmissions. Configure the peer to echo the frames, or manually send the same IDs and payloads, to see corresponding TX and RX lines. ACK alone does not produce an application RX frame. Incoming frames are printed without being automatically retransmitted.

Example log bodies when running the command with peer echo enabled, with ulog time, level, and tag prefixes omitted. TX/RX log ordering depends on the peer's response timing:

```text
can1 normal mode: arbitration=500000 bit/s, ISO CAN FD data=2000000 bit/s
[TX] can1 CAN STD ID=0x00000123 RTR=0 BRS=0 LEN=8 DATA=00 01 02 03 04 05 06 07
[RX] can1 CAN STD ID=0x00000123 RTR=0 BRS=0 LEN=8 DATA=00 01 02 03 04 05 06 07
[TX] can1 CANFD STD ID=0x00000124 RTR=0 BRS=1 LEN=64 DATA=00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
[RX] can1 CANFD STD ID=0x00000124 RTR=0 BRS=1 LEN=64 DATA=00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
```

### 6.3 CAN FD Configuration

This project enables `RT_CAN_USING_CANFD` and has a 1152-byte non-blocking TX buffer for 16 `rt_can_msg` objects; the example uses blocking writes to preserve transmission order. Other projects can enable `Enable CAN-FD support` under `RT-Thread Components -> Device Drivers -> Using CAN device drivers`.

`can_demo_init()` configures `enable_canfd` according to the build option before opening the device for interrupt-driven TX/RX in normal mode. Classic frames use `fd_frame=0`; subsequent FD frames use `fd_frame=1` and `brs=1`. The controller does not need to be reinitialized between the two frame types.

- `message.len` is a byte count: use `64` for 64 bytes, not the DLC encoding `15`. Legal lengths are 0 through 8, 12, 16, 20, 24, 32, 48, and 64. Other lengths from 9 through 63 are zero-padded to the next legal length, which is reported on reception.
- `fd_frame=0` sends a classic CAN frame of up to 8 bytes. `fd_frame=1, brs=0` sends an FD frame without bitrate switching; `fd_frame=1, brs=1` enables data-phase bitrate switching. CAN FD does not support remote frames.
- Reception still uses `rt_device_read`; `fd_frame`, `brs`, and `len` describe the actual frame. Set `hdr_index=-1` before reading the general RX queue when hardware filtering is enabled.
- The maximum data rate is 8 Mbit/s. The driver calculates timing from the actual APB2 clock with a shared arbitration/data prescaler and a sample point near 80%. At the default 300 MHz APB2 clock, 500 kbit/s arbitration supports 2, 4, and 5 Mbit/s data rates. An exact 8 Mbit/s is unavailable at this clock and returns `-RT_EINVAL`. Custom bit timing currently returns `-RT_ENOSYS`.
- A 64-byte CAN FD mailbox occupies 72 bytes, so the 512-byte message RAM holds at most seven mailboxes. With CAN FD compiled in, mailboxes 0-2 receive and 3-6 transmit. `RT_CANSND_BOX_NUM` accepts 1-3, leaving at least one TX mailbox for non-blocking sends. Without CAN FD compiled in, the original 16 RX / 16 TX layout is retained.
- `RT_CAN_CMD_SET_CANFD` with `(void *)0U` restores classic CAN mode. Stop transmitting and drain the TX queues before changing configuration; active hardware TX returns `-RT_EBUSY`.

### 6.4 Validation

1. Connect the transceiver, CANH/CANL, reference ground, and termination. Open UART1 at 115200-8-N-1.
2. Set the external peer to normal mode at 500 kbit/s arbitration. With CAN FD enabled, also configure ISO CAN FD with a 2 Mbit/s data rate.
3. Reset the board; no frames are sent yet. Run `gino_can_send` and confirm that the peer receives classic ID `0x123`, followed by FD ID `0x124` when enabled. Run the command again to send another set.
4. Send frames from the peer or enable peer echo, and check the complete `[RX]` data in the board's console. Other incoming IDs are also accepted.
5. Use `list_device` or `gino_device_probe` to inspect registration. For `[TX FAIL]`, check the peer's ACK, bitrates, wiring, and termination.

## 7. Runtime Results

### 7.1 Expected Behavior

- `list_device` and `gino_device_probe` find `can1`.
- Startup sends no frames. Each `gino_can_send` invocation sends classic CAN ID `0x123`, then CAN FD ID `0x124` when enabled.
- Frames sent or echoed by the peer appear as `[RX]` lines with their complete payloads.
- CANH/CANL show valid recessive levels while idle.
