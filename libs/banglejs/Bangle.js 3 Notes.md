Bangle.js 3 Notes
==================

The nRF54L15 has 3 ports - the pin names map as follows:

* P0.x -> Ax (eg P0.09 -> A9)
* P1.x -> Bx
* P2.x -> Cx
* Virtual pins (on PY32) -> Vx


## Building

This is currently a mess due to Zephyr's build process.

* run `make clean;BOARD=BANGLEJS3 RELEASE=1 make cmake` in the main `Espruino` dir
* start `VS Code` with the `NRF Connect SDK` in `Espruino/targets/zephyr`
* Try and build the project for the `banglejs3` target

## UART

Is on TX:P1.06/RX:P1.07 which can be switched onto the external connector by telling the PY32 to enable AUX_SWAP

## I2C

All I2C devices are connected together.

```
i2c = new I2C();
i2c.setup({scl:B4, sda:B5});
// LSM6DSOTR Accel/Gyro
i2c.readReg(106,0x1E,1)&1; // has data?
i2c.readReg(106,0x28,6);
```

* Address 21 = CST816S Touch Panel
* Address 48 = MMC5603NJ Magnetometer
* Address 106 = LSM6DSOTR Accel/Gyro
* Address 118 = BME690 Pressure/etc
* ? = HRM

## nRF54L15

```
P0.00    PY32_SCK
P0.01    PY32_MOSI
P0.02    PY32_MISO
P0.03    PY32_CS
P0.04    PY32_IRQ (input)
P1.00/1  32k Osc
P1.02    HR_INT
P1.03    ACC_GYRO_INT
P1.04    SCL
P1.05    SDA
P1.06    External IO 1 (UART TX)
P1.07    External IO 2 (UART RX)
P1.08    Speaker analog out
P1.09    PY32 SWDIO
P1.10    PY32 SWDCLK
P1.11    Microphone Enable
P1.12    Microphone Analog In
P1.13    unused
P1.14    VBAT_ADC
P2.00    Flash D3
P2.01    Flash SCK
P2.02    Flash D0
P2.03    Flash D2
P2.04    Flash D1
P2.05    Flash CS
P2.06    Vibration PWM
P2.07    ESP32 UART RX
P2.08    ESP32 UART TX
P2.09    LiPo Charge detect
P2.10    Torch RGB (neopixel)
```

### Power usage debugging

```JS
for (i=0;i<=C10;i++) print(Pin(i), E.toJS(Pin(i).getInfo()))
```


## PY32

See [banglejs3_py32/README.md](banglejs3_py32/README.md)

See [banglejs3_py32/src/const.h](banglejs3_py32/src/const.h) for the protocol between devices

Can be poked by serial:

```JS
// NOTE: this is now implemented in firmware
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
PB2    - AUX IOSwap (0=SWD, 1=GPIO)                => V12
PB3    - SPI SCK
PB4    - SPI MISO
PB5    - SPI MOSI
PB6    - LCD Backlight                             => V4
PB7    - IR
PB8    - Torch                                     => V5
PB9    - AUX power output (on IO connector)        => V13
PB10   - Touch RST                                 => V14
PB11   - Touch IRQ
PB12   - WiFi EN                                   => V10
PB13   - RGB EN                                    => V6
PB14   - Speaker EN                                => V7
PB15   - WiFi Boot Mode                            => V11
PC13   - SPI IRQ
PC14   - HRM AUX GPIO                              => V15
PC15   - LiPo Charge enable                        => V9
PF0    - NRF54 SWDIO
PF1    - NRF54 SWDCLK
PF2    - NRST
PF5    - LCD FRP
PF6    - Vibration EN                              => V8
PF9    - NC
```

## WiFi

Using ESP8684H4X

Firmware is ESP32C2-AT from https://github.com/espressif/esp-at/releases

```JS
Serial2.on("data",d => print("WiFi:",d));
V11.set(); // normal mode
//V10.set(); // on
Bangle.setWiFiPower(1);
// 76800 baud: 'invalid header: 0xffffffff' if not flashed
// if flashed, outputs at 115200
V10.reset(); // bootloader mode
```


```JS
// firmware update
pinMode(C8,"input"); // don't force USART
V11.reset(); // bootloader mode
V10.set(); // on
// then on PC with UART connection
esptool --chip esp32c2 --port /dev/ttyUSB0 --baud 921600 \
  write_flash -z --flash_mode dio --flash_freq 60m --flash_size 4MB \
  0x0 ESP32-C2-4MB-AT-V4.1.1.0/factory/factory_ESP32C2-4MB.bin
// finally
V10.reset(); // off
```

