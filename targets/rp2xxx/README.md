# RP2040 Target

This target contains the native Espruino support for `RP2040_PICO`.

Current state:

- experimental
- work in progress
- subject to further testing and review

The target is usable and a substantial part of the baseline board support has
been implemented and hardware-validated, but it should not yet be treated as a
fully matured upstream target.

## Documentation

Code-side RP2040 target documentation lives in:

- [docs/](https://github.com/SimonGAndrews/Espruino/tree/rp2040_port/targets/rp2040/docs)

Start with:

- [docs/README.md](https://github.com/SimonGAndrews/Espruino/blob/rp2040_port/targets/rp2040/docs/README.md)
- [docs/jshardware_function_matrix_V2.md](https://github.com/SimonGAndrews/Espruino/blob/rp2040_port/targets/rp2040/docs/jshardware_function_matrix_V2.md)

Project-side requirements, design, status, and harness documentation live in:

- [espruino-rp2040](https://github.com/SimonGAndrews/espruino-rp2040)

Start with:

- [docs/README.md](https://github.com/SimonGAndrews/espruino-rp2040/blob/main/docs/README.md)
- [docs/rp2040_status.md](https://github.com/SimonGAndrews/espruino-rp2040/blob/main/docs/rp2040_status.md)

## Building And Flashing

The normal RP2040 build is run from the Espruino worktree:

```text
cd /home/simon/SGAdev/Espruino-rp2040-dev
source scripts/setup.sh
source scripts/provision.sh RP2040_PICO
make BOARD=RP2040_PICO
```

That flow keeps Espruino's normal code generation as the primary build entry
point and then hands RP2040 compilation over to the Pico SDK CMake build.

Expected output artifacts include:

- `bin/espruino_<version>_rp2040_pico.uf2`
- `bin/espruino_<version>_rp2040_pico.bin`
- `bin/espruino_<version>_rp2040_pico.elf`

The simplest normal flashing path is UF2 drag-and-drop:

1. Disconnect the Pico from normal runtime power if needed.
2. Hold the `BOOTSEL` button on the Pico.
3. While holding `BOOTSEL`, connect the Pico USB cable to the host.
4. Release `BOOTSEL` once the board appears as the `RPI-RP2` mass-storage
   device.
   On both Windows and Linux, the normal expected behaviour is that the board
   shows up as a removable storage volume and may also open as a mounted folder
   or file-manager window automatically, depending on host settings.
5. Copy the generated UF2 file from `bin/` onto the mounted `RPI-RP2` drive.
6. Wait for the copy to complete. The Pico will reboot automatically into the
   new Espruino firmware.

Example:

```text
cp /home/simon/SGAdev/Espruino-rp2040-dev/bin/espruino_2v29.68_rp2040_pico.uf2 /media/<user>/RPI-RP2/
```

## Testing

The RP2040 port is validated primarily through saved hardware regression tests
that exercise the normal Espruino APIs on the standard harness, rather than
through a separate low-level unit-test framework.

Saved tests and harness specifications live in the companion project
repository:

- [testing/](https://github.com/SimonGAndrews/espruino-rp2040/tree/main/testing)
- [docs/rp2040_test_harness.md](https://github.com/SimonGAndrews/espruino-rp2040/blob/main/docs/rp2040_test_harness.md)

The normal runner is:

```text
cd /home/simon/SGAdev/espruino-rp2040
python3 testing/run_repl_tests.py --folder <folder_name>
```

Current saved test areas include:

- `repl_tests`
- `gpio_loopback_tests`
- `watch_tests`
- `adc_tests`
- `i2c_tests`
- `spi_tests`
- `flash_tests`
- `storage_tests`

Use the harness specification as the standard bench reference for wiring and
device allocation when interpreting or extending these tests.
