# SDK Integration

Upstream: https://github.com/RT-Thread-packages/rt-i2c-tools

Mirror: https://gitee.com/RT-Thread-Mirror/rt-i2c-tools

Version: `v1.0.0`, commit `afb8d5394f3e19c03a2b8e8bc4f6603803d2d2f5`.

Hardware tools license: MIT, see `LICENSE`. The optional SoftwareI2C sources
retain their upstream LGPL-2.1-or-later license headers and are not built here.

`Gino_driver_i2c` and `Gino_factory` enable this bundled package with
`PKG_USING_I2C_TOOLS`. Their `mklinks` scripts link to this directory.
`I2C_TOOLS_USE_SW_I2C` is disabled so commands use the registered hardware buses.

The upstream command is `i2c scan`, with optional hexadecimal start and exclusive
stop addresses. `i2c scan hwi2c1 08 78` probes `0x08-0x77` using address-only write
transfers. The factory touch page has its own scanner using the same RT-Thread
transfer API.

Local changes by CYFS on 2026-09-08:

- Reject read lengths outside 1-64 bytes and writes exceeding the command buffer.
- Start partial scan output on the row containing the first requested address.

Retain these local changes when updating or redownloading the package.
