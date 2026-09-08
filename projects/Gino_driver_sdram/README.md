# Gino_driver_sdram

## 1. Introduction

This project is the GD32H77D Gino development board reference project for 32 MiB external SDRAM. It is used to learn, configure, and validate SDRAM Timing, Refresh, and Heap Extension separately. The standalone project enables only the drivers and components required by this feature, and can be used as a reference for application development and integration.

This project has no standalone primary device node.

All examples keep `uart1` as the FinSH/MSH console at 115200-8-N-1, and PC4 LED is used as the run indicator.

Current project provides the following MSH commands:

| Command | Function | Example/Note |
| --- | --- | --- |
| `gino_device_probe` | Check whether the primary device used by this example is registered. | Checks the primary device by default. |
| `free` | Print RT-Thread heap usage. | Confirm SDRAM has extended available heap. |
| `sdram_test` | Measure SDRAM read/write speed and verify data, showing elapsed time and MiB/s. | Allocate a 1 MiB buffer from the SDRAM heap, run 8 passes per operation, then free it. |

## 2. SDRAM Timing, Refresh, and Heap Extension Details

The SDRAM controller must match bank, row/column, CAS, and refresh parameters. Once initialized, 32 MiB starts at `0xC0000000` and extends the heap. It has no standalone `rt_device`, so memory statistics and stress checks are used.

The complete data path is:

```text
system clock -> EXMC SDRAM timing/refresh -> 0xC0000000 memory -> RT-Thread heap
```

The protocol layer only defines communication and data processing rules. Final validation still depends on controller clocks, pin multiplexing, interrupt/DMA handling, and upper-layer state machines. Device discovery, bus registration, or a successful build cannot replace a complete data transfer test.

## 3. GD32H77D EXMC and SDRAM Features

The GD32H77D external memory controller configures SDRAM bank, row/column, CAS latency, and refresh. The 32 MiB device starts at `0xC0000000` and is sensitive to timing and bus contention.

## 4. RT-Thread Memory Management Device Interface

SDRAM has no standalone `rt_device`. After BSP initialization it extends the heap, so applications use it through `rt_malloc`, `rt_calloc`, and `rt_free` and inspect total and remaining capacity with `free`.

This project has no standalone primary device node. Use `list_device` and `gino_device_probe` first to check the runtime state. Upper-layer file system, network, or GUI components still need separate validation for mount, link, or refresh state.

## 5. Hardware

Initializes 32 MiB SDRAM at 0xC0000000 and extends the system heap; no standalone device node is expected.

Default console: UART1, PA2/PA3, AF7, 115200-8-N-1.

If display or camera features are later enabled, keep framebuffer/camera reservations out of the heap.

## 6. Example

Source paths below are relative to the project directory in the SDK repository:

- `../../libraries/Board_Drivers/drv_sdram_port.c`
- `../../libraries/gd32_drivers/drv_sdram.c`
- `../../libraries/Board_Drivers/board.h`
- `board/SConscript`

Read `applications/main.c` and `applications/device_probe.c` first, then follow the data path into the corresponding driver, component, or package. The example keeps MSH commands so device registration and runtime state can be observed without changing application code.

### 6.1 Runtime Commands

- `gino_device_probe`
- `free`
- `sdram_test`

`sdram_test` takes no arguments and accesses only its allocated SDRAM heap buffer, preserving heap metadata, existing allocations, and display/camera reservations. It reports an error if allocation or SDRAM initialization has failed.

The output table shows sequential `Write` and `Read` speeds: `Total KiB` is the total data accessed per operation, `Time ms` is elapsed time, and `MiB/s` is throughput (1 MiB = 1024 * 1024 bytes). Each operation transfers 8 MiB in total. Data verification runs separately and prints `Verify: PASS` or the first failing address with its expected and actual values.

Measurements use the current CPU, cache, and SDRAM timing configuration. Write timing includes a D-Cache clean on every pass; read timing includes an invalidate on every pass to fetch data from SDRAM. Results include loop and scheduling overhead and represent effective CPU sequential throughput, not theoretical SDRAM bus bandwidth. Measurements shorter than one system tick show `N/A`.

### 6.2 Operation Steps

1. Check power, wiring, external modules, and interface logic levels.
2. Reset the development board and confirm that the UART1 console is available and PC4 LED blinks normally.
3. Run `list_device` and confirm that dependency buses and target devices are registered.
4. Run the commands above in order while observing return values, external waveforms, network state, or display results.

## 7. Runtime Results

### 7.1 Expected Behavior

- The startup log reports SDRAM initialization.
- `free` shows substantially more heap than internal-RAM projects.
- `sdram_test` reports elapsed time and MiB/s for `Write` and `Read`, followed by `Verify: PASS`.
- Large allocate/write/verify/free cycles preserve data.
