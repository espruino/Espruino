# RP2040 jshardware Function Matrix V2

## 1. Purpose and Introduction

This is the active human-readable RP2040 `jshardware` coverage document.

It combines:

- a short RP2040 implementation summary based on
  `targets/rp2040/jshardware.c`
- the generated `jshardware.h` function matrix snapshot below

The archived raw V1 snapshot lives at:

- `targets/rp2040/docs/archive/jshardware_function_matrix.md`

The CSV remains alongside this document as the raw companion export:

- `targets/rp2040/docs/jshardware_function_matrix.csv`

The RP2040 milestones are primarily about building the working Espruino
hardware interface under `src/jshardware.h`.

Validation should normally happen through Espruino APIs that use this
interface, not through a separate unit-test layer for each low-level function.
Direct low-level testing is still useful when debugging unclear or failing API
behaviour.

## 2. RP2040 Port Summary

### RP2040 Status Summary

The baseline `RP2040_PICO` board support is implemented and hardware-validated
for the current milestone scope.

Already implemented and proven on hardware:

- standard build and delegated Pico SDK handoff
- USB CDC REPL and Web IDE RAM upload
- GPIO pin state and value handling
- `digitalPulse` and `shiftOut` on the standard GPIO harness loopbacks
- watch/event delivery on real GPIO and expander interrupt paths
- explicit `setTimeout` / `setInterval` / `clearTimeout` / `clearInterval`
- ADC read and first-pass PWM output on the standard harness
- hardware `I2C1` setup/read/write on one and two devices on the same bus
- hardware `I2C2` setup/read/write to a removable OLED on the second dedicated bus
- hardware `SPI1` setup/send on a shared bus with `MCP3008` and `W25xxx`
- hardware `Serial1` setup/read/write on `D0/D1`
- hardware `Serial2` setup/read/write on `D4/D5` with a Debug Probe UART peer
- `OneWire` on `D21` with `DS18B20` search and temperature conversion/read
- console movement between `USB` and `Serial1`, including forced `Serial1` retention
- flash read/write/erase in the saved-code bank
- `Storage` and `save()` restore

Still stubbed, minimal, or not yet proven on hardware:

- hardware validation of `jshPinAnalogFast`
- wider UART option coverage such as framing-error injection and broader configuration combinations
- wider SPI coverage for 16-bit transfers and additional mode/rate combinations
- low-power and deeper power-management behaviour
- first USB-attach startup banner preservation; current evidence points to a
  shared `EV_LIMBO` / TX-buffer capacity limit rather than a remaining
  RP2040-specific runtime console bug
- full pin-function reporting and output-function routing

### Next Hardware Validation Targets

- wider ADC/PWM coverage beyond the current standard harness node
- wider SPI coverage for 16-bit transfers and additional bus settings
- broader UART option coverage and automated console-switch regression
- explicit `Pin` object coverage and remaining shared API tidy-up

## 3. Espruino API Validation Summary

This summary follows the relevant headings in the Espruino software reference.
It does not try to reproduce the full reference. It records what has been
proven on `RP2040_PICO`, what still matters, and which saved tests or
`jshardware` functions provide the evidence chain.

**`E`**
- Proven: practical persistence-related behaviour exercised through `save()` and reset flow
- Outstanding: broader `E` utility coverage
- Evidence: `flash_tests`, `storage_tests`, `save()` restore validation
- `jshardware`: primarily `jshFlash*`, reset/reboot, and clock/time helpers

**`Flash`**
- Proven: `Flash.getFree`, `Flash.read`, `Flash.write`, `Flash.erasePage`
- Outstanding: broader repeated-compaction/endurance characterisation
- Evidence: `testing/flash_tests`, saved-code bank validation
- `jshardware`: `jshFlashGetFree`, `jshFlashRead`, `jshFlashWrite`, `jshFlashErasePage`, `jshFlashGetMemMapAddress`

