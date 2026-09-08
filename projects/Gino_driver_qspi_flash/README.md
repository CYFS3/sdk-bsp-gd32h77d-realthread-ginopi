# Gino_driver_qspi_flash

## 1. Introduction

This is the GD32H77D Gino QSPI flash reference project for validating the onboard GD25Q64E connected through OSPI0. The standalone project enables only the drivers and components required by this feature, and can be used as a reference for application development and integration.

Primary device: `qspi_flash0`.

All examples keep `uart1` as the FinSH/MSH console at 115200-8-N-1, and PC4 LED is used as the run indicator.

Current project provides the following MSH commands:

| Command | Function | Example/Note |
| --- | --- | --- |
| `gino_device_probe` | Check whether the primary device used by this example is registered. | Checks `qspi_flash0` by default. |
| `fal probe filesystem` | Probe the FAL `filesystem` partition. | Confirm partition geometry before filesystem access. |
| `fal read 0 16` | Read data from the current FAL device. | Use for a basic flash read check. |

## 2. QSPI NOR Flash and Filesystem Details

OSPI0 uses quad data phases to access the GD25Q64E NOR flash. The driver supplies read/program/erase operations, FAL exposes the `filesystem` partition, and a block adapter lets DFS/FatFs mount it at `/flash`. NOR erase and busy-state rules still apply.

The complete data path is:

```text
GD25Q64E -> OSPI0/MDMA -> qspi_flash0 -> FAL filesystem -> block device -> FatFs -> /flash
```

The protocol layer only defines communication and data processing rules. Final validation still depends on controller clocks, pin multiplexing, interrupt/DMA handling, and upper-layer state machines. Device discovery, bus registration, or a successful build cannot replace a complete data transfer test.

## 3. GD32H77D OSPI0 and MDMA Features

GD32H77D OSPI0 supports multi-line command/address/data phases and MDMA-assisted transfers. The onboard GD25Q64E is 8 MiB, with typical 4 KiB erase sectors and 256-byte program pages.

## 4. RT-Thread FAL, Block Device, and DFS Device Interface

The driver registers `qspi_flash0`. FAL uses `fal_init` and `fal_partition_find` to locate the `filesystem` partition, then exposes a block device through `fal_blk_device_create`, which DFS/FatFs mounts at `/flash`.

Primary device: `qspi_flash0`. Use `list_device` and `gino_device_probe` first to check whether the device has been registered. Upper-layer file system, network, or GUI components still need separate validation for mount, link, or refresh state.

## 5. Hardware

The onboard 8 MiB GD25Q64E is registered as qspi_flash0; the filesystem FAL partition is mounted at /flash and may need initial formatting.

Default console: UART1, PA2/PA3, AF7, 115200-8-N-1.

An unformatted partition may need formatting before `/flash` can mount; formatting destroys existing contents.

## 6. Example

Source paths below are relative to the project directory in the SDK repository:

- `../../libraries/Board_Drivers/drv_qspi_flash.c`
- `../../libraries/Board_Drivers/drv_filesystem.c`
- `../../libraries/Board_Drivers/fal_cfg.h`
- `../../libraries/gd32_drivers/drv_ospi.c`

Read `applications/main.c` and `applications/device_probe.c` first, then follow the data path into the corresponding driver, component, or package. The example keeps MSH commands so device registration and runtime state can be observed without changing application code.

### 6.1 Runtime Commands

- `gino_device_probe`
- `fal probe filesystem`
- `fal read 0 16`

### 6.2 Operation Steps

1. Check power, wiring, external modules, and interface logic levels.
2. Reset the development board and confirm that the UART1 console is available and PC4 LED blinks normally.
3. Run `list_device` and confirm that dependency buses and target devices are registered.
4. Run the commands above in order while observing return values, external waveforms, network state, or display results.

## 7. Runtime Results

### 7.1 Expected Behavior

- `gino_device_probe` finds `qspi_flash0`.
- `fal probe filesystem` reports partition geometry.
- `fal read 0 16` is stable, and a formatted partition mounts at `/flash`.
