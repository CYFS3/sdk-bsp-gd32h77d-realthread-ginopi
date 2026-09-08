# Gino_driver_spi

## 1. Introduction

This project is the GD32H77D Gino development board reference project for SPI3 bus. It is used to learn, configure, and validate SPI Synchronous Serial Protocol separately. The standalone project enables only the drivers and components required by this feature, and can be used as a reference for application development and integration.

Primary device: `spi3`.

All examples keep `uart1` as the FinSH/MSH console at 115200-8-N-1, and PC4 LED is used as the run indicator.

Current project provides the following MSH commands:

| Command | Function | Example/Note |
| --- | --- | --- |
| `gino_device_probe` | Check whether the primary device used by this example is registered. | Checks `spi3` by default. |
| `spi_loopback [length] [count]` | Run a full-duplex SPI3 loopback test and compare every received byte. | `spi_loopback` defaults to 32 bytes per round and 10 rounds; `spi_loopback 256 100` runs 100 rounds of 256 bytes. |
| `list_device` | List registered system devices. | Confirm that UART, PIN, buses, and target devices exist. |

## 2. SPI Synchronous Serial Protocol Details

SPI shifts data over SCK, MOSI, and MISO under master control, with a separate chip select per target. This project validates `spi3` registration and external loopback transfers. The loopback command uses 8-bit data, mode 0, MSB first, a requested maximum clock of 1 MHz, and no chip select.

The complete data path is:

```text
application -> RT-Thread SPI bus -> SPI3 -> SCK/MOSI/MISO -> external target
```

The protocol layer only defines communication and data processing rules. Final validation still depends on controller clocks, pin multiplexing, interrupt/DMA handling, and upper-layer state machines. Device discovery, bus registration, or a successful build cannot replace a complete data transfer test.

## 3. GD32H77D SPI3 Features

The project enables GD32H77D SPI3 with PE12/PF0/PF1 for clock and data. The controller provides programmable prescaling, CPOL/CPHA, and data width; chip select is managed by an attached SPI device or GPIO.

## 4. RT-Thread SPI Bus Device Interface

RT-Thread registers `spi3` as a bus. A concrete target is attached with `rt_spi_bus_attach_device` or a BSP helper, then uses `rt_spi_configure`, `rt_spi_transfer`, and `rt_spi_send`/`rt_spi_recv`.

Primary device: `spi3`. Use `list_device` and `gino_device_probe` first to check whether the device has been registered. Upper-layer file system, network, or GUI components still need separate validation for mount, link, or refresh state.

## 5. Hardware

SPI3 uses PE12/PF0/PF1. Chip select, mode, and clock are configured by the target application.

For external loopback, disconnect external SPI targets and connect **PF1 (MOSI)** directly to **PF0 (MISO)**. **PE12 (SCK)** needs no jumper and can be observed with a logic analyzer. No CS connection is required. Use the configured pins instead if the SPI3 pin mapping has been changed.

Default console: UART1, PA2/PA3, AF7, 115200-8-N-1.

A registered idle SPI bus has no waveform until a concrete SPI device issues a transfer.

## 6. Example

Source paths below are relative to the project directory in the SDK repository:

- `applications/main.c`
- `applications/device_probe.c`
- `../../libraries/gd32_drivers/drv_spi.c`
- `../../libraries/gd32_drivers/config/spi_config.h`

Read `applications/main.c` and `applications/device_probe.c` first, then follow the data path into the corresponding driver, component, or package. The example keeps MSH commands so device registration and runtime state can be observed without changing application code.

### 6.1 Runtime Commands

- `gino_device_probe`
- `list_device`
- `spi_loopback`: 32 bytes per round, 10 rounds.
- `spi_loopback 256 100`: 256 bytes per round, 100 rounds.
- `spi_loopback --help`: show parameter ranges and wiring.

`length` accepts 1-256 bytes and `count` accepts 1-1000 rounds, both in decimal. The command attaches a persistent `spi3_loop` device to `spi3` on first use and reuses it on subsequent runs. Each round sends a changing byte pattern while receiving, checks the transfer length, and compares every byte. The current BSP uses polling transfers; the actual SCK frequency depends on the peripheral clock and prescaler.

### 6.2 Operation Steps

1. Disconnect external SPI targets and short PF1 (MOSI) to PF0 (MISO).
2. Reset the development board and confirm that the UART1 console is available and PC4 LED blinks normally.
3. Run `list_device` and confirm that dependency buses and target devices are registered.
4. Run `gino_device_probe`, then `spi_loopback`. A successful default run prints `SPI loopback PASS: 10 rounds, 320 bytes checked.`
5. Run `spi_loopback 256 100` for a longer test. On failure, check the transfer count or the reported round, zero-based byte offset, and TX/RX values against the wiring and pin configuration.

## 7. Runtime Results

### 7.1 Expected Behavior

Runtime logs use ulog with time, level, and tag: `spi.demo` for startup and `spi.test` for device probing and loopback tests. INFO/PASS is green, WARN for invalid arguments is yellow, and ERROR/FAIL is red in terminals that support ANSI colors. Command help uses plain ulog output.

- `list_device` shows `spi3`.
- `gino_device_probe` finds the SPI3 bus.
- `spi_loopback` produces clock/data activity and prints `PASS` when the received bytes match the transmitted bytes.
- Missing or incorrect loopback wiring normally produces `FAIL` with the first mismatching byte. Registration alone does not validate data transfer. The driver's polling waits currently have no timeout, so a controller/clock fault can block the command.