**`Globals`**
- Proven: `pinMode`, `digitalRead`, `digitalWrite`, `digitalPulse`, `shiftOut`, `analogRead`, `analogWrite`, `setWatch`, `save`, `reset`
- Outstanding: broader timing and deep-sleep behaviour
- Evidence: `gpio_loopback_tests`, `pin_api_tests`, `watch_tests`, `adc_tests`, `flash_tests`, `storage_tests`, reset/power-cycle validation
- `jshardware`: `jshPinSetState`, `jshPinGetValue`, `jshPinSetValue`, `jshPinAnalog`, `jshPinAnalogOutput`, `jshPinWatch`, `jshUtilTimer*`, `jshFlash*`, `jshReboot`

**`I2C`**
- Proven: `I2C1.setup`, `I2C2.setup`, `writeTo`, `readFrom`, `readReg`
- Outstanding: broader second-bus device coverage beyond the current OLED
- Evidence: `testing/i2c_tests`, including two `MCP23008` devices on shared `I2C1` and an `SSD1306`-compatible OLED at `0x3c` on `I2C2`
- `jshardware`: `jshI2CSetup`, `jshI2CWrite`, `jshI2CRead`

**`Modules`**
- Proven: `require("Storage")`
- Outstanding: wider built-in module coverage outside the current baseline
- Evidence: `testing/storage_tests`, Web IDE Device Storage workflow
- `jshardware`: indirect via `jshFlash*`

**`OneWire`**
- Proven: bus reset, device search, ROM handling, and temperature conversion/read with `DS18B20`
- Outstanding: broader device coverage beyond the current `DS18B20` harness path
- Evidence: `testing/onewire_tests`
- `jshardware`: shared software implementation depends on GPIO, timing, and interrupt primitives already present in the port

**`Pin`**
- Proven: practical pin use through the global GPIO, ADC, PWM, and watch APIs
- Outstanding: no separate `Pin`-object-focused validation
- Evidence: `gpio_loopback_tests`, `watch_tests`, `adc_tests`
- `jshardware`: GPIO, ADC/PWM, and watch-related `jshPin*` functions

**`Process`**
- Proven: `process.env`, `process.memory()`, `process.version`, and interactive `process.on('uncaughtException', ...)`
- Outstanding: no saved regression yet for process-level exception-hook behavior
- Evidence: live Web IDE checks on current RP2040 firmware, including `uncaughtException` callback handling
- `jshardware`: indirect; depends on stable console/runtime/event delivery rather than a dedicated `jshardware` API

**`Serial`**
- Proven: USB CDC REPL and Web IDE console path, `Serial1.setup`, `Serial2.setup`, `write`, `read`, `on("data",...)`, and explicit `Serial.unsetup()` / re-setup cycling on both hardware UARTs; unforced console movement between `USB` and `Serial1`; forced `Serial1` retention across USB disconnect/reconnect
- Outstanding: broader UART option coverage and an automated saved regression for console-switch behavior
- Evidence: standard REPL and Web IDE operation on the Pico CDC port; `testing/serial_tests`; direct bench validation of `USB <-> Serial1` console switching
- `jshardware`: `jshUSARTSetup`, `jshUSARTKick`, `jshIsDeviceInitialised`, RP2040 USB/runtime lifecycle helpers

**`SPI`**
- Proven: `SPI1.setup`, `send`
- Outstanding: 16-bit SPI path, `SPI2`, broader mode/rate coverage
- Evidence: `testing/spi_tests`, including `MCP3008`, `W25xxx`, and shared-bus coexistence
- `jshardware`: `jshSPISetup`, `jshSPISend`, `jshSPISend16`, `jshSPISet16`, `jshSPISetReceive`, `jshSPIWait`

**`Storage`**
- Proven: `require("Storage").read`, `write`, persistent file visibility and boot-time use
- Outstanding: none essential in the current baseline
- Evidence: `testing/storage_tests`, Web IDE Device Storage checks, power-cycle persistence
- `jshardware`: `jshFlash*` via Espruino flash/storage layers