### WiFi rebuild

The built files are in `banglejs3_esp32`

```bash
git clone https://github.com/espressif/esp-at --depth=1
edit ./components/customized_partitions/raw_data/factory_param/factory_param_data.csv
# Change PLATFORM_ESP32C2,ESP32C2-4MB,"4MB, Wi-Fi + BluFi, OTA, All ECOs, TX:7 RX:6",4,78,1,1,13,CN,115200,7,6,5,4,1
# To     PLATFORM_ESP32C2,ESP32C2-4MB,"4MB, Wi-Fi + BluFi, OTA, All ECOs, TX:20 RX:19",4,78,0,1,13,CN,115200,20,19,5,4,1

# https://docs.espressif.com/projects/esp-at/en/latest/esp32c2/Compile_and_Develop/How_to_clone_project_and_compile_it.html
./build.py install
./build.py build
choose PLATFORM_ESP32C2
choose ESP32C2-4MB
choose NO logging

# to flash
esptool.py --chip esp32c2 -p /dev/ttyUSB0 -b 460800 --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 60m --flash_size 4MB 0x0 bootloader/bootloader.bin 0x60000 esp-at.bin 0x8000 partition_table/partition-table.bin 0xd000 ota_data_initial.bin 0x1e000 at_customize.bin 0x1f000 customized_partitions/mfg_nvs.bin
```

## Microphone

```JS
// currently fails because of analogRead in ISR
B11.set(); // Mic on
var w = new Waveform(240);
w.on("finish", function(buf) {
  B11.set(); // Mic off
  g.clear(1);
  var l = g.drawLine.bind(g);
  buf.forEach((x,y)=>l(x,120+y));
});
w.startInput(B12,2000,{repeat:false});

// works
B11.set(); // Mic on
setInterval(function() {
  g.clear(1);
  var l = g.lineTo.bind(g), r = analogRead.bind(null,B12);
  for (var x=0;x<240;x++) l(x,r()*2400 - 1240);
},100);
```

## Speaker

