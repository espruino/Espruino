# jshardware Function Matrix

Generated from `src/jshardware.h`.

| Function | Core Usage | Uses | STM32 | NRF5x | ESP32 | Notes |
|---|---|---:|---|---|---|---|
| `jshReset` | used by core/libs | 10 | yes | yes | yes | libs/pipboy/jswrap_pipboy.c | src/jshardware.h | src/jsinteractive.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshIdle` | used by core/libs | 9 | yes | yes | yes | src/jshardware.h | src/jsinteractive.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshBusyIdle` | used by core/libs | 5 |  | yes |  | src/jsdevices.c | src/jshardware.h | src/jshardware_common.c | targets/emscripten/jshardware.c | targets/nrf5x/jshardware.c |
| `jshSleep` | used by core/libs | 10 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | src/jshardware.h | src/jsinteractive.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshKill` | used by core/libs | 15 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | src/jshardware.h | src/jswrap_espruino.c | targets/emscripten/jshardware.c | targets/emscripten/main.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/linux/main.c | targets/nrf5x/jshardware.c | targets/nrf5x/main.c | targets/stm32/jshardware.c |
| `jshGetSerialNumber` | used by core/libs | 9 | yes | yes | yes | src/jshardware.h | src/jswrap_interactive.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshIsUSBSERIALConnected` | used by core/libs | 13 | yes | yes | yes | src/jsdevices.c | src/jshardware.h | src/jsinteractive.c | src/jswrap_serial.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_boot/main.c | targets/stm32_boot/utils.c |
| `jshGetSystemTime` | used by core/libs | 32 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | libs/filesystem/fat_sd/spi_diskio.c | libs/misc/heartrate.c | libs/misc/heartrate_vc31_binary.c | libs/misc/hrm_emulated.c | libs/network/cc3000/evnt_handler.c | libs/network/esp8266/jswrap_esp8266_network.c | libs/network/wiznet/DHCP/dhcp.c | libs/network/wiznet/DNS/dns.c | libs/network/wiznet/network_wiznet.c | libs/pipboy/jswrap_pipboy.c | libs/trigger/jswrap_trigger.c |
| `jshSetSystemTime` | used by core/libs | 10 | yes | yes | yes | src/jshardware.h | src/jswrap_interactive.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/bluetooth.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshGetTimeFromMilliseconds` | used by core/libs | 32 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | libs/filesystem/fat_sd/spi_diskio.c | libs/microbit/jswrap_microbit.c | libs/misc/hrm_analog.c | libs/misc/hrm_emulated.c | libs/network/cc3000/evnt_handler.c | libs/network/wiznet/DHCP/dhcp.h | libs/network/wiznet/DNS/dns.c | libs/network/wiznet/network_wiznet.c | libs/pipboy/jswrap_pipboy.c | libs/puckjs/jswrap_puck.c | libs/swdcon/jswrap_swdcon.c |
| `jshGetMillisecondsFromTime` | used by core/libs | 19 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | libs/misc/heartrate.c | libs/misc/heartrate_vc31_binary.c | libs/misc/hrm_emulated.c | libs/pipboy/jswrap_pipboy.c | libs/trigger/jswrap_trigger.c | src/jshardware.h | src/jsinteractive.c | src/jsserial.c | src/jswrap_process.c | src/jswrap_timer.c | targets/embed/main.c |
| `jshInterruptOff` | used by core/libs | 20 | yes | yes | yes | libs/neopixel/jswrap_neopixel.c | libs/pipboy/stm32_i2s.c | libs/tv/tv.c | src/jsdevices.c | src/jshardware.h | src/jstimer.c | src/jsvar.c | src/jswrap_interactive.c | src/jswrap_onewire.c | src/jswrap_spi_i2c.c | src/jswrap_timer.c | targets/embed/main.c |
| `jshInterruptOn` | used by core/libs | 20 | yes | yes | yes | libs/neopixel/jswrap_neopixel.c | libs/pipboy/stm32_i2s.c | libs/tv/tv.c | src/jsdevices.c | src/jshardware.h | src/jstimer.c | src/jsvar.c | src/jswrap_interactive.c | src/jswrap_onewire.c | src/jswrap_spi_i2c.c | src/jswrap_timer.c | targets/embed/main.c |
| `jshIsInInterrupt` | used by core/libs | 12 | yes | yes | yes | libs/graphics/jswrap_terminal.c | src/jsdevices.c | src/jshardware.h | src/jsvar.c | targets/embed/main.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshDelayMicroseconds` | used by core/libs | 32 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | libs/filesystem/jswrap_file.c | libs/graphics/lcd_fsmc.c | libs/graphics/lcd_memlcd.c | libs/graphics/lcd_spi_unbuf.c | libs/graphics/lcd_spilcd.c | libs/graphics/lcd_st7789_8bit.c | libs/hexbadge/jswrap_hexbadge.c | libs/misc/hrm_vc31.c | libs/misc/jswrap_esp32_cyd.c | libs/network/cc3000/board_spi.c | libs/network/wiznet/DNS/dns.c |
| `jshPinSetValue` | used by core/libs | 34 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | libs/graphics/lcd_fsmc.c | libs/graphics/lcd_memlcd.c | libs/graphics/lcd_spi_unbuf.c | libs/graphics/lcd_spilcd.c | libs/graphics/lcd_st7789_8bit.c | libs/hexbadge/jswrap_hexbadge.c | libs/joltjs/jswrap_jolt.c | libs/network/cc3000/board_spi.c | libs/nordic_thingy/jswrap_thingy.c | libs/pixljs/jswrap_pixljs.c | libs/puckjs/jswrap_puck.c |
| `jshPinGetValue` | used by core/libs | 29 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | libs/filesystem/jswrap_file.c | libs/graphics/lcd_fsmc.c | libs/joltjs/jswrap_jolt.c | libs/misc/jswrap_esp32_cyd.c | libs/network/cc3000/board_spi.c | libs/pipboy/jswrap_pipboy.c | libs/pixljs/jswrap_pixljs.c | libs/puckjs/jswrap_puck.c | src/jsdevices.c | src/jsflash.c | src/jshardware.h |
| `jshPinSetState` | used by core/libs | 29 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | libs/graphics/lcd_fsmc.c | libs/joltjs/jswrap_jolt.c | libs/microbit/jswrap_microbit.c | libs/misc/jswrap_esp32_cyd.c | libs/neopixel/jswrap_neopixel.c | libs/network/cc3000/board_spi.c | libs/nordic_thingy/jswrap_thingy.c | libs/pipboy/jswrap_pipboy.c | libs/pixljs/jswrap_pixljs.c | libs/puckjs/jswrap_puck.c | libs/tv/tv.c |
| `jshPinGetState` | used by core/libs | 13 | yes | yes | yes | libs/puckjs/jswrap_puck.c | src/jshardware.h | src/jsinteractive.c | src/jswrap_espruino.c | src/jswrap_io.c | src/jswrap_pin.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c |
| `jshIsPinStateDefault` | used by core/libs | 5 |  |  | yes | src/jshardware.h | src/jshardware_common.c | src/jsinteractive.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c |
| `jshPinAnalog` | used by core/libs | 15 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | libs/joltjs/jswrap_jolt.c | libs/misc/hrm_analog.c | libs/pixljs/jswrap_pixljs.c | libs/puckjs/jswrap_puck.c | src/jshardware.h | src/jswrap_pin.c | src/jswrap_waveform.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c |
| `jshPinAnalogFast` | used by core/libs | 9 | yes | yes | yes | src/jshardware.h | src/jstimer.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshPinAnalogOutput` | used by core/libs | 13 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | libs/graphics/lcd_memlcd.c | libs/puckjs/jswrap_puck.c | libs/tv/tv.c | src/jshardware.h | src/jswrap_io.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c |
| `jshCanWatch` | used by core/libs | 9 | yes | yes | yes | src/jshardware.h | src/jswrap_io.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshPinWatch` | used by core/libs | 15 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | libs/misc/hrm_vc31.c | libs/puckjs/jswrap_puck.c | libs/trigger/jswrap_trigger.c | src/jshardware.h | src/jsinteractive.c | src/jsserial.c | src/jswrap_io.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c |
| `jshGetCurrentPinFunction` | used by core/libs | 9 | yes | yes | yes | src/jshardware.h | src/jstimer.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshSetOutputValue` | used by core/libs | 9 | yes | yes | yes | src/jshardware.h | src/jstimer.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshEnableWatchDog` | used by core/libs | 10 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | src/jshardware.h | src/jswrap_espruino.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshKickWatchDog` | used by core/libs | 16 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | libs/graphics/jswrap_graphics.c | src/jsflash.c | src/jshardware.h | src/jshardware_common.c | src/jsinteractive.c | src/jsvar.c | src/jswrap_espruino.c | targets/embed/main.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c |
| `jshKickSoftWatchDog` | used by core/libs | 5 |  | yes |  | src/jsflash.c | src/jshardware.h | src/jshardware_common.c | src/jsvar.c | targets/nrf5x/jshardware.c |
| `jshGetWatchedPinState` | used by core/libs | 9 | yes | yes | yes | src/jsdevices.c | src/jshardware.h | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshIsEventForPin` | used by core/libs | 10 | yes | yes | yes | libs/trigger/trigger.c | src/jshardware.h | src/jsinteractive.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshIsDeviceInitialised` | used by core/libs | 15 | yes | yes | yes | src/jshardware.h | src/jsinteractive.c | src/jsserial.c | src/jsspi.c | src/jswrap_serial.c | src/jswrap_spi_i2c.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp32/jshardwareI2c.c | targets/esp32/jshardwareSpi.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c |
| `jshUSARTInitInfo` | used by core/libs | 8 | yes | yes |  | libs/banglejs/jswrap_bangle.c | src/jshardware.h | src/jshardware_common.c | src/jsinteractive.c | src/jsserial.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshUSARTSetup` | used by core/libs | 12 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | src/jshardware.h | src/jsinteractive.c | src/jsserial.c | src/jswrap_serial.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshUSARTUnSetup` | used by core/libs | 6 | yes | yes |  | libs/pipboy/jswrap_pipboy.c | src/jshardware.h | src/jshardware_common.c | src/jswrap_serial.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c |
| `jshUSARTKick` | used by core/libs | 11 | yes | yes | yes | libs/pipboy/jswrap_pipboy.c | src/jsdevices.c | src/jshardware.h | src/jsinteractive.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshSPIInitInfo` | used by core/libs | 13 |  |  |  | libs/filesystem/jswrap_file.c | libs/graphics/lcd_memlcd.c | libs/graphics/lcd_spi_unbuf.c | libs/graphics/lcd_spilcd.c | libs/misc/jswrap_esp32_cyd.c | libs/neopixel/jswrap_neopixel.c | libs/network/cc3000/jswrap_cc3000.c | libs/network/wiznet/jswrap_wiznet.c | libs/tv/tv.c | src/jshardware.h | src/jshardware_common.c | src/jsspi.c |
| `jshSPISetup` | used by core/libs | 19 | yes | yes | yes | libs/filesystem/jswrap_file.c | libs/graphics/lcd_memlcd.c | libs/graphics/lcd_spi_unbuf.c | libs/graphics/lcd_spilcd.c | libs/neopixel/jswrap_neopixel.c | libs/network/cc3000/jswrap_cc3000.c | libs/network/wiznet/jswrap_wiznet.c | libs/tv/tv.c | src/jshardware.h | src/jsspi.c | src/jswrap_spi_i2c.c | targets/emscripten/jshardware.c |
| `jshSPISend` | used by core/libs | 16 | yes | yes | yes | libs/graphics/lcd_pcd8544.c | libs/graphics/lcd_spi_unbuf.c | libs/graphics/lcd_spilcd.c | libs/network/cc3000/board_spi.c | libs/network/wiznet/jswrap_wiznet.c | src/jshardware.h | src/jshardware_common.c | src/jsspi.c | targets/emscripten/jshardware.c | targets/esp32/jshardwareSpi.c | targets/esp32/jshardwareSpi.h | targets/esp8266/jshardware.c |
| `jshSPISend16` | used by core/libs | 10 | yes | yes | yes | src/jshardware.h | src/jsspi.c | targets/emscripten/jshardware.c | targets/esp32/jshardwareSpi.c | targets/esp32/jshardwareSpi.h | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshSPISet16` | used by core/libs | 11 | yes | yes | yes | libs/neopixel/jswrap_neopixel.c | src/jshardware.h | src/jswrap_spi_i2c.c | targets/emscripten/jshardware.c | targets/esp32/jshardwareSpi.c | targets/esp32/jshardwareSpi.h | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshSPISetReceive` | used by core/libs | 12 | yes | yes | yes | libs/neopixel/jswrap_neopixel.c | src/jshardware.h | src/jsspi.c | src/jswrap_spi_i2c.c | targets/emscripten/jshardware.c | targets/esp32/jshardwareSpi.c | targets/esp32/jshardwareSpi.h | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshSPIWait` | used by core/libs | 14 | yes | yes | yes | libs/graphics/lcd_spilcd.c | libs/neopixel/jswrap_neopixel.c | src/jshardware.h | src/jshardware_common.c | src/jsspi.c | src/jswrap_spi_i2c.c | targets/emscripten/jshardware.c | targets/esp32/jshardwareSpi.c | targets/esp32/jshardwareSpi.h | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c |
| `jshI2CInitInfo` | used by core/libs | 7 |  |  |  | libs/banglejs/jswrap_bangle.c | libs/microbit/jswrap_microbit.c | libs/pipboy/jswrap_pipboy.c | libs/puckjs/jswrap_puck.c | src/jshardware.h | src/jshardware_common.c | src/jsi2c.c |
| `jshI2CSetup` | used by core/libs | 12 | yes | yes | yes | libs/microbit/jswrap_microbit.c | libs/pipboy/jswrap_pipboy.c | src/jshardware.h | src/jswrap_spi_i2c.c | targets/emscripten/jshardware.c | targets/esp32/jshardwareI2c.c | targets/esp32/jshardwareI2c.h | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshI2CUnSetup` | used by core/libs | 4 | yes |  |  | libs/pipboy/jswrap_pipboy.c | src/jshardware.h | src/jshardware_common.c | targets/stm32/jshardware.c |
| `jshI2CWrite` | used by core/libs | 12 | yes | yes | yes | libs/microbit/jswrap_microbit.c | libs/pipboy/jswrap_pipboy.c | src/jshardware.h | src/jswrap_spi_i2c.c | targets/emscripten/jshardware.c | targets/esp32/jshardwareI2c.c | targets/esp32/jshardwareI2c.h | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshI2CRead` | used by core/libs | 12 | yes | yes | yes | libs/microbit/jswrap_microbit.c | libs/pipboy/jswrap_pipboy.c | src/jshardware.h | src/jswrap_spi_i2c.c | targets/emscripten/jshardware.c | targets/esp32/jshardwareI2c.c | targets/esp32/jshardwareI2c.h | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshFlashGetPage` | used by core/libs | 11 | yes | yes | yes | src/jsflash.c | src/jshardware.h | src/jshardware_common.c | src/jswrap_flash.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshFlashGetFree` | used by core/libs | 10 | yes | yes | yes | src/jshardware.h | src/jswrap_flash.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/esp8266/jswrap_esp8266.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshFlashErasePage` | used by core/libs | 13 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | src/jsflash.c | src/jshardware.h | src/jshardware_common.c | src/jswrap_flash.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/bluetooth.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c |
| `jshFlashErasePages` | used by core/libs | 4 |  | yes |  | src/jsflash.c | src/jshardware.h | src/jshardware_common.c | targets/nrf5x/jshardware.c |
| `jshFlashRead` | used by core/libs | 16 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | libs/joltjs/jswrap_jolt.c | libs/puckjs/jswrap_puck.c | src/jsflash.c | src/jshardware.h | src/jshardware_common.c | src/jsvariterator.h | src/jswrap_flash.c | src/jswrap_storage.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c |
| `jshFlashWrite` | used by core/libs | 12 | yes | yes | yes | libs/joltjs/jswrap_jolt.c | libs/puckjs/jswrap_puck.c | src/jsflash.c | src/jshardware.h | src/jshardware_common.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshFlashWriteAligned` | used by core/libs | 4 |  |  |  | src/jsflash.c | src/jshardware.h | src/jshardware_common.c | src/jswrap_flash.c |
| `jshFlashGetMemMapAddress` | used by core/libs | 11 | yes | yes | yes | src/jsflash.c | src/jshardware.h | src/jswrap_espruino.c | src/jswrap_io.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshUtilTimerStart` | used by core/libs | 9 | yes | yes | yes | src/jshardware.h | src/jstimer.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshUtilTimerReschedule` | used by core/libs | 9 | yes | yes | yes | src/jshardware.h | src/jstimer.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshUtilTimerDisable` | used by core/libs | 10 | yes | yes | yes | src/jshardware.h | src/jstimer.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/bluetooth.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshGetPinAddress` | used by core/libs | 5 | yes |  |  | src/jshardware.h | src/jswrap_io.c | src/jswrap_pin.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshSetupRTCPrescalerValue` | used by core/libs | 3 | yes |  |  | src/jshardware.h | src/jswrap_espruino.c | targets/stm32/jshardware.c |
| `jshGetRTCPrescalerValue` | used by core/libs | 3 | yes |  |  | src/jshardware.h | src/jswrap_espruino.c | targets/stm32/jshardware.c |
| `jshResetRTCTimer` | used by core/libs | 3 | yes |  |  | src/jshardware.h | src/jswrap_espruino.c | targets/stm32/jshardware.c |
| `jshClearUSBIdleTimeout` | used by core/libs | 4 | yes |  |  | src/jshardware.h | targets/stm32/jshardware.c | targets/stm32_boot/utils.c | targets/stm32_ll/jshardware.c |
| `jshReadVRef` | used by core/libs | 13 | yes | yes | yes | libs/bluetooth/jswrap_bluetooth.c | libs/joltjs/jswrap_jolt.c | libs/pixljs/jswrap_pixljs.c | libs/puckjs/jswrap_puck.c | src/jshardware.h | src/jswrap_espruino.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c |
| `jshReadVDDH` | used by core/libs | 3 |  | yes |  | src/jshardware.h | src/jswrap_espruino.c | targets/nrf5x/jshardware.c |
| `jshGetRandomNumber` | used by core/libs | 9 | yes | yes | yes | libs/network/network.c | src/jshardware.h | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshSetSystemClock` | used by core/libs | 10 | yes | yes | yes | src/jshardware.h | src/jswrap_espruino.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp8266/jshardware.c | targets/esp8266/jswrap_esp8266.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshGetSystemClock` | used by core/libs | 4 | yes |  |  | src/jshardware.h | src/jshardware_common.c | src/jswrap_espruino.c | targets/stm32/jshardware.c |
| `jshReboot` | used by core/libs | 12 | yes | yes | yes | libs/banglejs/jswrap_bangle.c | src/jshardware.h | src/jswrap_espruino.c | targets/emscripten/jshardware.c | targets/esp32/jshardware.c | targets/esp32/jswrap_esp32.c | targets/esp8266/jshardware.c | targets/esp8266/jswrap_esp8266.c | targets/linux/jshardware.c | targets/nrf5x/jshardware.c | targets/stm32/jshardware.c | targets/stm32_ll/jshardware.c |
| `jshVirtualPinSetValue` | used by core/libs | 4 |  | yes |  | libs/microbit/jswrap_microbit.c | libs/nordic_thingy/jswrap_thingy.c | src/jshardware.h | targets/nrf5x/jshardware.c |
| `jshVirtualPinGetValue` | used by core/libs | 4 |  | yes |  | libs/microbit/jswrap_microbit.c | libs/nordic_thingy/jswrap_thingy.c | src/jshardware.h | targets/nrf5x/jshardware.c |
| `jshVirtualPinGetAnalogValue` | used by core/libs | 4 |  | yes |  | libs/microbit/jswrap_microbit.c | libs/nordic_thingy/jswrap_thingy.c | src/jshardware.h | targets/nrf5x/jshardware.c |
| `jshVirtualPinSetState` | used by core/libs | 4 |  | yes |  | libs/microbit/jswrap_microbit.c | libs/nordic_thingy/jswrap_thingy.c | src/jshardware.h | targets/nrf5x/jshardware.c |
| `jshVirtualPinGetState` | used by core/libs | 4 |  | yes |  | libs/microbit/jswrap_microbit.c | libs/nordic_thingy/jswrap_thingy.c | src/jshardware.h | targets/nrf5x/jshardware.c |

## Full Prototypes

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