**`timer`**
- Proven: `setTimeout`, `setInterval`, `clearTimeout`, and `clearInterval`; asynchronous timing used successfully by saved tests
- Outstanding: broader timing characterisation and deep-sleep interaction
- Evidence: `testing/timer_tests` plus saved tests that depend on `setTimeout` sequencing across GPIO, I2C, and SPI paths
- `jshardware`: `jshGetSystemTime`, `jshGetTimeFromMilliseconds`, `jshGetMillisecondsFromTime`, `jshUtilTimer*`

## 4. jshardware Implementation Summary

Purpose: provide a status view of the functions within the RP2040 target's
implementation of the `jshardware.h` interface.

Appendix A provides the full `jshardware.h` prototypes as a raw reference list.

### How To Use The RP2040 Tracking Columns

The generated RP2040 snapshot uses the following columns alongside the
cross-target reference data.

Use them as follows:

- `RP2040 Status`
  - set to `impl`, `impl-common`, `part`, `stub`, or
    `unrev`
  - use `impl-common` where the RP2040 target is currently satisfied by
    shared Espruino hardware-layer support such as `src/jshardware_common.c`
- `Milestone`
  - note the phase where the function matters most, for example `M1`, `M2`,
    `M3`, or `Later`
- `Primary API Test`
  - record the main Espruino API expected to exercise that function during
    validation
  - prefix with `?` where the mapping is a good inferred first guess rather
    than a fully confirmed one
- `Hardware Validated`
  - set to `Yes` only after the behaviour has been proven on real RP2040
    hardware through the intended Espruino-facing test path
  - use `No` until that proof exists
- `Notes`
  - keep this to a short RP2040-specific comment such as `USB CDC only`,
    `common helper`, `returns placeholder value`, or `out of current scope`

Rows can be updated incrementally as implementation and testing progress.
Unreviewed rows currently use placeholder values so this document can act as
the active tracking matrix without losing the generated reference content.

### Generated Matrix Snapshot

Generated from `src/jshardware.h`.

