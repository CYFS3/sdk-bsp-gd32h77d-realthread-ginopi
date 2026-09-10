# sdk-bsp-gd32h77d-gino

English | [中文](README_zh.md)

[Online documentation](https://rt-thread-studio.github.io/sdk-bsp-gd32h77d-realthread-ginopi/latest/index_en.html) | [Local preview](docs/config/README.md)

## Overview

This repository provides an RT-Thread Studio board support package for the GD32H77D Gino board, based on RT-Thread 5.3.0. Shared RT-Thread, GD32 libraries, and offline packages are combined with 16 independent projects for peripherals and application demos.

![GD32H77D Gino](figures/board.png)

The projects cover the base template, UART, I2C, SPI, QSPI flash, SD card, CAN, PWM, RTC, SDRAM, USB device/host, LVGL display/touch, Wi-Fi, Ethernet, camera, MQTT, and an integrated factory demo, with both Studio and MDK5 project support.

## Board Summary

| Item           | Configuration                                                     |
| -------------- | ----------------------------------------------------------------- |
| MCU            | GD32H77DIW, Arm Cortex-M7, up to 600 MHz                          |
| Clock source   | 25 MHz HXTAL                                                      |
| Console        | UART1, PA2/PA3, AF7, 115200-8-N-1                                 |
| Run LED        | PC4, toggled every 500 ms                                         |
| External SDRAM | 32 MiB at`0xC0000000`                                           |
| Onboard flash  | 8 MiB GD25Q64E QSPI flash on OSPI0                                |
| Display        | 720 x 720 MIPI DSI LCD, FL7707N, RGB565                           |
| Touch          | GT911 on I2C3                                                     |
| Camera         | OV7670, QVGA RGB565, DCI + DMA                                    |
| Ethernet       | ENET1 RMII, default PHY address 2                                 |
| Wi-Fi          | GD32VW553 AT module on UART4                                      |
| Download       | SWD; DAP-Link/PyOCD in Studio, DAP-Link (CMSIS-DAP) in MDK |

See the [SDK user guide](docs/project-guide/README.md) for the SDK directory layout, project selection, hardware connections, and build and download steps.

## Project Matrix

| Project                    | Purpose                            | Main device or mount point  | Validation                                   |
| -------------------------- | ---------------------------------- | --------------------------- | -------------------------------------------- |
| `Gino_template`          | LED blink and development template | PC4 LED                     | `pin list`                                 |
| `Gino_driver_i2c`        | I2C1 bus                           | `hwi2c1`                  | `i2c scan hwi2c1 08 78`                    |
| `Gino_driver_spi`        | SPI3 bus                           | `spi3`                    | `list_device`                              |
| `Gino_driver_qspi_flash` | GD25Q64E, FAL, FatFs               | `qspi_flash0`, `/flash` | `fal probe filesystem`                     |
| `Gino_driver_sdcard`     | SDIO1 and FatFs                    | `sd0`, `/sd`            | `ls /sd`                                   |
| `Gino_driver_can`        | CAN1                               | `can1`                    | `gino_device_probe`                        |
| `Gino_driver_pwm`        | Dual PWM output                    | `pwm2`, `pwm30`         | `pwm_dual_test start`                      |
| `Gino_driver_rtc`        | RTC and alarm                      | `rtc`                     | `date`                                     |
| `Gino_driver_sdram`      | External SDRAM                     | SDRAM heap                  | `free`                                     |
| `Gino_driver_usb_device` | USBHS0 device                      | `usbd`                    | `list_device`                              |
| `Gino_driver_usb_host`   | USBHS1 host and mass storage       | `usbh`, `/udisk`        | `ls /udisk`                                |
| `Gino_driver_eth`        | ENET1 RMII Ethernet                | `e0`                      | `ifconfig`, `ping <gateway>`             |
| `Gino_display_lvgl`      | LCD, touch, and LVGL               | `lcd`, `gt911`          | automatic LVGL demo with touch input |
| `Gino_display_camera`    | OV7670 LCD preview                 | `ov7670`                  | `ov7670_preview start`                     |
| `Gino_component_mqtt`    | SAL and kawaii-mqtt over Wi-Fi     | `wifi0`                   | `gd32vw553_mqtt_start`                     |
| `Gino_factory`           | Integrated board demonstration     | multiple devices            | integrated touch UI, `ifconfig` |

Each project contains its own configuration, Studio metadata, MDK5 project files, application code, and README. The project README covers the hardware, device interfaces, example operation, and expected behavior.

## RT-Thread Studio

1. Open RT-Thread Studio and install the GD32H77D-Gino **v1.0.0** board support package.
2. Select "File -> New -> RT-Thread Project -> Based on Board", choose `GD32H77D-Gino`, and create an example or template project.

   ![Create a board project in RT-Thread Studio](figures/rt-thread-studio-project.png)

3. Build with GNU Arm Embedded 13.3; connect the UART1 console at 115200-8-N-1.
4. Connect DAP-Link, select target `GD32H77DIW` and BIN mode, then download `Debug/rtthread.bin` at `0x08000000`.
5. For debugging, configure the GDB executable from the GNU Arm Embedded 13.3 toolchain, as shown below.

   ![Configure GDB from the GNU Arm Embedded 13.3 toolchain](figures/1789024939745.png)

Studio builds generate `Debug/rtthread.bin` for download and retain `Debug/rtthread.elf` for debugging. The BIN image starts at the boot entry at `0x08000000`, which provides compatibility with different chip revisions; the application vector table is at offset `0x10000` within the image (`0x08010000` in Flash). Set the BIN download address to `0x08000000`.

## MDK5 Build

First install the [GigaDevice device support pack](tools/mdk_pack/GigaDevice.GD32H77x_78x_DFP.0.6.1.pack).

The `rt-thread` and `libraries` directories are shared to avoid duplicating them in every project under `projects` as the SDK grows. Opening an MDK project directly before linking these shared directories results in missing-file errors. Complete the following steps:

1. Use [Env](https://club.rt-thread.org/ask/question/5699.html) to run `mklinks.bat`, creating symbolic links to the `rt-thread` and `libraries` directories.
2. Check that the `rt-thread` and `libraries` folder icons appear in the project directory.
3. Use [Env](https://club.rt-thread.org/ask/question/5699.html) to run `scons --target=mdk5` and update the MDK5 project files.

Use Env 2.0 or later ([download](https://www.rt-thread.org/download.html#download-rt-thread-env-tool)). MDK uses Arm Compiler 6 and requires GigaDevice `GD32H77x_78x_DFP` 0.6.1. See the [project guide](docs/project-guide/README.md) for the complete steps.
