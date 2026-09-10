# GD32H77D Gino SDK User Guide

[中文](README_zh.md) | [SDK overview](https://github.com/RT-Thread-Studio/sdk-bsp-gd32h77d-realthread-ginopi#readme)

This guide covers the SDK directory layout, project selection, hardware connections, project creation, builds, downloads, and example operation.

## 1. Platform and Tools

- Board: GD32H77D Gino; MCU: GD32H77DIW, Arm Cortex-M7, up to 600 MHz.
- RT-Thread: 5.3.0.
- RT-Thread Studio toolchain: GNU Arm Embedded 13.3.
- Env: version 2.0 or later, with a configured GNU Arm Embedded toolchain.
- MDK: Keil MDK 5.43 or compatible, Arm Compiler 6, GigaDevice `GD32H77x_78x_DFP` 0.6.1, and DAP-Link (CMSIS-DAP).
- Download connection: SWD with DAP-Link/PyOCD in Studio, or DAP-Link (CMSIS-DAP) in MDK.
- Console: UART1, PA2/PA3, AF7, 115200-8-N-1.
- Projects: 16, including `Gino_template` and 15 examples.

### SDK Directory Layout

The examples share the RT-Thread kernel, GD32 libraries, board drivers, and offline packages. Paths below are relative to the SDK root.

| Directory | Contents |
| --- | --- |
| `projects/<name>/applications` | Application entry point and example code |
| `projects/<name>/board` | Project board configuration and linker scripts |
| `libraries/Board_Drivers` | Shared board initialization and onboard peripheral drivers |
| `libraries/gd32_drivers` | GD32 peripheral drivers for RT-Thread |
| `libraries/gd32-arm-*` | GD32 CMSIS, startup code, and peripheral libraries |
| `rt-thread` | RT-Thread kernel, components, and build tools |
| `packages` | Offline packages used by the examples |

Each project has its own `.config`, `rtconfig.h`, Studio metadata, and MDK project files. Configure features in the selected project; changes to shared drivers affect all projects that use them.

For a first build or a new application, start with [Gino_template](../../projects/Gino_template/README.md). To test a peripheral, select its independent example from section 6. Use [Gino_factory](../../projects/Gino_factory/README.md) for the integrated board demonstration.

## 2. Hardware Map

| Peripheral | Pins or resource | Notes |
| --- | --- | --- |
| Run LED | PC4 | toggled every 500 ms |
| UART1 | PA2/PA3, AF7 | console |
| UART4 | PB12/PB13, AF14 | GD32VW553 AT module |
| I2C1 | PH4/PB11, AF4 | independent I2C example |
| I2C2 | PA8/PC9, AF4 | OV7670 control |
| I2C3 | PD12/PD13, AF4 | GT911 touch |
| SPI3 | PE12/PF0/PF1 | SCK/MISO/MOSI |
| CAN1 | PB4/PB5 | external CAN transceiver and termination required |
| LCD backlight | TIMER1 CH0, PA0 | `pwm1` channel 1 |
| PWM outputs | PA7 and PG7 | `pwm2` channel 2 and `pwm30` channel 3 |
| GT911 | reset PH3, interrupt PH5 | address `0x5D` |
| OV7670 | XCLK PE9, I2C2, DCI + DMA1 channel 7 | QVGA RGB565 |
| QSPI flash | CS PC11, CLK PF10, D0-D3 PF8/PF9/PF7/PA1 | 8 MiB GD25Q64E on OSPI0 |
| USB device | USBHS0, PA11/PA12 | Type-C |
| USB host | USBHS1, PB14/PB15, VBUS PF2 | USB-A |
| Ethernet | ENET1 RMII | PHY address 2, 50 MHz REF_CLK on PC12 |
| SDRAM | `0xC0000000`, 32 MiB | display buffers and extended heap |

Follow the corresponding project README for external module power, common ground, and interface voltage requirements.

## 3. Memory and Download Layout

| Region | Address | Size | Use |
| --- | --- | --- | --- |
| CNVM | `0x08010000` | 1984 KiB | application vector table, startup, and primary code |
| ECNVM | `0x08200000` | 7680 KiB | extended program flash, as allocated by the linker script |
| ITCMRAM | `0x00000000` | 128 KiB | allocated by the linker script |
| DTCMRAM | `0x20000000` | 256 KiB | allocated by the linker script |
| AXISRAM | `0x24000000` | 768 KiB | data, BSS, stack, internal heap, and camera buffers |
| SRAM1 | `0x30004000` | 16 KiB | Ethernet DMA descriptors and buffers |
| SDRAM | `0xC0000000` | 32 MiB | display buffers and extended heap |

The application vector table uses `0x08010000`; a boot entry at `0x08000000` provides compatibility with different chip revisions. The default BIN image includes this boot entry and places the application vector table at offset `0x10000`. Download the complete BIN image at `0x08000000`.

With `BSP_USING_LCD_MIPI` enabled, the first 3 MiB of SDRAM are reserved and the extended heap starts at `0xC0300000`. Without LCD, the SDRAM heap starts at `0xC0000000`. Camera capture and preview buffers use internal AXI SRAM, separately from the SDRAM display area.

## 4. RT-Thread Studio

1. Install the GD32H77D-Gino BSP **v1.0.0** described by `sdk-bsp-gd32h77d-gino.yaml`.
2. Create an RT-Thread project based on the `GD32H77D-Gino` board.
3. Select `Gino_template` or one of the 15 examples.
4. Configure peripherals and packages in RT-Thread Settings, then build with GNU Arm Embedded 13.3.
5. Connect UART1 at 115200-8-N-1 and the SWD download adapter.
6. Select DAP-Link, target `GD32H77DIW`, and BIN mode; set the download start address to `0x08000000` and download `Debug/rtthread.bin`. These are the SDK's default download settings. Builds retain `Debug/rtthread.elf` for debugging.
7. Reset the board and run the selected example as described below.

Studio-created projects contain the shared source trees required by the selected example.

## 5. Env and MDK

### 5.1 Env with GCC

In an Env PowerShell session, enter the chosen project from the SDK root. For example:

```powershell
cd projects\Gino_template
.\mklinks.bat
$env:RTT_CC = "gcc"
$env:RTT_EXEC_PATH = "<gnu-arm-toolchain>\bin"
scons --pyconfig-silent
scons -j8
```

Replace the toolchain path with the directory containing `arm-none-eabi-gcc`. `mklinks.bat` creates links to the shared `rt-thread` and `libraries` directories. SCons produces `rt-thread.elf`, `rtthread.hex`, and `rtthread.bin`; download `rtthread.bin` at `0x08000000`.

After changing features with `menuconfig`, regenerate `rtconfig.h` using `scons --pyconfig-silent`. Run `scons --target=eclipse` when Eclipse project metadata needs regeneration.

### 5.2 MDK5 with Arm Compiler 6

Install the MDK dependencies listed in section 1. In an Env PowerShell session, enter the chosen project from the SDK root:

```powershell
cd projects\Gino_template
.\mklinks.bat
$env:RTT_CC = "keil"
$env:RTT_EXEC_PATH = "C:\Keil_v5"
scons --pyconfig-silent
scons --target=mdk5 --project-name=project
```

Adjust `RTT_EXEC_PATH` to the MDK installation root. Open `project.uvprojx` and build the target matching the project directory. Select `CMSIS-DAP Debugger` under `Options for Target -> Debug`, then select the connected DAP-Link and the SWD interface in `Settings`. Download `Objects/rt-thread.axf` or `Objects/rt-thread.hex` through DAP-Link. The project uses `board/linker_scripts/link.sct` and the `GD32H77x_78x_CNVM_2M.FLM` and `GD32H77x_78x_ECNVM_7M_512K.FLM` flash algorithms.

`template.uvprojx` and `template.uvoptx` hold the uVision defaults. SCons generates source groups, include paths, and defines from the current configuration. Regenerate the uVision project after changing configuration or source selection; generated source groups are overwritten.

## 6. Example Operation

The sidebar contains each example's README with its wiring, configuration, commands, and expected results. This table summarizes the prerequisites and first operation.

| Project | Prerequisite | Operation or expected behavior |
| --- | --- | --- |
| `Gino_template` | board power | PC4 blinks and the UART1 console is available |
| `Gino_driver_i2c` | pull-ups and an I2C target | `i2c scan hwi2c1 08 78` |
| `Gino_driver_spi` | SPI target and application-configured CS | use `spi3`; configure the target protocol in the application |
| `Gino_driver_qspi_flash` | onboard GD25Q64E | access files with `ls /flash` |
| `Gino_driver_sdcard` | FAT/FAT32 SD card | `ls /sd` |
| `Gino_driver_can` | CAN transceiver, termination, and peer | use `can1`; match bitrate and frame format to the peer |
| `Gino_driver_pwm` | external output connection as needed | `pwm_dual_test start`; PWM on PA7/PG7 |
| `Gino_driver_rtc` | RTC clock source | `date` |
| `Gino_driver_sdram` | onboard SDRAM | external memory is added to the heap; `free`, `list_memheap` |
| `Gino_driver_usb_device` | data-capable USB cable | host enumerates the configured USB class |
| `Gino_driver_usb_host` | FAT USB drive | `ls /udisk` |
| `Gino_driver_eth` | Ethernet cable and network | `ifconfig`, `ping <gateway>` |
| `Gino_display_lvgl` | LCD, touch, and SDRAM | LVGL demo starts automatically; operate it through touch |
| `Gino_display_camera` | OV7670, LCD, and SDRAM | automatic preview; `ov7670_preview start` / `ov7670_preview stop` |
| `Gino_component_mqtt` | configured Wi-Fi and broker | `gd32vw553_mqtt_start`, `gd32vw553_mqtt_pub <topic> <message>` |
| `Gino_factory` | enabled board peripherals | operate the integrated display, camera, storage, and network examples |

### Flash Filesystem

The `filesystem` FAL partition is mounted at `/flash`. For a new partition that needs formatting:

```text
mkfs -t elm filesystem
mount filesystem /flash elm
ls /flash
```

Formatting erases existing partition data. FatFs must allow a 4096-byte maximum sector size.

### Wi-Fi and MQTT

Configure the Wi-Fi SSID/password, broker address/port, topics, client ID, and optional authentication in RT-Thread Settings or `menuconfig`. Keep real credentials in local configuration. Use a numeric IPv4 broker address when module firmware does not support DNS through `AT+CIPDOMAIN`.

### Display and Camera

`Gino_display_lvgl` starts the LVGL demo automatically. `Gino_display_camera` requests preview after LVGL initialization and displays QVGA RGB565 frames. Use `ov7670_preview stop` and `ov7670_preview start` to control preview. LCD framebuffers remain in the reserved SDRAM area; application allocations must use the configured heap.