```JS
var wave = E.toArrayBuffer(atob("haezvL6+ua+ZX0pAOzpASGKbsLm+v7qwl1xJPTs6RE5/rLa+v7qxllpHPTo9Rl6asru/vLSdYEY+OT9Haqa1vr+4rHdIQTg/RWmlt76+tqJiRT06QlOStbvAt6dnRTw7Ql+itsC7tYpNQThAUJO0vr21kE4/OUFZobfAua1sQjs+S4m3vb2vcUI9PE+Pu7u+oV48Pj1srL28s3ZAPTxgpry9tHk/PjxnrL68qmA8PEePvLy5fUA9P3i4u7yEQj0+erm7uno+PUaTvb2rWTs8ZLS7u3Y+PFCkvb2KPz5Inby+hT89UKe9unM6PWi7uqtLP0Gdur5yPjmAusCJQThuuL+VRDlot8COQDpzvrt9N0OIxq1lL16px4xCOoLCr1wzZra+fDVPoceJPUOZxpM/QZTHk0BDmcaFOE6owm40Yr2vVDeFx5A5U7G6XzWAxpA4WLixTj2cwms0isZ9MnPEkjVqw5g3ZsOYN2nEkjVxx3s0fsdrN567TEy6nTdxxnY3orRDWsWCNJO9SFzFeTWisT1zyFtNwoI4rqM3jb1AccJPV8ZfUMVxQsFzQsFxRMNvSbivur2/vLSnek1GOjw6Rk+AqrS9vr60qnpNRTo7PUhdm6+8vcC1rX9PQjs6QUl2qbW+v7mwjVJEOztBTYKvt8C8tqJoREA4QUh9rbjAvLSXVkM6PENdnre9vradWkQ6PUNqqbfAurN/SUA5QlactcC7tIZKPjpCYae3wbiqZUI6PkyOt768r3JBPTxRlLq8vKNgPD4+aKq+vLR4Qj08WqO9vbV5QT47Zau8vqtiPT1Ehbq8uohEPT1ss7y9kUc9PGy0vLyJQj1AhLu8tGg7PVWqvL6HQT1GlL69nEc+QIy8vpdFPUSVvryJPT5Usrq3XD87h7u/iUE5Z7W/oEk6U6nBqFM2VKjDoU81Xq/DlEU4cru8fDdFjcikVzJmtMB2OEmbx5VDPInGo082fcKrVTR4wqxUNXvDqE45j8WWPEqmw3gyZr6rTjqNx4Q1X7ytSz+ew20ze8aON128qENOtrBNQKq2UD+nt09BqrRJR6+vQFe/lzdpxYEyjMJaQbKoOmfGeDWcuEZYw3g2orI/ZcVkP7SbNJG7RGjJYEvBgDiykjKdqTOXrDaDujqDuDqHtTaOsqnBu8G3sZdhSUE6Oz5JX5muub6/urKbYUk/OjtCTHeqs7+9vbGiZUk+OztFVI+vub++tqlySUE5PURalbS6wbm0jlRDOzpDVJC0usC5sH9KQjhARnWsuMC7s4xOQjhASHyxucG4q25DPTpEZaq2wrivbEY7PUNzrru+uZhUPzo/XKK5vruhWz87P1yivL26kk89PER9try+qWY8Pj1vr729rWg9PT91tby9nlM8PE6Xvby0bz09Qoa6vLl/Pz1Bhrq8tW88PkiXvb6jUjw9aLW7unY9PVCkvryKPz1Lo7u+hkA7U6m9unY6PmO5uq5PPz+Wu755Pzl3ucCTRDhltcCeSTdfscKWRThquL+IPD59wrVxMlCZyZhMNXK7umo1UqfEizxDk8iYRjuJxaFKOITGoko5h8eXQkGZxos4Uq6/bzFwwqNHP5bGejNowaVERae/YzSGx4Q0Z8GgPVe8qEVIsq5IRrCvR0iyrEFQt6U6YcOLNHXGdTSXvlBIuZ02c8dsOaewPmXFazusqjlzxFhHvIgznrI9dcdTVsRrP7qFNaqdM6OhM5GxNZGvNZWrMp2nqMK7wbawk11IQDo7QEljna+6vr+5sZZcSD47O0NOfay0v768sZ1gRz46PEVYlbC6v761pmxHQDk+RF+btbvBubKJUEM6PENYlrW7wLiueElBOUFJe663v7mwhUxCOkJMg7G4vrWmaEU/Pkhsq7S/tKlnSD1CSHqtuLm0kFNDP0Rko7W5tZlZREFFZKO2t7OKT0NCTYOytbegY0NFRnistbWjZERFSX2wtLSUVEVEW5yztKhrRkdNi7KzrXVHR02LsbKpakdIVZixspdVSEl2ra+scEhJXqGwrn1KSlqgrrB9TElhpK6qcElMcK2snVVNUZesrXJNS4CrroZQSnGorY9TS2ymrYpRTXWqqn9MU4KwoG9HZJixildNe6qja0ton6qBTlqRrolVVIusj1hUh6yPWVWJq4dUW5OpgFBnnqJvUHuojllckaZ1Unamj1lhmZ9pVYele1N2o4pXbqGPXGackl1lm5JdZ5uQW26di1p0oH5af590XY6XZGmahlx9nHBiko5ge5pwZZOKYYGWa2yWe2OMjGR/lGt0lHJskntpj4JoioNohodqhYZrhYRthYSFiYeHhYN+eXh6fH18fXx9fH18fXx9fH0="));

var w = new Waveform(wave);
w.on("finish", function() {
  print("Done");
  B8.reset(); // PWM off
  V7.reset(); // speaker off
});
analogWrite(B8, 0.5, {freq:40000});
V7.set();w.startOutput(B8,4000);
```


## Working

* Simple bluetooth comms
* Apps run from internal flash
* Graphics
* Buttons
* Accelerometer
* Magnetometer
* Touchscreen press + swipe gestures.
* Touchscreen 'tap' handlers
* Using external Flash memory in QSPI mode (and 64mbyte)
* WiFi https: requests
* Speaker via analogWrite

## TODO

* Touchscreen sometimes misses lift events (touch IRQ has been missed by PY32)
  * To fix, change display update so we send an SPI display update packet first, and *then* the data
* Graphics updates of <16px (and non-even row start) - using SPI display update packet
* Gyro
* Pressure sensor
* BME690 gas sensing
* Microphone (analogRead in ISR, plus ranging)
* Wrap Waveform handling to allow sounds to be played more easily
* LCD update speed (12fps currently, not async)
* WiFi (Use `AT+CIPRECVMODE=1` for flow control)
* ... much more

## Testing

* Are accelerometer/gyro axes correct? (check on in-device PCB)

```JS
// 4 bpp test
for (i=0;i<4;i++) g.setColor(i/4,i/4,i/4).fillRect(i*60,0,(i+1)*60,239);
// full dither test
for (i=0;i<32;i++) g.setColor(i/32,i/32,i/32).fillRect(i*7,0,(i+1)*7,239);
```