| Function | Uses<br>Targets | RP2040 Status | Phase | Primary API Test | Hardware Validated | Notes |
|---|---|---|---|---|---|---|
| `jshReset` | 10<br>STM<br>NRF<br>ESP | `impl` | `M1` | `reset()/boot` | `Yes` | reset/boot path exercised during persistence and power-cycle validation |
| `jshIdle` | 9<br>STM<br>NRF<br>ESP | `impl` | `M1` | `USB REPL / Web IDE` | `Yes` | USB runtime polling path proven by CDC REPL and Web IDE use; first-attach boot banner remains limited by shared `EV_LIMBO` / TX-buffer capacity rather than RP2040 polling logic |
| `jshBusyIdle` | 5<br>NRF | `impl` | `M1` | `USB REPL / Web IDE` | `Yes` | USB runtime polling path proven by CDC REPL and Web IDE use |
| `jshSleep` | 10<br>STM<br>NRF<br>ESP | `part` | `Later` | `jsiIdle / idle runtime` | `Yes` | narrow RP2040 light-sleep path now uses Pico SDK WFE-style waiting plus normal Espruino sleep-indicator state transitions; no-USB `Serial1` bench validation has been re-proven, while deeper RP2040 sleep-mode design remains deferred |
| `jshKill` | 15<br>STM<br>NRF<br>ESP | `impl` | `M1` | `?E.kill` | `No` | no-op cleanup |
| `jshGetSerialNumber` | 9<br>STM<br>NRF<br>ESP | `impl` | `M1` | `getSerial()` | `Yes` | direct `getSerial()` saved test now proves the Pico unique-board-id path and Espruino string formatting |
| `jshIsUSBSERIALConnected` | 13<br>STM<br>NRF<br>ESP | `impl` | `M1` | `USB REPL / Web IDE` | `Yes` | TinyUSB CDC state proven through repeated host connect/disconnect use |
| `jshGetSystemTime` | 32<br>STM<br>NRF<br>ESP | `impl` | `M1` | `getTime` | `Yes` | `getTime` and timer scheduling validated by explicit `timer_tests` and wider saved async test use |
| `jshSetSystemTime` | 10<br>STM<br>NRF<br>ESP | `impl` | `M1` | `setTime` | `Yes` | explicit `setTime()` / `getTime()` roundtrip saved test proves the RP2040 system-time offset path |
| `jshGetTimeFromMilliseconds` | 32<br>STM<br>NRF<br>ESP | `impl` | `M1` | `setTimeout / setInterval` | `Yes` | time conversion path validated by explicit `timer_tests` and async scheduling across the saved packs |
| `jshGetMillisecondsFromTime` | 19<br>STM<br>NRF<br>ESP | `impl` | `M1` | `getTime / setTimeout` | `Yes` | time conversion path validated by explicit `timer_tests` and `getTime` use |
| `jshInterruptOff` | 20<br>STM<br>NRF<br>ESP | `impl` | `M1` | `OneWire timing-critical paths` | `Yes` | nested IRQ-disable stack is now supported indirectly by repeated DS18B20 OneWire transactions under concurrent timer activity |
| `jshInterruptOn` | 20<br>STM<br>NRF<br>ESP | `impl` | `M1` | `OneWire timing-critical paths` | `Yes` | nested IRQ-enable unwind is now supported indirectly by repeated DS18B20 OneWire transactions under concurrent timer activity |
| `jshIsInInterrupt` | 12<br>STM<br>NRF<br>ESP | `impl` | `M1` | internal core guard paths | `No` | now uses the Cortex-M0+ `IPSR` register to distinguish IRQ from thread context; internal semantic fix, not directly user-visible |
| `jshDelayMicroseconds` | 32<br>STM<br>NRF<br>ESP | `impl` | `M1` | `OneWire / I2C delay paths` | `Yes` | busy wait used successfully by proven `OneWire` DS18B20 transactions and existing bit-delay users |
| `jshPinSetValue` | 34<br>STM<br>NRF<br>ESP | `impl` | `M1` | `digitalWrite` | `Yes` | proven on real cross-pin loopbacks and harness control paths |
| `jshPinGetValue` | 29<br>STM<br>NRF<br>ESP | `impl` | `M1` | `digitalRead` | `Yes` | proven on real cross-pin loopbacks and harness feedback paths |
| `jshPinSetState` | 29<br>STM<br>NRF<br>ESP | `impl` | `M1` | `pinMode` | `Yes` | input/output/pull state handling proven on harness GPIO and watch tests |
| `jshPinGetState` | 13<br>STM<br>NRF<br>ESP | `impl` | `M1` | `pinMode` | `Yes` | software-tracked state proven indirectly through working `pinMode` paths |
| `jshIsPinStateDefault` | 5<br>ESP | `impl-common` | `TBD` | `?pinMode` | `No` | common helper |
| `jshPinAnalog` | 15<br>STM<br>NRF<br>ESP | `impl` | `M2` | `analogRead` | `Yes` | RP2040 ADC path implemented for ADC-capable pins; hardware validated on `D26` via harness feedback node |
| `jshPinAnalogFast` | 9<br>STM<br>NRF<br>ESP | `impl` | `M2` | `?analogRead` | `No` | uses same RP2040 ADC path as `jshPinAnalog`; direct API-level hardware proof still pending |
| `jshPinAnalogOutput` | 13<br>STM<br>NRF<br>ESP | `part` | `M2` | `analogWrite` | `Yes` | first-pass PWM output implemented; hardware PWM validated on `D10`; software PWM fallback exists but is not yet characterised |
| `jshCanWatch` | 9<br>STM<br>NRF<br>ESP | `impl` | `M2` | `setWatch` | `Yes` | RP2040 GPIO IRQ path; currently permissive, no RP2040-specific conflict filtering yet |
| `jshPinWatch` | 15<br>STM<br>NRF<br>ESP | `impl` | `M2` | `setWatch` | `Yes` | EV_EXTI slot map + GPIO edge IRQ enable; `JshPinWatchFlags` not yet differentiated on RP2040; normal watch use proven, burst-edge stress still TBD |
| `jshGetCurrentPinFunction` | 9<br>STM<br>NRF<br>ESP | `impl` | `M2` | `analogWrite / Waveform.startOutput` | `Yes` | returns a target-local cached PWM function token for RP2040 hardware-PWM outputs; saved `Waveform.startOutput` test on `D10 -> D26` proves the token is consumed correctly by the utility-timer output path |
| `jshSetOutputValue` | 9<br>STM<br>NRF<br>ESP | `impl` | `M2` | `Waveform.startOutput` | `Yes` | decodes the cached RP2040 PWM token and updates the active slice/channel duty cycle; saved `Waveform.startOutput` test on `D10 -> D26` validates the waveform-driven duty-cycle updates |
| `jshEnableWatchDog` | 10<br>STM<br>NRF<br>ESP | `impl` | `M2` | `E.enableWatchdog` | `Yes` | maps to Pico SDK `watchdog_enable(...)`, starts the RP2040 watchdog tick in target init, clamps timeout to RP2040 limits, and keeps `pause_on_debug` disabled so the real timeout-reset path remains active; saved tests prove auto/manual survival and mode switching, while the destructive timeout-reset helper proves recovery after a wedged interpreter in both auto and manual modes |
| `jshKickWatchDog` | 16<br>STM<br>NRF<br>ESP | `impl` | `M2` | `E.kickWatchdog` | `Yes` | maps to Pico SDK `watchdog_update()`; saved tests prove repeated kicks keep the watchdog alive, harmless pre-enable kicks do not break runtime, and manual-to-auto switching remains stable before the destructive timeout-reset helper confirms recovery when kicks stop |
| `jshKickSoftWatchDog` | 5<br>NRF | `unrev` | `TBD` | `?E.kickWatchdog` | `No` | review later |
| `jshGetWatchedPinState` | 9<br>STM<br>NRF<br>ESP | `impl` | `M2` | `setWatch` | `Yes` | cached IRQ-time pin level; sufficient for current watch semantics, stress characterization still TBD |
| `jshIsEventForPin` | 10<br>STM<br>NRF<br>ESP | `impl` | `M2` | `setWatch` | `Yes` | EV_EXTI slot-to-pin mapping |
| `jshIsDeviceInitialised` | 15<br>STM<br>NRF<br>ESP | `impl` | `M1` | `USB REPL / Serial/SPI/I2C` | `Yes` | USB plus validated RP2040 Serial1/Serial2, SPI, and I2C device state |
| `jshUSARTInitInfo` | 8<br>STM<br>NRF | `impl-common` | `M1` | `Serial.setup` | `No` | common helper |
| `jshUSARTSetup` | 12<br>STM<br>NRF<br>ESP | `impl` | `M1` | `Serial.setup`, `Serial.setConsole`, `E.setConsole` | `Yes` | validated for the RP2040 USB console path plus `Serial1` on `D0/D1` and `Serial2` on `D4/D5`; hardware console auto-init and runtime console movement are now proven |
| `jshUSARTUnSetup` | 6<br>STM<br>NRF<br>ESP | `impl` | `M1` | `Serial.unsetup` | `Yes` | validated for `Serial1` and `Serial2`, including setup -> use -> unsetup -> setup -> use lifecycle tests |
| `jshUSARTKick` | 11<br>STM<br>NRF<br>ESP | `impl` | `M1` | `USB REPL / Serial.print` | `Yes` | USB CDC transmit path proven by REPL and Web IDE interaction; RP2040 UART0/UART1 transmit paths proven through `Serial1` and `Serial2` validation |
| `jshSPIInitInfo` | 13 | `impl-common` | `M2` | `SPI.setup` | `No` | common helper |
| `jshSPISetup` | 19<br>STM<br>NRF<br>ESP | `impl` | `M2` | `SPI.setup` | `Yes` | RP2040 hardware SPI setup validated on `SPI1` with explicit/default harness pins and shared-bus use with `MCP3008` plus `W25xxx` |
| `jshSPISend` | 16<br>STM<br>NRF<br>ESP | `impl` | `M2` | `SPI.send` | `Yes` | blocking RP2040 SPI transfer validated against `MCP3008` reads, `W25xxx` JEDEC/status commands, and shared-bus coexistence |
| `jshSPISend16` | 10<br>STM<br>NRF<br>ESP | `impl` | `M2` | `?SPI.send` | `No` | first-pass 16-bit support implemented; wider hardware validation still TBD |
| `jshSPISet16` | 11<br>STM<br>NRF<br>ESP | `impl` | `M2` | `?SPI.setup` | `No` | reconfigures RP2040 SPI frame size between 8-bit and 16-bit |
| `jshSPISetReceive` | 12<br>STM<br>NRF<br>ESP | `impl` | `M2` | `?SPI.send` | `No` | target state used to enable/disable receive path when MISO is present |
| `jshSPIWait` | 14<br>STM<br>NRF<br>ESP | `impl` | `M2` | `SPI.send` | `No` | blocking backend waits for idle and drains RX state |
| `jshI2CInitInfo` | 7 | `impl-common` | `M2` | `I2C.setup` | `No` | common helper |
| `jshI2CSetup` | 12<br>STM<br>NRF<br>ESP | `impl` | `M2` | `I2C.setup` | `Yes` | RP2040 hardware I2C master setup validated on `I2C1` with default/explicit pins and two `MCP23008` devices on one bus, plus `I2C2` on `D14/D15` with an OLED at `0x3c` |
| `jshI2CUnSetup` | 4<br>STM | `impl` | `M2` | internal reset / I2C teardown paths | `No` | target hook is implemented and used by RP2040 reset/teardown logic; no public JS-level `I2C.unsetup()` API exists for standalone saved-test validation |
| `jshI2CWrite` | 12<br>STM<br>NRF<br>ESP | `impl` | `M2` | `I2C.writeTo` | `Yes` | RP2040 blocking master write validated against `MCP23008` register writes at `0x20` and `0x21` on the same bus and OLED command/data traffic at `0x3c` on `I2C2` |
| `jshI2CRead` | 12<br>STM<br>NRF<br>ESP | `impl` | `M2` | `I2C.readFrom` | `Yes` | RP2040 blocking master read validated against `MCP23008` register readback at `0x20` and `0x21`; `I2C2` OLED validation is write-oriented and does not add meaningful controller readback |
| `jshFlashGetPage` | 11<br>STM<br>NRF<br>ESP | `impl` | `M1` | `Flash.getPage` | `Yes` | direct saved test proves RP2040 4 KiB page geometry on the reserved saved-code bank |
| `jshFlashGetFree` | 10<br>STM<br>NRF<br>ESP | `impl` | `M2` | `Flash.getFree` | `Yes` | returns the fixed RP2040 saved-code bank at the top of flash |
| `jshFlashErasePage` | 13<br>STM<br>NRF<br>ESP | `impl` | `M2` | `Flash.erasePage` | `Yes` | RP2040 sector erase constrained to the saved-code bank |
| `jshFlashErasePages` | 4<br>NRF | `weak` | `M2` | `?Flash.erasePage` | `No` | not overridden; single-page erase path is used |
| `jshFlashRead` | 16<br>STM<br>NRF<br>ESP | `impl` | `M1` | `Flash.read / Storage.read` | `Yes` | XIP flash read proven by `Flash` and `Storage` roundtrip tests plus Web IDE storage access |
| `jshFlashWrite` | 12<br>STM<br>NRF<br>ESP | `impl` | `M2` | `Flash.write / Storage.write` | `Yes` | RP2040 page-program path validated by `Flash` roundtrip and `Storage` write/read |
| `jshFlashWriteAligned` | 4 | `impl-common` | `M2` | `?Flash.write` | `Yes` | common helper now works on RP2040 because target `jshFlashWrite` exists |
| `jshFlashGetMemMapAddress` | 11<br>STM<br>NRF<br>ESP | `impl` | `M1` | `Flash.read / Storage.read` | `Yes` | XIP address mapping exercised by the proven flash/storage read paths |
| `jshUtilTimerStart` | 9<br>STM<br>NRF<br>ESP | `impl` | `M2` | `digitalPulse` | `Yes` | RP2040 hardware-alarm backend now drives the util timer and is validated by `digitalPulse` edge-pattern tests |
| `jshUtilTimerReschedule` | 9<br>STM<br>NRF<br>ESP | `impl` | `M2` | `digitalPulse` | `Yes` | util timer reschedule path is exercised by multi-stage pulse sequences on real hardware |
| `jshUtilTimerDisable` | 10<br>STM<br>NRF<br>ESP | `impl` | `M2` | `digitalPulse` | `Yes` | util timer disable/reset path is exercised by pulse completion and reset-time cleanup without residual pulses |
| `jshGetPinAddress` | 5<br>STM | `stub` | `Later` | `?peek / poke` | `No` | no RP2040 mapping |
| `jshSetupRTCPrescalerValue` | 3<br>STM | `unrev` | `Later` | `?E.setClock` | `No` | out of current scope |
| `jshGetRTCPrescalerValue` | 3<br>STM | `unrev` | `Later` | `?E.getClock` | `No` | out of current scope |
| `jshResetRTCTimer` | 3<br>STM | `unrev` | `Later` | `?E.setClock` | `No` | out of current scope |
| `jshClearUSBIdleTimeout` | 4<br>STM | `unrev` | `Later` | `?USB REPL idle` | `No` | likely unnecessary |
| `jshReadVRef` | 13<br>STM<br>NRF<br>ESP | `part` | `M2` | `E.getAnalogVRef` | `No` | fixed 3.3V |
| `jshReadVDDH` | 3<br>NRF | `stub` | `Later` | `?E.getBattery` | `No` | unsupported on RP2040 |
| `jshGetRandomNumber` | 9<br>STM<br>NRF<br>ESP | `impl` | `M2` | `Math.random` | `No` | now uses Pico SDK `pico_rand` via `get_rand_32()`; improved implementation, but no deeper statistical or source-quality validation is claimed yet |
| `jshSetSystemClock` | 10<br>STM<br>NRF<br>ESP | `stub` | `Later` | `E.setClock` | `Yes` | RP2040 now defers clock control explicitly; `E.setClock(...)` returns `0` per the documented unimplemented contract and is covered by a saved API test |
| `jshGetSystemClock` | 4<br>STM | `weak` | `Later` | `E.getClock` | `Yes` | RP2040 intentionally leaves the weak default in place, so `E.getClock()` returns `undefined`; this deferred state is covered by a saved API test |
| `jshReboot` | 12<br>STM<br>NRF<br>ESP | `impl` | `M1` | `reset()` | `Yes` | reboot/reset path proven during `save()` restore and persistence validation |
| `jshVirtualPinSetValue` | 4<br>NRF | `unrev` | `Later` | `?virtual pins` | `No` | out of current scope |
| `jshVirtualPinGetValue` | 4<br>NRF | `unrev` | `Later` | `?virtual pins` | `No` | out of current scope |
| `jshVirtualPinGetAnalogValue` | 4<br>NRF | `unrev` | `Later` | `?virtual pins` | `No` | out of current scope |
| `jshVirtualPinSetState` | 4<br>NRF | `unrev` | `Later` | `?virtual pins` | `No` | out of current scope |
| `jshVirtualPinGetState` | 4<br>NRF | `unrev` | `Later` | `?virtual pins` | `No` | out of current scope |

