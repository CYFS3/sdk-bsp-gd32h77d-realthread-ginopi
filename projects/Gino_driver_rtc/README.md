# Gino_driver_rtc

## 1. Introduction

This project is the GD32H77D Gino development board reference project for On-chip RTC and alarm. It is used to learn, configure, and validate RTC and Alarm separately. The standalone project enables only the drivers and components required by this feature, and can be used as a reference for application development and integration.

Primary device: `rtc`.

All examples keep `uart1` as the FinSH/MSH console at 115200-8-N-1, and PC4 LED is used as the run indicator.

Current project provides the following MSH commands:

| Command | Function | Example/Note |
| --- | --- | --- |
| `gino_device_probe` | Check whether the primary device used by this example is registered. | Checks `rtc` by default. |
| `date` | Read or set the RTC time through the MSH date command. | Run repeatedly to confirm seconds advance. |
| `rtc_alarm [seconds]` | Start a one-shot RTC alarm. | Default: 5 seconds; range: 1..86400. Replaces the previous example alarm. |
| `rtc_alarm status` | Show the example alarm state and deadline. | Times are displayed in the local timezone. |
| `rtc_alarm stop` | Cancel and delete the example alarm. | Does not delete alarms owned by other applications. |

## 2. RTC and Alarm Details

The RTC uses a low-speed clock and backup domain. RT-Thread exposes it as `rtc` and `date` accesses it through the common RTC API. Alarm events come from RTC compare logic instead of main-loop polling.

The complete data path is:

```text
low-speed clock/backup domain -> on-chip RTC -> rt_device rtc -> date/alarm
```

The protocol layer only defines communication and data processing rules. Final validation still depends on controller clocks, pin multiplexing, interrupt/DMA handling, and upper-layer state machines. Device discovery, bus registration, or a successful build cannot replace a complete data transfer test.

## 3. GD32H77D RTC and Backup Domain Features

The on-chip RTC runs from the low-speed clock and backup domain and can continue while the CPU is stopped. Alarm compare logic can interrupt or wake the system; retention depends on backup-domain and VBAT conditions.

## 4. RT-Thread RTC and Alarm Device Interface

RT-Thread registers `rtc`. RTC device-control operations back POSIX time functions and the MSH `date` command. The alarm component manages alarm objects and callbacks.

Primary device: `rtc`. Use `list_device` and `gino_device_probe` first to check whether the device has been registered. Upper-layer file system, network, or GUI components still need separate validation for mount, link, or refresh state.

## 5. Hardware

Validates the on-chip RTC and alarm. Retention depends on the RTC clock, backup domain, and backup power.

Default console: UART1, PA2/PA3, AF7, 115200-8-N-1.

RTC retention depends on the low-speed clock, backup domain, and VBAT conditions.

## 6. Example

Source paths below are relative to the project directory in the SDK repository:

- `applications/device_probe.c`
- `applications/rtc_alarm_example.c`
- `../../libraries/gd32_drivers/drv_rtc.c`
- `board/Kconfig`

Read `applications/main.c` and `applications/device_probe.c` first, then follow the data path into the corresponding driver, component, or package. The example keeps MSH commands so device registration and runtime state can be observed without changing application code.

### 6.1 Runtime Commands

- `gino_device_probe`
- `date`
- `rtc_alarm 5`
- `rtc_alarm status`
- `rtc_alarm stop`

```text
date 2026 09 08 12 00 00
rtc_alarm 5
rtc_alarm status
```

After approximately five seconds, ulog prints a single `[I/rtc.alarm] Triggered:` message with the local date and time. `rtc_alarm` without arguments also schedules an alarm five seconds ahead; `rtc_alarm help` prints command usage. To test cancellation, run `rtc_alarm 30` followed by `rtc_alarm stop` before it expires.

The command uses `rt_alarm_create()` and `rt_alarm_start()` in one-shot mode. The RTC Alarm0 interrupt notifies the RT-Thread Alarm service, which prints the callback log from its thread. The example enables the ulog console backend at INFO level and uses a 2048-byte Alarm service stack. Application messages also use ulog; the built-in `date` and `list_device` commands keep their standard shell output.

Alarm deadlines are derived from the RTC time; avoid changing `date` while an alarm is armed. The example alarm object is held in RAM and must be recreated after a reset, even when RTC time is retained.

### 6.2 Operation Steps

1. Check power, wiring, external modules, and interface logic levels.
2. Reset the development board and confirm that the UART1 console is available and PC4 LED blinks normally.
3. Run `list_device` and confirm that dependency buses and target devices are registered.
4. Run the commands above in order while observing return values, external waveforms, network state, or display results.

## 7. Runtime Results

### 7.1 Expected Behavior

- `gino_device_probe` opens `rtc`.
- Repeated `date` calls show increasing seconds.
- `rtc_alarm 5` produces one ulog trigger message; a stopped alarm does not trigger.
- Retention across reset matches the backup-domain and VBAT design.
