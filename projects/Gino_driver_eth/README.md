# Gino_driver_eth

## 1. Introduction

This project is the GD32H77D Gino development board reference project for ENET1 RMII Ethernet. It is used to learn, configure, and validate Ethernet, RMII, and TCP/IP Layers separately. The standalone project enables only the drivers and components required by this feature, and can be used as a reference for application development and integration.

Primary device: `e0`.

All examples keep `uart1` as the FinSH/MSH console at 115200-8-N-1, and PC4 LED is used as the run indicator.

Current project provides the following MSH commands:

| Command | Function | Example/Note |
| --- | --- | --- |
| `gino_device_probe` | Check whether the primary device used by this example is registered. | Checks `e0` by default. |
| `ifconfig` | Check the connection state and IP address of the netdev. | Confirm that the network is ready. |
| `ping 192.168.1.1` | Send ICMP packets through the active network interface. | Replace the address with the actual gateway or peer. |

## 2. Ethernet, RMII, and TCP/IP Layers Details

lwIP pbufs pass through RT-Thread `eth_device` to ENET1. TX copies pbuf chains and hands DMA descriptor ownership to hardware; RX interrupt only wakes the network thread, which copies valid frames and returns descriptors to DMA. A PHY thread handles address detection, autonegotiation, and link status.

The complete data path is:

```text
socket/SAL -> lwIP -> e0 eth_device -> ENET DMA -> RMII -> PHY -> cable
```

The protocol layer only defines communication and data processing rules. Final validation still depends on controller clocks, pin multiplexing, interrupt/DMA handling, and upper-layer state machines. Device discovery, bus registration, or a successful build cannot replace a complete data transfer test.

## 3. GD32H77D ENET1 and RMII Features

The project uses GD32H77D ENET1 RMII with PHY address 2 and a 50 MHz REF_CLK on PC12. ENET DMA transfers frames through dedicated descriptors/buffers, while MDIO/MDC manages the PHY.

## 4. RT-Thread Ethernet, Netdev, and SAL Device Interface

The driver registers an RT-Thread `eth_device` and the `e0` netdev. RX interrupts call `eth_device_ready` to wake the network thread, lwIP supplies IP/TCP/UDP, and SAL exposes sockets; `ifconfig` shows network configuration and `ping` checks reachability.

Primary device: `e0`. Use `list_device` and `gino_device_probe` first to check whether the device has been registered. Upper-layer file system, network, or GUI components still need separate validation for mount, link, or refresh state.

## 5. Hardware

ENET1 uses RMII with default PHY address 2. The PHY must provide a 50 MHz reference clock on PC12.

Default console: UART1, PA2/PA3, AF7, 115200-8-N-1.

## 6. Example

Source paths below are relative to the project directory in the SDK repository:

- `../../libraries/Board_Drivers/drv_enet.c`
- `board/SConscript`
- `rtconfig.h`
- `applications/device_probe.c`

Read `applications/main.c` and `applications/device_probe.c` first, then follow the data path into the corresponding driver, component, or package. The example keeps MSH commands so device registration and runtime state can be observed without changing application code.

### 6.1 Runtime Commands

- `gino_device_probe`
- `ifconfig`
- `ping 192.168.1.1`

### 6.2 Operation Steps

1. Check power, wiring, external modules, and interface logic levels.
2. Reset the development board and confirm that the UART1 console is available and PC4 LED blinks normally.
3. Run `list_device` and confirm that dependency buses and target devices are registered.
4. Run the commands above in order while observing return values, external waveforms, network state, or display results.

## 7. Runtime Results

### 7.1 Expected Behavior

- The PHY is found at default address 2 and autonegotiates.
- `ifconfig` reports `e0` link up and a valid IP.
- Gateway ping is stable without descriptor errors under traffic.