## Appendix A. Full Prototypes

- `void jshReset();`
- `void jshIdle();`
- `void jshBusyIdle();`
- `bool jshSleep(JsSysTime timeUntilWake);`
- `void jshKill();`
- `int jshGetSerialNumber(unsigned char *data, int maxChars);`
- `bool jshIsUSBSERIALConnected();`
- `JsSysTime jshGetSystemTime();`
- `void jshSetSystemTime(JsSysTime time);`
- `JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms);`
- `JsVarFloat jshGetMillisecondsFromTime(JsSysTime time);`
- `void jshInterruptOff();`
- `void jshInterruptOn();`
- `bool jshIsInInterrupt();`
- `void jshDelayMicroseconds(int microsec);`
- `void jshPinSetValue(Pin pin, bool value);`
- `bool jshPinGetValue(Pin pin);`
- `void jshPinSetState(Pin pin, JshPinState state);`
- `JshPinState jshPinGetState(Pin pin);`
- `bool jshIsPinStateDefault(Pin pin, JshPinState state);`
- `JsVarFloat jshPinAnalog(Pin pin);`
- `int jshPinAnalogFast(Pin pin);`
- `JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags);`
- `bool jshCanWatch(Pin pin);`
- `IOEventFlags jshPinWatch(Pin pin, bool shouldWatch, JshPinWatchFlags flags);`
- `JshPinFunction jshGetCurrentPinFunction(Pin pin);`
- `void jshSetOutputValue(JshPinFunction func, int value);`
- `void jshEnableWatchDog(JsVarFloat timeout);`
- `void jshKickWatchDog();`
- `void jshKickSoftWatchDog();`
- `bool jshGetWatchedPinState(IOEventFlags device);`
- `bool jshIsEventForPin(IOEventFlags eventFlags, Pin pin);`
- `bool jshIsDeviceInitialised(IOEventFlags device);`
- `void jshUSARTInitInfo(JshUSARTInfo *inf);`
- `void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf);`
- `void jshUSARTUnSetup(IOEventFlags device);`
- `void jshUSARTKick(IOEventFlags device);`
- `void jshSPIInitInfo(JshSPIInfo *inf);`
- `void jshSPISetup(IOEventFlags device, JshSPIInfo *inf);`
- `int jshSPISend(IOEventFlags device, int data);`
- `void jshSPISend16(IOEventFlags device, int data);`
- `void jshSPISet16(IOEventFlags device, bool is16);`
- `void jshSPISetReceive(IOEventFlags device, bool isReceive);`
- `void jshSPIWait(IOEventFlags device);`
- `void jshI2CInitInfo(JshI2CInfo *inf);`
- `void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf);`
- `void jshI2CUnSetup(IOEventFlags device);`
- `void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop);`
- `void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop);`
- `bool jshFlashGetPage(uint32_t addr, uint32_t *startAddr, uint32_t *pageSize);`
- `JsVar *jshFlashGetFree();`
- `void jshFlashErasePage(uint32_t addr);`
- `bool jshFlashErasePages(uint32_t addr, uint32_t byteLength);`
- `void jshFlashRead(void *buf, uint32_t addr, uint32_t len);`
- `void jshFlashWrite(void *buf, uint32_t addr, uint32_t len);`
- `void jshFlashWriteAligned(void *buf, uint32_t addr, uint32_t len);`
- `size_t jshFlashGetMemMapAddress(size_t ptr);`
- `void jshUtilTimerStart(JsSysTime period);`
- `void jshUtilTimerReschedule(JsSysTime period);`
- `void jshUtilTimerDisable();`
- `volatile uint32_t *jshGetPinAddress(Pin pin, JshGetPinAddressFlags flags);`
- `void jshSetupRTCPrescalerValue(unsigned int prescale);`
- `int jshGetRTCPrescalerValue(bool calibrate);`
- `void jshResetRTCTimer();`
- `void jshClearUSBIdleTimeout();`
- `JsVarFloat jshReadVRef();`
- `JsVarFloat jshReadVDDH();`
- `unsigned int jshGetRandomNumber();`
- `unsigned int jshSetSystemClock(JsVar *options);`
- `JsVar *jshGetSystemClock();`
- `void jshReboot();`
- `void jshVirtualPinSetValue(Pin pin, bool state);`
- `bool jshVirtualPinGetValue(Pin pin);`
- `JsVarFloat jshVirtualPinGetAnalogValue(Pin pin);`
- `void jshVirtualPinSetState(Pin pin, JshPinState state);`
- `JshPinState jshVirtualPinGetState(Pin pin);`
