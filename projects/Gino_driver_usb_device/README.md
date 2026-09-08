# Gino_driver_usb_device

## 1. Introduction

This project is the GD32H77D Gino development board reference project for USBHS0 device. It is used to learn, configure, and validate USB Device Enumeration and CDC separately. The standalone project enables only the drivers and components required by this feature, and can be used as a reference for application development and integration.

Primary device: `usbd`.

CDC data device: `vcom`. After the PC opens the USB COM port with DTR enabled, the application sends a numbered test message every second and echoes received bytes.

All examples keep `uart1` as the FinSH/MSH console at 115200-8-N-1, and PC4 LED is used as the run indicator.

Current project provides the following MSH commands:

| Command | Function | Example/Note |
| --- | --- | --- |
| `gino_device_probe` | Check whether the primary device used by this example is registered. | Checks `usbd` by default. |
| `list_device` | List registered system devices. | Confirm that UART, PIN, buses, and target devices exist. |

## 2. USB Device Enumeration and CDC Details

USBHS0 acts as a device, responding to reset and enumeration with device, configuration, interface, and endpoint descriptors. The configuration enables the RT-Thread USB device stack and CDC composite support. The PC loads the corresponding driver based on these descriptors.

The complete data path is:

```text
PC host -> Type-C D+/D- -> USBHS0 device -> RT-Thread USB device/CDC -> application endpoints
```

The protocol layer only defines communication and data processing rules. Final validation still depends on controller clocks, pin multiplexing, interrupt/DMA handling, and upper-layer state machines. Device discovery, bus registration, or a successful build cannot replace a complete data transfer test.

## 3. GD32H77D USBHS0 Device Features

GD32H77D USBHS0 is configured as a device on the Type-C PA11/PA12 interface. It responds to host reset and endpoint transactions; the RT-Thread configuration enables device, composite, and CDC support.

## 4. RT-Thread USB Device Interface

The RT-Thread USB device stack defines devices, configurations, interfaces, and endpoints through descriptors and dispatches class requests to CDC. It is not a direct replacement for a plain character `rt_device`; applications normally exchange data through USB class callbacks or class-specific interfaces.

The controller is registered as `usbd`, and the CDC serial interface is registered as `vcom`. The application opens `vcom` with `rt_device_open()`, reads and echoes data with `rt_device_read()` / `rt_device_write()`, and checks `RT_USBD_CLASS_CTRL_CONNECTED` before sending. This state follows the DTR signal set by the PC serial tool. `rt_kprintf()` and FinSH/MSH continue to use `uart1`.

## 5. Hardware

USBHS0 device uses the Type-C connector and PA11/PA12. Use a data-capable USB cable.

Default console: UART1, PA2/PA3, AF7, 115200-8-N-1.

Use a data-capable cable and confirm the PC actually enumerates the device instead of only powering the board.

## 6. Example

Source paths below are relative to the project directory in the SDK repository:

- `../../libraries/gd32_drivers/usb/drv_otghs_dev.c`
- `../../libraries/gd32_drivers/usb/drv_usb_dev.c`
- `../../libraries/gd32_drivers/usb/gd32h7_usb_hw.c`
- `rtconfig.h`

Read `applications/main.c` and `applications/device_probe.c` first, then follow the data path into the corresponding driver, component, or package. The example keeps MSH commands so device registration and runtime state can be observed without changing application code.

### 6.1 Runtime Commands

- `gino_device_probe`
- `list_device`

### 6.2 Operation Steps

1. Check power, wiring, external modules, and interface logic levels.
2. Reset the development board and confirm that the UART1 console is available and PC4 LED blinks normally.
3. Run `list_device` on UART1 and confirm that both `usbd` and `vcom` are registered. `gino_device_probe` checks the `usbd` controller.
4. Connect the USBHS0 Type-C port to the PC, open the enumerated USB COM port at 115200-8-N-1, and enable DTR in the serial tool.
5. Check for one numbered test message per second. Send a short string such as `hello\r\n` and confirm that it is echoed; disable local echo in the PC tool to avoid mistaking local input for a device response.
6. Close and reopen the USB COM port with DTR enabled, then confirm that periodic messages resume. This is a low-rate communication example; the receive buffer is 128 bytes, so use short packets for the echo test.

## 7. Runtime Results

### 7.1 Expected Behavior

- A data-capable cable lets the PC enumerate a USB/CDC device.
- The device does not repeatedly connect/disconnect or remain unknown.
- UART1 remains available for enumeration logs.
- With the USB COM port open and DTR enabled, one message is received per second:

```text
Gino USB CDC test: 0
Gino USB CDC test: 1
Gino USB CDC test: 2
```

- Received bytes are echoed unchanged. Periodic messages share the same COM port with echo data.
- Transmission pauses while DTR is cleared or USB is disconnected and resumes after reopening the port with DTR enabled.
