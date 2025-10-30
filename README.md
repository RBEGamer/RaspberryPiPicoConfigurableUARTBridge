Raspberry Pi Pico USB-UART and I2C Bridge
=================================
This project converts the Raspberry Pi Pico(or any RP2040) into a USB to 6 UART and I2C board with activity LEDs.

Supporting hardware, the "PicoUART6" is available on [Tindie](https://www.tindie.com/products/32981/) and [eBay](https://www.ebay.co.uk/itm/204561279568).

The 6 USB UART are compatible with the builtin USB CDC class drivers in Linux, macOS, and >= Windows 10.

The I2C interface requires an i2c-tiny-usb driver. On Linux copy the "90-PicoUART6.rules" file into "/etc/udev/rules.d/", run "sudo udevadm control --reload" and then unplug/replug the Pico. Running "i2cdetect -l" will then show the i2c-tiny-usb bus you need to use in your application.

History
----------

This expands [JoeSc's](https://github.com/JoeSc/pico-sexa-uart-bridge) project to add activity LED for each UART which itself expands [Noltari's](https://github.com/Noltari/pico-uart-bridge) project to add 4 additional UARTs using the pico PIOs. And expands on [harrywalsh's](https://github.com/harrywalsh/pico-hw_and_pio-uart-gridge) project to provide better SEO and remove some data loss when using all 6 UARTs concurrently.
It then has [Nicolai-Electronics's](https://github.com/Nicolai-Electronics/rp2040-i2c-interface) project for i2c-tiny-usb support merged in.
[8086net's](https://github.com/8086net/pico-sexa-uart-bridge) pico-sexa-uart-bridge which provides a static code configured six uart and one i2c port.

This repository uses a dedicated configuration file `uart_config.txt`.

Disclaimer
----------

This software is provided without warranty, according to the MIT License, and should therefore not be used where it may endanger life, financial stakes, or cause discomfort and inconvenience to others.

Configuration
-------------
UART pin mappings live in `uart_config.txt`. Each non-comment line
specifies one CDC interface using the format:

```
TX=<pin> RX=<pin> LED=<pin> [UART=0|1|PIO=<sm>]
```

- `TX`, `RX`, and `LED` are mandatory and refer to Pico GPIO numbers.
- `UART=0`/`UART=1` can be added to force use of the hardware UART instances.
- `PIO=<sm>` assigns a specific PIO state machine (0–3). When neither option is
  present the build fills hardware UART0, then UART1, and finally the PIO
  state machines in ascending order.

The generated `uart_config.inc` is included by `uart-i2c-bridge.c` during the
build, so any edit to the configuration file is picked up automatically at the
next configure step.

I2C behaviour is controlled by `i2c_config.txt`:

```
ENABLED=0|1 SDA=<pin> SCL=<pin> [LED=<pin>]
```

- Set `ENABLED=1` to expose the vendor-class I2C bridge, or `0` to compile it
  out entirely.
- When enabled, `SDA` and `SCL` must be a valid pin pair for the RP2040. The
  build deduces whether `i2c0` or `i2c1` should be used from the supplied pins.
- `LED` defaults to GPIO14 if omitted.

The repository ships with:

```
ENABLED=1 SDA=26 SCL=27 LED=14
```

matching the Pico's default `i2c1` pins and the activity LED used by the
reference hardware. Switch `ENABLED=0` to remove the vendor interface from the
firmware without touching the source.

The parser emits `i2c_config.inc`, which provides the pin macros consumed by
`uart-i2c-bridge.c`. Editing either configuration file triggers regeneration
as part of the next build.

Raspberry Pi Pico Pinout
------------------------
The default configuration below matches the contents of `uart_config.txt`:

| Raspberry Pi Pico GPIO | Function |
|:----------------------:|:--------:|
| GPIO0 (Pin 1)          | UART0 TX |
| GPIO1 (Pin 2)          | UART0 RX |
| GPIO2 (Pin 4)          | UART0 Activity LED |
| GPIO4 (Pin 6)          | UART1 TX |
| GPIO5 (Pin 7)          | UART1 RX |
| GPIO3 (Pin 5)          | UART1 Activity LED |
| GPIO8 (Pin 11)         | UART2 TX |
| GPIO9 (Pin 12)         | UART2 RX |
| GPIO6 (Pin 9)          | UART2 Activity LED |
| GPIO12 (Pin 16)        | UART3 TX |
| GPIO13 (Pin 17)        | UART3 RX |
| GPIO7 (Pin 10)         | UART3 Activity LED |
| GPIO16 (Pin 21)        | UART4 TX |
| GPIO17 (Pin 22)        | UART4 RX |
| GPIO10 (Pin 14)        | UART4 Activity LED |
| GPIO20 (Pin 26)        | UART5 TX |
| GPIO21 (Pin 27)        | UART5 RX |
| GPIO11 (Pin 15)        | UART5 Activity LED |
| GPIO26 (Pin 31)        | I2C SDA |
| GPIO27 (Pin 32)        | I2C SCL |
| GPIO14 (Pin 19)        | I2C Activity LED |
| GPIO15 (Pin 20)        | Power LED |

Building
--------
The project uses CMake together with the Pico SDK. After installing the
toolchain, generate the build system and compile the firmware:

```bash
cmake -B build -S .
cmake --build build
```

By default the build uses `i2c_config.txt`; supply
`-DI2C_CONFIG_FILE=/path/to/i2c_config.txt` or
`-DUART_PIN_CONFIG=/path/to/uart_config.txt` to point at alternate presets.

The generated artifacts (UF2, ELF, map, etc.) are written inside `build/`.
