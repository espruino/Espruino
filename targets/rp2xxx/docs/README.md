# RP2040 Target Documentation

## Purpose

This directory contains code-side documentation for the Espruino `RP2040_PICO`
target.

The RP2040 port should currently be treated as:

- work in progress
- experimental
- subject to further testing and review

This documentation is intended to stay close to the live target sources in the
Espruino codebase. Use it for implementation tracking and code-adjacent
technical reference.

## Current Status

The best current code-side status summary is:

- [jshardware_function_matrix_V2.md](https://github.com/SimonGAndrews/Espruino/blob/rp2040_port/targets/rp2040/docs/jshardware_function_matrix_V2.md)

That document records:

- RP2040 `jshardware.h` implementation status
- current Espruino API validation summary
- hardware validation state
- remaining gaps and next steps

Current validated highlights now include:

- `digitalPulse` and `shiftOut` on the standard GPIO harness loopbacks
- explicit `setTimeout` / `setInterval` / cancel validation
- `I2C1` with one and two `MCP23008` devices on the shared harness bus
- `I2C2` on `D14/D15` with a removable `SSD1306`-compatible OLED at `0x3c`
- `OneWire` on `D21` with `DS18B20` search and temperature conversion/read
- shared-bus `SPI1` validation with `MCP3008` and `W25xxx`
- non-USB `Serial1` validation on `D0/D1`
- non-USB `Serial2` validation on `D4/D5`
- console movement validated between `USB` and `Serial1`, including forced
  `Serial1` retention across USB disconnect/reconnect

## Documentation Split

Documentation for this port is split across two repositories.

### In This Repository

Use this directory for documentation that is tightly coupled to the live
Espruino RP2040 target sources.

Current active files here include:

- [jshardware_function_matrix_V2.md](https://github.com/SimonGAndrews/Espruino/blob/rp2040_port/targets/rp2040/docs/jshardware_function_matrix_V2.md)
- [jshardware_function_matrix.csv](https://github.com/SimonGAndrews/Espruino/blob/rp2040_port/targets/rp2040/docs/jshardware_function_matrix.csv)

Archived older target-local snapshots live under:

- [archive/](https://github.com/SimonGAndrews/Espruino/tree/rp2040_port/targets/rp2040/docs/archive)

### In The Project Repository

Use the separate project repository for the active project-side documentation:

- [espruino-rp2040](https://github.com/SimonGAndrews/espruino-rp2040)

The main active document set there is:

- [docs/README.md](https://github.com/SimonGAndrews/espruino-rp2040/blob/main/docs/README.md)
- [docs/rp2040_requirements.md](https://github.com/SimonGAndrews/espruino-rp2040/blob/main/docs/rp2040_requirements.md)
- [docs/rp2040_design.md](https://github.com/SimonGAndrews/espruino-rp2040/blob/main/docs/rp2040_design.md)
- [docs/rp2040_status.md](https://github.com/SimonGAndrews/espruino-rp2040/blob/main/docs/rp2040_status.md)
- [docs/rp2040_test_harness.md](https://github.com/SimonGAndrews/espruino-rp2040/blob/main/docs/rp2040_test_harness.md)

Use the project repository when you need:

- requirements and scope
- architectural rules
- current milestone/progress status
- harness specification and bench wiring
- memory-layout and policy documents

## Practical Reading Order

For someone reviewing the current RP2040 port, the simplest reading order is:

1. [docs/rp2040_status.md](https://github.com/SimonGAndrews/espruino-rp2040/blob/main/docs/rp2040_status.md)
2. [jshardware_function_matrix_V2.md](https://github.com/SimonGAndrews/Espruino/blob/rp2040_port/targets/rp2040/docs/jshardware_function_matrix_V2.md)
3. [docs/rp2040_test_harness.md](https://github.com/SimonGAndrews/espruino-rp2040/blob/main/docs/rp2040_test_harness.md)

That gives:

- project status
- implementation and validation detail
- the standard hardware test platform
