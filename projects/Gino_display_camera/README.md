# Gino_display_camera

## 1. Introduction

This example provides OV7670 camera capture and live preview through LVGL and the MIPI LCD on the GD32H77D Gino board. Preview starts automatically after LVGL initialization.

The primary device is `ov7670`. UART1 is the FinSH/MSH console at 115200-8-N-1, and PC4 is the run indicator.

## 2. Capture and Display

I2C2/SCCB configures OV7670, and TIMER0 supplies XCLK on PE9. DCI receives the parallel pixel stream. DMA1 channel 7 writes a dedicated capture buffer in internal AXI SRAM. After cache maintenance and RGB565 processing, `rt_device_read` copies a QVGA frame into the application's destination buffer.

The preview thread uses two additional internal AXI SRAM buffers. It tracks buffers being written, ready, and displayed so the LVGL image source remains stable until rendering completes. LVGL renders through a 32-line draw buffer, IPA updates the back framebuffer in SDRAM, and the LCD driver swaps framebuffers during vertical blanking.

## 3. Memory Requirements

The DMA capture buffer and the two preview buffers are aligned to 32 bytes. LCD framebuffers use the reserved SDRAM area; with LCD enabled, the SDRAM heap starts at `0xC0300000`.

Board initialization completes SDRAM and MPU setup before enabling D-Cache. LCD use requires successful SDRAM initialization. A bootloader must clean and disable D-Cache before handing control to this firmware.

## 4. Device Interfaces

The `ov7670` device supports `rt_device_open` and `rt_device_read`. Allocate an output buffer large enough for a 320 x 240 RGB565 frame; a successful read returns the frame size in bytes.

The preview uses the `lcd` graphic device, the LVGL image API, and the LVGL thread for UI updates. GT911 supplies touch input when enabled.

## 5. Hardware

- OV7670: power, common ground, reset, XCLK, PCLK, VSYNC, HREF, D0-D7, and I2C2/SCCB connections.
- Camera control: I2C2 on PA8/PC9; XCLK on PE9.
- Display: MIPI LCD with SDRAM; GT911 touch on I2C3.
- Console: UART1 on PA2/PA3, AF7, 115200-8-N-1.
- Run LED: PC4, toggled every 500 ms.

Connect the camera with the board powered off and use the module's required supply and signal levels.

## 6. Example Operation

Source paths below are relative to the project directory in the SDK repository:

- `applications/main.c`
- `applications/lv_port.c`
- `applications/ov7670_preview.c`
- `../../libraries/Board_Drivers/drv_ov7670.c`
- `../../libraries/Board_Drivers/ports/camera/sensors/ov7670.c`

| Command | Function |
| --- | --- |
| `ov7670_preview start` | request live preview on the LCD |
| `ov7670_preview stop` | stop live preview and release the preview screen |

1. Connect the camera, LCD, power, and console.
2. Build and download this project using RT-Thread Studio or MDK5.
3. Reset the board and wait for the LCD to display the camera image automatically.
4. Use the start and stop commands when manual preview control is needed.

## 7. Expected Behavior

- LVGL initializes the display and starts OV7670 preview.
- The LCD displays continuous QVGA RGB565 camera images.
- The preview can be stopped and restarted from the console.
