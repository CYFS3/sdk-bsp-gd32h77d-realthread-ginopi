# Gino_driver_usb_host

## 1. Introduction

This project is the GD32H77D Gino development board reference project for USBHS1 host and mass-storage. It is used to learn, configure, and validate USB Host Enumeration and Mass Storage separately. The standalone project enables only the drivers and components required by this feature, and can be used as a reference for application development and integration.

Primary device: `usbh`.

All examples keep `uart1` as the FinSH/MSH console at 115200-8-N-1, and PC4 LED is used as the run indicator.

Current project provides the following MSH commands:

| Command | Function | Example/Note |
| --- | --- | --- |
| `gino_device_probe` | Check whether the primary device used by this example is registered. | Checks `usbh` by default. |
| `list_device` | List registered system devices. | Confirm that UART, PIN, buses, and target devices exist. |
| `ls /udisk` | List files on the mounted USB mass-storage filesystem. | Requires an enumerated FAT/FAT32 drive. |

## 2. USB Host Enumeration and Mass Storage Details

USBHS1 acts as host and PF2 controls downstream VBUS. The host stack detects, resets, reads descriptors, and selects a class driver. Mass storage then becomes a block device mounted by DFS/FatFs at `/udisk`.

The complete data path is:

```text
PF2 VBUS -> USB-A drive -> USBHS1 enumeration -> mass storage -> block device -> FatFs -> /udisk
```

The protocol layer only defines communication and data processing rules. Final validation still depends on controller clocks, pin multiplexing, interrupt/DMA handling, and upper-layer state machines. Device discovery, bus registration, or a successful build cannot replace a complete data transfer test.

## 3. GD32H77D USBHS1 Host Features

GD32H77D USBHS1 operates as host, with PF2 controlling USB-A VBUS. The controller handles reset, control transfers, and pipes while the RT-Thread host stack parses descriptors and selects class drivers.

## 4. RT-Thread USB Host and DFS Device Interface

The mass-storage class registers a block device, which DFS/FatFs mounts at `/udisk`. Applications use DFS/POSIX file APIs instead of raw USB pipes.

Primary device: `usbh`. Use `list_device` and `gino_device_probe` first to check whether the device has been registered. Upper-layer file system, network, or GUI components still need separate validation for mount, link, or refresh state.

## 5. Hardware

USBHS1 host uses the USB-A connector, with VBUS controlled by PF2; compatible drives mount at /udisk.

Default console: UART1, PA2/PA3, AF7, 115200-8-N-1.

Measure VBUS on the USB-A connector first if no device is enumerated.

## 6. Example

Source paths below are relative to the project directory in the SDK repository:

- `../../libraries/gd32_drivers/usb/drv_otghs_host.c`
- `../../libraries/gd32_drivers/usb/drv_usb_host.c`
- `../../libraries/gd32_drivers/usb/gd32h7_usb_hw.c`
- `../../libraries/Board_Drivers/drv_filesystem.c`

Read `applications/main.c` and `applications/device_probe.c` first, then follow the data path into the corresponding driver, component, or package. The example keeps MSH commands so device registration and runtime state can be observed without changing application code.

### 6.1 Runtime Commands

- `gino_device_probe`
- `list_device`
- `ls /udisk`

### 6.2 Operation Steps

1. Check power, wiring, external modules, and interface logic levels.
2. Reset the development board and confirm that the UART1 console is available and PC4 LED blinks normally.
3. Run `list_device` and confirm that dependency buses and target devices are registered.
4. Run the commands above in order while observing return values, external waveforms, network state, or display results.

## 7. Runtime Results

### 7.1 Expected Behavior

- A compatible drive produces enumeration and mass-storage logs.
- Host/storage devices appear in `list_device`.
- `ls /udisk` lists FAT/FAT32 files.
