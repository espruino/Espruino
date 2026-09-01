ESP32 IDF5 builds
=================

## Prepare your Espruino build environment

### IDF
Only [Step 1. Install Prerequisites](https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32/get-started/linux-macos-setup.html) is required.


### Repo Espruino - for now

```
git clone -b esp32_5 https://github.com/MaBecker/Espruino.git
```

now you have a folder named Espruino

### Provisoning IDF 5.5.3 for ESP32 devices

```
cd Espruino
source scripts/provision.sh ESP32_IDF5
```
I prefer 
```
cd esp-idf-5
source esp-idf/export.sh
cd ..
```

### Install QEMU emulator

Execute the [prerequisites](https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32/api-guides/tools/qemu.html#prerequisites) for your build systen.

add qemu for xtensa and riscv32
```
python $IDF_PATH/tools/idf_tools.py install qemu-xtensa qemu-riscv32
cd esp-idf-5
source esp-idf/export.sh
cd ..
```

## Build Espruino for ESP32 with IDF5

### Source IDF environment - if you start a new shell

```
cd Espruino
source scripts/provision.sh ESP32_IDF5
```

### Call make

```
cd Espruino
BOARD=ESP32_IDF5 RELEASE=1 make
```

this create bin/build and copies required files from targets/esp32/IDF5 and saves firmaware in release specific folder like `espruino_2v28.2_esp32`

## Migration status - last update 09-March-2026

### Changes required because of newer GCC version

- fixed make issues
- working on ESP32 warning turned to error

---
### Get Espruino up and running in QEMU and allow debugging

- reduce libraries
- used #ifdef to exclude NET and BLUETOOTH
- new rtosutil_idf5 using 5.5.3 GPTimer version - for now 
- include QEMU_BUILD to make command and source code
- reduce variables

```
BOARD=ESP32_IDF5 QEMU_BUILD=1 RELEASE=1 make
```

---

### Next steps

- include devices by using legacy driver - for now
- migrate WIFI
- migrate BLE
- chack make for other ESP devices like  ESP32, ESP32_IDF4 ......

---

## Some AI generated content about headers and their components

### Core System Components

#### `esp_system`

**Headers**

```
esp_system.h
esp_err.h
esp_attr.h
esp_spi_flash.h
esp_cpu.h
```

**Provides**

* Core system APIs
* Reset/restart
* Flash access (`esp_flash_*`)
* IRAM_ATTR / DRAM_ATTR macros

---

### `esp_hw_support`

**Headers**

```
esp_mac.h
esp_chip_info.h
```

**Provides**

* `esp_read_mac()`
* Chip info
* Hardware identification

---

### `efuse`

**Headers**

```
esp_efuse.h
esp_efuse_table.h
```

**Provides**

* eFuse access
* MAC base reads (legacy way)

---

### `freertos`

**Headers**

```
freertos/FreeRTOS.h
freertos/task.h
freertos/queue.h
freertos/semphr.h
freertos/event_groups.h
```

**Provides**

* Tasks
* Queues
* Semaphores
* Event groups

---

### `esp_timer`

**Headers**

```
esp_timer.h
```

**Provides**

* High resolution timers
* esp_timer_start_*

---

## Driver Layer

### `driver`

**Headers**

```
driver/gpio.h
driver/uart.h
driver/spi_master.h
driver/i2c.h
driver/adc.h
driver/ledc.h
driver/rmt.h
driver/timer.h
```

**Provides**

* GPIO
* UART
* SPI
* I2C
* ADC
* PWM (LEDC)
* RMT (Neopixel)

---

## Networking

### `esp_event`

**Headers**

```
esp_event.h
```

**Provides**

* Event loop
* Event handlers

---

### `esp_netif`

**Headers**

```
esp_netif.h
```

**Provides**

* TCP/IP adapter (replaces old tcpip_adapter)
* Network interface setup

---

### `esp_wifi`

**Headers**

```
esp_wifi.h
esp_wifi_types.h
esp_wifi_default.h
```

**Provides**

* WiFi driver
* STA/AP mode
* Scan
* Connect

---

### `lwip`

**Headers**

```
lwip/sockets.h
lwip/netdb.h
lwip/ip_addr.h
lwip/apps/sntp.h
```

**Provides**

* BSD sockets
* TCP/UDP
* DNS
* SNTP

---

### `mdns` (External in IDF 5)

**Headers**

```
mdns.h
```

**Provides**

* mDNS responder
* Service advertising

⚠ In IDF 5 this may require `idf.py add-dependency espressif/mdns`

---

## Bluetooth

### `bt`

**Headers**

```
esp_bt.h
esp_bt_main.h
esp_bt_defs.h
```

**Provides**

* BT controller
* Stack init

---

### `esp_bt`

**Headers**

```
esp_gap_ble_api.h
esp_gattc_api.h
esp_gatts_api.h
esp_spp_api.h
```

**Provides**

* BLE GAP
* BLE GATT
* Classic SPP

---

## Storage / OTA

### `nvs_flash`

**Headers**

```
nvs_flash.h
nvs.h
```

**Provides**

* NVS storage
* WiFi credential storage
* BLE bonding storage

---

### `app_update`

**Headers**

```
esp_ota_ops.h
esp_partition.h
```

**Provides**

* OTA update
* Partition management

---

## Flash / Partitions

### `esp_partition` (usually pulled automatically)

**Headers**

```
esp_partition.h
```

**Provides**

* Partition read/write
* Partition lookup

---

Below is the correct **IDF 5 breakdown of what used to live in `driver`**.

## ESP-IDF 5 Driver Component Breakdown

### GPIO

**Component**

```
esp_driver_gpio
```

**Headers**

```
driver/gpio.h
```

---

### UART

**Component**

```
esp_driver_uart
```

**Headers**

```
driver/uart.h
```

---

### SPI (Master/Slave)

**Component**

```
esp_driver_spi
```

**Headers**

```
driver/spi_master.h
driver/spi_slave.h
```

---

### I2C

**Component**

```
esp_driver_i2c
```

**Headers**

```
driver/i2c.h
```

---

### ADC

**Component**

```
esp_adc
```

**Headers**

```
driver/adc.h
esp_adc/adc_oneshot.h
esp_adc/adc_continuous.h
```

ADC was significantly refactored in IDF 5.

---

### LEDC (PWM)

**Component**

```
esp_driver_ledc
```

**Headers**

```
driver/ledc.h
```

---

### RMT (Neopixel!)

**Component**

```
esp_driver_rmt
```

**Headers**

```
driver/rmt.h
```

RMT was heavily redesigned in IDF 5.

---

### Timer

**Component**

```
esp_driver_timer
```

**Headers**

```
driver/timer.h
```

---

### PCNT (Pulse Counter)

**Component**

```
esp_driver_pcnt
```

**Headers**

```
driver/pulse_cnt.h
```

---

### MCPWM

**Component**

```
esp_driver_mcpwm
```

**Headers**

```
driver/mcpwm.h
```

---

## Important

In **IDF 5**, the old `driver` component still exists for backward compatibility in some configurations, but:

* It no longer guarantees all subdrivers
* It may not export all headers
* It’s not future-proof

You should depend on specific subcomponents instead.

---

## For Espruino ESP32

Based on typical Espruino usage, you likely need:

```
esp_driver_gpio
esp_driver_uart
esp_driver_spi
esp_driver_i2c
esp_driver_ledc
esp_driver_rmt
esp_driver_timer
esp_adc
```
---

## If You See These Errors

| Error         | Missing Component |
| ------------- | ----------------- |
| driver/gpio.h | esp_driver_gpio   |
| driver/rmt.h  | esp_driver_rmt    |
| driver/uart.h | esp_driver_uart   |
| driver/i2c.h  | esp_driver_i2c    |
| driver/ledc.h | esp_driver_ledc   |
| driver/adc.h  | esp_adc           |

---

You can temporarily keep:

```
driver
```

But long-term, replace it with granular driver components.
