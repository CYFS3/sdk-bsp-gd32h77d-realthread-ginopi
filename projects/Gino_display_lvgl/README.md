# Gino_display_lvgl

## 1. Introduction

This project is the GD32H77D Gino development board reference project for MIPI DSI LCD, GT911 and LVGL graphics. It is used to learn, configure, and validate LVGL Rendering, Input, and Double Buffering separately. The standalone project enables only the drivers and components required by this feature, and can be used as a reference for application development and integration.

Primary device: `lcd`.

All examples keep `uart1` as the FinSH/MSH console at 115200-8-N-1, and PC4 LED is used as the run indicator.

Current project provides the following MSH commands:

| Command | Function | Example/Note |
| --- | --- | --- |
| `gino_device_probe` | Check whether the primary device used by this example is registered. | Checks `lcd` by default. |

## 2. LVGL Rendering, Input, and Double Buffering Details

A dedicated LVGL thread initializes objects, display, and touch and periodically calls `lv_timer_handler` to process animations, input, and refreshes. This project uses two 200-line partial draw buffers in internal AXI SRAM. IPA asynchronously copies rendered rectangles to a single scanout framebuffer in SDRAM and releases each draw buffer after transfer completion.

The complete data path is:

```text
GT911 input -> LVGL objects/events -> RGB565 buffer -> cache clean/IPA -> TLI/DSI -> LCD
```

The protocol layer only defines communication and data processing rules. Final validation still depends on controller clocks, pin multiplexing, interrupt/DMA handling, and upper-layer state machines. Device discovery, bus registration, or a successful build cannot replace a complete data transfer test.

## 3. GD32H77D SDRAM, TLI/DSI, and I2C3 Features

The project combines SDRAM, TLI, DSI, IPA, I2C3, and timer PWM. TLI/DSI scan framebuffers, GT911 supplies touch, IPA accelerates copies, and M7 D-Cache must be cleaned before hardware reads.

Board initialization protects unused external address windows with MPU region 0; higher-priority regions keep valid SDRAM accessible. Mapped OSPI requires separate accessible MPU regions before this guard can be used.

D-Cache remains disabled until SDRAM initialization and display-memory MPU setup finish. The LCD uses the MPU and cache attributes configured during board initialization. A bootloader must clean and disable D-Cache before handing control to this firmware.

## 4. RT-Thread LVGL, Graphic, and Touch Device Interface

The RT-Thread LVGL port creates an `LVGL` thread that calls `lv_timer_handler`. It uses the `lcd` graphic and `gt911` touch devices, with `lv_display_set_buffers` and a flush callback coordinating buffer handoff between LVGL and the BSP.

Primary device: `lcd`. Use `list_device` and `gino_device_probe` first to check whether the device has been registered. Upper-layer file system, network, or GUI components still need separate validation for mount, link, or refresh state.

## 5. Hardware

Combines the LCD, GT911, SDRAM, and the offline LVGL package.

Default console: UART1, PA2/PA3, AF7, 115200-8-N-1.

Interact with the screen and check that animations and touch responses remain stable.

## 6. Example

Source paths below are relative to the project directory in the SDK repository:

- `applications/lv_port.c`
- `applications/lv_conf.h`
- `../../libraries/Board_Drivers/drv_lcd.c`
- `../../packages/LVGL-latest/env_support/rt-thread/lv_rt_thread_port.c`

Read `applications/main.c` and `applications/device_probe.c` first, then follow the data path into the corresponding driver, component, or package. The example keeps MSH commands so device registration and runtime state can be observed without changing application code.

### 6.1 Runtime Commands

- `gino_device_probe`

### 6.2 Operation Steps

1. Check power, wiring, external modules, and interface logic levels.
2. Reset the development board and confirm that the UART1 console is available and PC4 LED blinks normally.
3. Run `list_device` and confirm that dependency buses and target devices are registered.
4. Run the commands above in order while observing return values, external waveforms, network state, or display results.

## 7. Runtime Results

### 7.1 Expected Behavior

- The LVGL demo appears and touch operates controls.
- Draw buffers alternate while IPA transfers screen updates to the LCD.
