# Gino_template

## 1. Introduction

This project is the GD32H77D Gino development board reference project for LED blink and secondary development. It is used to learn, configure, and validate GPIO and RTOS scheduling separately. The standalone project enables only the drivers and components required by this feature, and can be used as a reference for application development and integration.

This project has no standalone primary device node.

All examples keep `uart1` as the FinSH/MSH console at 115200-8-N-1, and PC4 LED is used as the run indicator.

Current project provides the following MSH commands:

| Command | Function | Example/Note |
| --- | --- | --- |
| `gino_device_probe` | Check the runtime probe state used by this example. | This template has no standalone primary device node. |
| `pin list` | List PIN device information. | Confirm GPIO/PIN registration. |

## 2. GPIO and RTOS Scheduling Details

After startup assembly initializes the data sections, RT-Thread configures clocks, console, system tick, and the scheduler, then enters the application's `main`. The application drives PC4 through the PIN device API and yields with `rt_thread_mdelay`; blinking therefore validates GPIO, tick timing, and scheduling together.

The complete data path is:

```text
reset -> board init -> RT-Thread scheduler -> main thread -> rt_pin_write -> PC4 LED
```

The protocol layer only defines communication and data processing rules. Final validation still depends on controller clocks, pin multiplexing, interrupt/DMA handling, and upper-layer state machines. Device discovery, bus registration, or a successful build cannot replace a complete data transfer test.

## 3. GD32H77D GPIO and SysTick Features

GD32H77D GPIO supports input, push-pull/open-drain output, and alternate functions. The run LED is on PC4. The BSP configures clocks, SysTick, the PIN driver, and UART1 in `board.c`, so the blink also checks tick and main-thread scheduling.

## 4. RT-Thread PIN and Thread Device Interface

The RT-Thread PIN API uses `rt_pin_mode` for direction and `rt_pin_write`/`rt_pin_read` for levels. The demo calls `rt_thread_mdelay(500)` to yield instead of busy-waiting.

This project has no standalone primary device node. Use `list_device` and `gino_device_probe` first to check the runtime state. Upper-layer file system, network, or GUI components still need separate validation for mount, link, or refresh state.

## 5. Hardware

No additional hardware is required. The PC4 LED validates the system tick, GPIO, and basic startup.

Default console: UART1, PA2/PA3, AF7, 115200-8-N-1.

## 6. Example

Source paths below are relative to the project directory in the SDK repository:

- `applications/main.c`
- `applications/device_probe.c`
- `../../libraries/Board_Drivers/board.c`
- `board/SConscript`

Read `applications/main.c` and `applications/device_probe.c` first, then follow the data path into the corresponding driver, component, or package. The example keeps MSH commands so device registration and runtime state can be observed without changing application code.

### 6.1 Runtime Commands

- `gino_device_probe`
- `pin list`

### 6.2 Operation Steps

1. Check power, wiring, external modules, and interface logic levels.
2. Reset the development board and confirm that the UART1 console is available and PC4 LED blinks normally.
3. Run `list_device` and confirm that dependency buses and target devices are registered.
4. Run the commands above in order while observing return values, external waveforms, network state, or display results.

## 7. Runtime Results

### 7.1 Expected Behavior

- UART1 prints the RT-Thread banner and project name.
- PC4 toggles every 500 ms, giving an approximately 1 s on/off cycle.
- `pin list` shows the PIN device and pin information.
