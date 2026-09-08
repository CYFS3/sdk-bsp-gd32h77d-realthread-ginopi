# GD32H77D Gino BSP Architecture

[中文](ARCHITECTURE_zh.md) | [Project guide](PROJECT_GUIDE.md) | [SDK overview](../README.md)

This document describes source organization, startup, configuration, device interfaces, and peripheral data flow for application development.

## 1. Source Organization

The SDK provides 16 projects that share RT-Thread, GD32 libraries, board drivers, and offline packages.

| Directory | Responsibility |
| --- | --- |
| `projects/<name>/applications` | example entry, application threads, and component ports |
| `projects/<name>/board` | project Kconfig, board build entry, and linker scripts |
| `libraries/Board_Drivers` | shared board initialization, LCD, touch, camera, Ethernet, and filesystem integration |
| `libraries/gd32_drivers` | RT-Thread peripheral drivers and common peripheral configuration |
| `libraries/gd32-arm-cmsis-latest` | CMSIS, startup code, and system initialization |
| `libraries/gd32-arm-series-latest` | GD32 peripheral firmware library |
| `rt-thread` | kernel, device framework, components, and build tools |
| `packages` | offline LVGL, GT911, at_device, and kawaii-mqtt packages |

Each project owns its `.config`, generated `rtconfig.h`, application sources, Studio metadata, and MDK files. Its `board/SConscript` loads the shared `libraries/Board_Drivers/SConscript`. Start development from `Gino_template` or the closest peripheral example; `Gino_factory` combines the board features in one application.

## 2. Startup and Memory

Startup assembly calls `SystemInit`, prepares the C runtime, and enters RT-Thread. The application vector table is at `0x08010000`; the boot entry at `0x08000000` provides compatibility with different chip revisions. See the project guide for the memory map and download formats.

Shared `libraries/Board_Drivers/board.c` performs the following board initialization:

1. Select the application vector table and enable I-Cache.
2. Configure SysTick and UART, then run board-level initialization, including SDRAM when enabled.
3. Select the console and configure display-memory MPU regions when LCD is enabled.
4. Enable D-Cache and initialize the internal heap.

Entry into board initialization requires D-Cache to be disabled. A bootloader must clean and disable D-Cache before handing control to the application. LCD use requires successful SDRAM initialization.

RT-Thread initializes scheduling and system threads, then runs component initialization in the main thread. `INIT_*_EXPORT()` places initialization entries in `.rti_fn.<level>` sections: board level `1`, device level `3`, component level `4`, and environment level `5`. Device registration precedes dependent application use, filesystem mounting, and LVGL access.

## 3. Configuration and Build

The build uses the following inputs:

- `Kconfig` and `.config`: feature selection.
- `rtconfig.h`: configuration macros generated for C and assembly sources.
- `SConstruct` and `SConscript`: source groups, dependencies, include paths, and compiler options.
- `board/linker_scripts/link.ld`: GCC memory and section placement.
- `board/linker_scripts/link.sct`: MDK memory and section placement.
- `template.uvprojx` and `template.uvoptx`: uVision project and download defaults.

Use RT-Thread Settings or `menuconfig` for configuration changes, then regenerate `rtconfig.h`. `scons --target=eclipse` exports Studio/Eclipse metadata; `scons --target=mdk5` exports the current build graph to uVision. Generated source groups should be maintained through SCons inputs.

The GCC startup source is `startup_gd32h77x_78x.S`. Uppercase `.S` enables preprocessing, including access to `rtconfig.h`. Eclipse metadata supplies include paths for C, C++, and assembly and uses `rtconfig_preinc.h` for internal RT-Thread definitions.

## 4. Device Interfaces

Applications find registered RT-Thread devices by name, open them with `rt_device_open`, and use the corresponding device-class read, write, or control interfaces.

| Function | Device or interface |
| --- | --- |
| Console | `uart1` |
| I2C example | `hwi2c1` |
| SPI example | `spi3` |
| CAN | `can1` |
| LCD and touch | `lcd`, `gt911` |
| Camera | `ov7670` |
| Ethernet and Wi-Fi | `e0`, `wifi0` netdevs and SAL sockets |
| Storage | `qspi_flash0`, `sd0`, DFS mounts |

The LVGL RT-Thread port creates an `LVGL` thread. It initializes LVGL, display, input, and application UI, then repeatedly calls `lv_timer_handler`. Keep LVGL object operations in this thread or follow the port's synchronization rules. Interrupt handlers notify threads or complete peripheral transfers; application processing belongs in thread context.

## 5. Cache, DMA, and Buffers

The Cortex-M7 CPU and DMA engines share memory. Buffers use cache-line alignment and explicit ownership:

- Clean CPU-written cache lines before hardware reads the data.
- Prepare DMA destination buffers before capture and invalidate them before the CPU consumes the result.
- Reuse a buffer only after its previous hardware or display consumer has completed.

