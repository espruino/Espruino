Bangle.js 3 Notes
==================

## Building

This is currently a mess due to Zephyr's build process.

* run `make clean;BOARD=BANGLEJS3 RELEASE=1 make cmake` in the main `Espruino` dir
* start `VS Code` with the `NRF Connect SDK` in `Espruino/targets/zephyr`
* Try and build the project for the `banglejs3` target

## UART

Is on TX:P1.06/RX:P1.07 which can be switched onto the external connector by telling the PY32 to enable AUX_SWAP

## PY32

Can be poked by serial:

```JS
var spi = new SPI();
var IRQ = A4;
var CS = A3;
spi.setup({miso:A2,mosi:A1,sck:A0});
spi.send([1,1],CS); // backlight on
spi.send([1,0],CS); // backlight off

// very hacky transfer of SPI data
a = new Uint8Array(721);
a[0]=2;
b = new Uint8Array(720);
CS.reset();spi.send(a);for (var i=1;i<30;i++){b.fill((i&1)?255:0);spi.send(b);}CS.set();
```

```
PA0    - LCD R1
PA1    - LCD R2
PA2    - LCD G1
PA3    - LCD G2
PA4    - LCD B1
PA5    - LCD B2
PA6    - LCD HCLK
PA7    - LCD VCLK
PA8    - LCD HST
PA9    - LCD VST
PA10   - LCD ENB
PA11   - LCD XRST
PA12   - NRF54 NRST
PA13   - SWDIO
PA14   - SWDCLK
PA15   - SPI NSS
PB0    - BUTTONS (ADC)
PB1    - IR input (ADC)
PB2    - AUX IOSwap (0=SWD, 1=GPIO)
PB3    - SPI SCK
PB4    - SPI MISO
PB5    - SPI MOSI
PB6    - LCD Backlight
PB7    - IR
PB8    - Torch
PB9    - AUX power output (on IO connector)
PB10   - Touch RST
PB11   - Touch IRQ
PB12   - WiFi EN
PB13   - RGB EN
PB14   - Speaker EN
PB15   - WiFi Boot Mode
PC13   - SPI IRQ
PC14   - HRM AUX GPIO
PC15   - Charge enable
PF0    - NRF54 SWDIO
PF1    - NRF54 SWDCLK
PF2    - NRST
PF5    - LCD FRP
PF6    - MOTO PWM
PF9    - NC
```