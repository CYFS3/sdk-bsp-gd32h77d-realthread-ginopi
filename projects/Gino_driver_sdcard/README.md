# Gino_driver_sdcard

## 1. Introduction

This project is the GD32H77D Gino development board reference project for SDIO1 and SD card filesystem. It is used to learn, configure, and validate SD/SDIO Block Protocol separately. The standalone project enables only the drivers and components required by this feature, and can be used as a reference for application development and integration.

Primary device: `sd0`.

All examples keep `uart1` as the FinSH/MSH console at 115200-8-N-1, and PC4 LED is used as the run indicator.

Current project provides the following MSH commands:

| Command | Function | Example/Note |
| --- | --- | --- |
| `gino_device_probe` | Check whether the primary device used by this example is registered. | Checks `sd0` by default. |
| `ls /sd` | List files on the mounted SD card filesystem. | Requires a usable FAT/FAT32 card. |

## 2. SD/SDIO Block Protocol Details

After SDIO1 identifies the card and provides block I/O, it registers `sd0`. DFS/FatFs reads partition and filesystem metadata and mounts it at `/sd`. Card detect alone does not prove initialization or filesystem health.

The complete data path is:

```text
SD card -> SDIO1/card detect -> sd0 block device -> DFS/FatFs -> /sd
```

The protocol layer only defines communication and data processing rules. Final validation still depends on controller clocks, pin multiplexing, interrupt/DMA handling, and upper-layer state machines. Device discovery, bus registration, or a successful build cannot replace a complete data transfer test.

## 3. GD32H77D SDIO1 Features

The project enables GD32H77D SDIO1 and card detect. The controller handles commands, data lines, and block transfers. Active-low card detect indicates insertion only, not successful card or filesystem initialization.

## 4. RT-Thread SDIO, Block Device, and DFS Device Interface

After card discovery, the RT-Thread SDIO/MMCSd layer registers the `sd0` block device. DFS/elm-FatFs accesses sectors and mounts the filesystem at `/sd`.

Primary device: `sd0`. Use `list_device` and `gino_device_probe` first to check whether the device has been registered. Upper-layer file system, network, or GUI components still need separate validation for mount, link, or refresh state.

## 5. Hardware

SDIO1 registers sd0 and mounts a FAT/FAT32 filesystem at /sd. Insert a usable SD card before testing.

Default console: UART1, PA2/PA3, AF7, 115200-8-N-1.

Insert a usable FAT/FAT32 SD card before testing the mount point.

## 6. Example

Source paths below are relative to the project directory in the SDK repository:

- `../../libraries/Board_Drivers/drv_filesystem.c`
- `../../libraries/gd32_drivers/drv_sdio.c`
- `../../libraries/gd32_drivers/config/sdio_config.h`

Read `applications/main.c` and `applications/device_probe.c` first, then follow the data path into the corresponding driver, component, or package. The example keeps MSH commands so device registration and runtime state can be observed without changing application code.

### 6.1 Runtime Commands

- `gino_device_probe`
- `ls /sd`

### 6.2 Operation Steps

1. Check power, wiring, external modules, and interface logic levels.
2. Reset the development board and confirm that the UART1 console is available and PC4 LED blinks normally.
3. Run `list_device` and confirm that dependency buses and target devices are registered.
4. Run the commands above in order while observing return values, external waveforms, network state, or display results.

## 7. Runtime Results

### 7.1 Expected Behavior

- `sd0` appears after card insertion.
- `ls /sd` lists FAT/FAT32 files and directories.
- Insertion and removal update device and mount status according to the configuration without disturbing the UART1 console.