| Buffer | Location and use |
| --- | --- |
| LCD framebuffers | reserved SDRAM beginning at `0xC0000000` |
| LVGL partial draw buffers | internal AXI SRAM |
| OV7670 capture and preview buffers | internal AXI SRAM, aligned to 32 bytes |
| Ethernet DMA descriptors and buffers | `.enet_dma` in SRAM1 at `0x30004000` |
| Extended heap with LCD enabled | SDRAM beginning at `0xC0300000` |

Board initialization configures MPU attributes before enabling D-Cache. It protects unused external address windows and makes valid SDRAM accessible through higher-priority regions. OSPI0 flash uses indirect transfers; applications that add memory-mapped external devices must configure accessible MPU regions for those devices.

The 3 MiB SDRAM reservation remains separate from the heap. Display memory attributes are selected by the LCD/camera configuration in `board.c`; retain these boundaries when extending the application.

## 6. Peripheral Data Flow

### Display and Touch

LVGL renders RGB565 pixels, IPA copies the rendered regions to SDRAM framebuffers, and TLI/DSI drives the 720 x 720 FL7707N panel. GT911 reports I2C3 touch input through the RT-Thread touch device to LVGL.

`Gino_display_lvgl` uses two 200-line partial draw buffers and asynchronous IPA copies into a single scanout framebuffer. The camera-enabled display and factory projects use a 32-line draw buffer, IPA updates to a back framebuffer, and vertical-blank framebuffer swaps. The flush callback releases each LVGL buffer only after its transfer is complete.

### Camera

I2C2/SCCB configures OV7670 and TIMER0 supplies XCLK. DCI receives parallel pixels, and DMA1 channel 7 writes the internal AXI SRAM capture buffer. The driver performs cache maintenance and RGB565 frame processing, then returns image data through `rt_device_read`.

The preview application uses a capture thread and two internal preview buffers. It tracks buffers being written, ready, or displayed so LVGL retains a stable image source until rendering completes. The image is then rendered through the LCD path. `Gino_display_camera` starts preview after LVGL initialization.

### Ethernet and Wi-Fi

Ethernet traffic follows SAL sockets, lwIP, the RT-Thread Ethernet device, ENET DMA, and the RMII PHY. The driver transfers frames between lwIP pbufs and DMA buffers and notifies the receive thread from the interrupt handler.

Wi-Fi traffic follows SAL sockets, AT sockets, at_device/at_client, UART4, and GD32VW553. The module handles Wi-Fi and TCP/IP; the H7 sends AT commands and processes responses and unsolicited events.

### Storage

OSPI0 accesses the GD25Q64E NOR flash. FAL exposes the flash device and partitions; a block adapter and DFS/FatFs provide the `/flash` filesystem. SD and USB mass storage provide block devices mounted at `/sd` and `/udisk`. Filesystem mounting follows device initialization and requires a supported filesystem on the media.

### MQTT

The MQTT example waits for `wifi0`, temporarily selects it as the default SAL network device, and uses kawaii-mqtt to connect, subscribe, and publish. Wi-Fi and broker settings come from project configuration. The example uses QoS 0, which does not provide delivery acknowledgement or retry guarantees.

## 7. Integrated Application

`Gino_factory` combines LVGL, camera, storage, Ethernet, Wi-Fi/MQTT, and common peripherals. Enabled features share memory, buses, interrupt resources, and network routing. Keep per-device configuration consistent with its standalone example, preserve buffer ownership, and select the intended network device for socket traffic.

## 8. Code Map

Paths below are relative to the SDK root. For a Studio-created project, the selected shared directories are copied into that project.

| Topic | Starting point |
| --- | --- |
| GCC startup | `libraries/gd32-arm-cmsis-latest/GD32H77x_78x/GD/GD32H77x_78x/Source/GCC/startup_gd32h77x_78x.S` |
| Board initialization and memory definitions | `libraries/Board_Drivers/board.c`, `libraries/Board_Drivers/board.h` |
| RT-Thread initialization | `rt-thread/src/components.c`, `rt-thread/include/rtdef.h` |
| Build | `projects/Gino_template/SConstruct`, `rt-thread/tools/building.py` |
| LCD and touch | `libraries/Board_Drivers/drv_lcd.c`, `libraries/Board_Drivers/drv_gt911.c` |
| LVGL | `projects/Gino_display_lvgl/applications/lv_port.c`, `packages/LVGL-latest/env_support/rt-thread/lv_rt_thread_port.c` |
| Camera | `libraries/Board_Drivers/drv_ov7670.c`, `projects/Gino_display_camera/applications/ov7670_preview.c` |
| Ethernet | `libraries/Board_Drivers/drv_enet.c` |
| AT Wi-Fi | `projects/Gino_component_mqtt/applications/gd32vw553_at_port.c`, `packages/at_device-latest/class/gd32vw553/` |
| MQTT | `projects/Gino_component_mqtt/applications/gd32vw553_mqtt_demo.c` |
| Integrated UI | `projects/Gino_factory/applications/board_demo.c`, `projects/Gino_factory/applications/board_demo_backend.c` |
