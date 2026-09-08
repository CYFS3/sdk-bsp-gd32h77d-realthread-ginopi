# Gino_driver_pwm

## 1. Introduction

This project is the GD32H77D Gino development board reference project for PWM2 and PWM30 dual-channel output. It is used to learn, configure, and validate PWM Timer Output separately. The standalone project enables only the drivers and components required by this feature, and can be used as a reference for application development and integration.

Primary device: `pwm2`.

All examples keep `uart1` as the FinSH/MSH console at 115200-8-N-1, and PC4 LED is used as the run indicator.

Current project provides the following MSH commands:

| Command | Function | Example/Note |
| --- | --- | --- |
| `gino_device_probe` | Check whether the primary device used by this example is registered. | Checks `pwm2` by default. |
| `pwm_dual_test start` | Start the dual-channel PWM output test. | Outputs PWM on PA7 and PG7. |
| `pwm_dual_test stop` | Stop the dual-channel PWM output test. | Disables the test outputs. |

## 2. PWM Timer Output Details

A timer period sets PWM frequency and a compare value sets high time. The demo drives `pwm2` channel 2 and `pwm30` channel 3 while changing pulse width, validating both timer channels and pin muxes.

The complete data path is:

```text
pwm_dual_test -> RT-Thread PWM -> timer compare outputs -> PA7 and PG7
```

The protocol layer only defines communication and data processing rules. Final validation still depends on controller clocks, pin multiplexing, interrupt/DMA handling, and upper-layer state machines. Device discovery, bus registration, or a successful build cannot replace a complete data transfer test.

## 3. GD32H77D Timer PWM Features

GD32H77D timers use a counter period for PWM frequency and a compare value for active time. The project exposes `pwm2` channel 2 on PA7 and `pwm30` channel 3 on PG7.

## 4. RT-Thread PWM Device Interface

The RT-Thread PWM interface uses `rt_device_pwm_set`/`rt_pwm_set` to set period and pulse, `rt_device_pwm_enable`/`rt_pwm_enable` to start a channel, and the corresponding disable interface to stop it. Time units follow the active RT-Thread PWM API definition.

Primary device: `pwm2`. Use `list_device` and `gino_device_probe` first to check whether the device has been registered. Upper-layer file system, network, or GUI components still need separate validation for mount, link, or refresh state.

## 5. Hardware

pwm2 channel 2 is on PA7 and pwm30 channel 3 is on PG7; inspect them with an oscilloscope or logic analyzer.

Default console: UART1, PA2/PA3, AF7, 115200-8-N-1.

Use an oscilloscope or logic analyzer to confirm frequency and duty cycle on PA7 and PG7.

![PWM output reference](figures/1788862150638.png)

## 6. Example

Source paths below are relative to the project directory in the SDK repository:

- `applications/pwm_dual_test.c`
- `../../libraries/gd32_drivers/drv_pwm.c`
- `board/Kconfig`

Read `applications/main.c` and `applications/device_probe.c` first, then follow the data path into the corresponding driver, component, or package. The example keeps MSH commands so device registration and runtime state can be observed without changing application code.

### 6.1 Runtime Commands

- `gino_device_probe`
- `pwm_dual_test start`
- `pwm_dual_test stop`

### 6.2 Operation Steps

1. Check power, wiring, external modules, and interface logic levels.
2. Reset the development board and confirm that the UART1 console is available and PC4 LED blinks normally.
3. Run `list_device` and confirm that dependency buses and target devices are registered.
4. Run the commands above in order while observing return values, external waveforms, network state, or display results.

## 7. Runtime Results

### 7.1 Expected Behavior

- `pwm2` and `pwm30` appear in `list_device`.
- PA7 and PG7 output about 2 kHz after start.
- Duty cycle changes on an oscilloscope and outputs stop on command.
