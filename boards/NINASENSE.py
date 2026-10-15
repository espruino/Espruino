#!/bin/false
import pinutils

info = {
  'name' : 'NINASENSE Rev 1.1',
  'link' : [ 'https://www.blusense.com.br/ninasense.html', 'https://github.com/nearmeter/nrfclaw' ],
  'default_console' : 'EV_SERIAL1',
  'default_console_tx' : 'D15',
  'default_console_rx' : 'D16',
  'default_console_baudrate' : '115200',
  'variables' : 2400,
  'binary_name' : 'espruino_%v_ninasense.hex',
  'build' : {
    'optimizeflags' : '-Os',
    'libraries' : [ 'BLUETOOTH' ],
    'makefile' : [
      'NRF_SDK17=1',
      'DEFINES+=-DNINASENSE',
      'DEFINES+=-DESPR_LSE_ENABLE',
      'DEFINES+=-DNRF_SDH_CLOCK_LF_SRC=1',
      'DEFINES+=-DNRF_SDH_CLOCK_LF_ACCURACY=7',
      'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"NINASENSE"\'',
      'DEFINES+=-DBOARD_PCA10040 -DPCA10040',
      'DEFINES+=-DCONFIG_NFCT_PINS_AS_GPIOS',
      'DEFINES+=-DESPR_DCDC_ENABLE',
      'DEFINES+=-DNRF_BLE_GATT_MAX_MTU_SIZE=53 -DNRF_BLE_MAX_MTU_SIZE=53',
      'DEFINES+=-DCENTRAL_LINK_COUNT=2 -DNRF_SDH_BLE_CENTRAL_LINK_COUNT=2',
      'LDFLAGS+=-Xlinker --defsym=LD_APP_RAM_BASE=0x2E40',
      'LDFLAGS+=-nostartfiles',
      'ASFLAGS+=-D__STARTUP_CLEAR_BSS -D__START=main',
      'INCLUDE+=-I$(ROOT)/libs/nrfclaw_hw',
      'WRAPPERSOURCES+=libs/nrfclaw_hw/jswrap_ninasense.c',
      'SOURCES+=libs/nrfclaw_hw/nrfclaw_espruino.c',
      'SOURCES+=libs/nrfclaw_hw/nrfclaw_llcc68.c',
      'SOURCES+=libs/nrfclaw_hw/nrfclaw_lora_profile.c',
      'SOURCES+=libs/nrfclaw_hw/nrfclaw_battery.c',
    ]
  }
}

chip = {
  'part' : 'NRF52832', 'family' : 'NRF52', 'package' : 'QFN48',
  'ram' : 64, 'flash' : 512, 'speed' : 64,
  'usart' : 1, 'spi' : 1, 'i2c' : 1, 'adc' : 1, 'dac' : 0,
  'saved_code' : {
    # nRFClaw DFU accepts application bytes only below 0x6C000.
    # 0x6C000..0x75FFF remains Espruino Storage (10 x 4 KiB).
    'address' : 0x6C000,
    'page_size' : 4096,
    'pages' : 10,
    # S132 v7 app starts at 0x26000: 0x6C000-0x26000 = 280 KiB.
    'flash_available' : 280
  }
}

devices = {
  'BTN1' : { 'pin' : 'D21', 'pinstate' : 'IN_PULLUP' },
  'RX_PIN_NUMBER' : { 'pin' : 'D16' },
  'TX_PIN_NUMBER' : { 'pin' : 'D15' },
  'LORA' : {
    'device' : 'LLCC68', 'pin_sck':'D2', 'pin_nss':'D3', 'pin_mosi':'D4',
    'pin_miso':'D5', 'pin_busy':'D6', 'pin_rxen':'D7', 'pin_txen':'D8',
    'pin_dio1':'D9', 'pin_dio2':'D10', 'pin_rst':'D19'
  },
  'ACCEL' : {
    'device':'LIS2DH12', 'addr':0x18, 'pin_scl':'D13', 'pin_sda':'D18',
    'pin_cs':'D17', 'pin_int1':'D22', 'pin_int2':'D23'
  },
  'BAT' : { 'pin_voltage':'D28' }
}

board = {
  'left' : ['D2','D3','D4','D5','D6','D7','D8','D9','D10','D11','D13','D14'],
  'right': ['D15','D16','D17','D18','D19','D20','D21','D22','D23','D28','D29','D30','D31'],
  '_notes' : {
    'D2':'LLCC68 SCK', 'D3':'LLCC68 NSS', 'D4':'LLCC68 MOSI', 'D5':'LLCC68 MISO',
    'D6':'LLCC68 BUSY', 'D7':'LLCC68 RXEN', 'D8':'LLCC68 TXEN', 'D9':'LLCC68 DIO1',
    'D10':'LLCC68 DIO2', 'D11':'HALL1', 'D13':'LIS2DH12 SCL', 'D14':'HALL2',
    'D15':'Serial1 TX', 'D16':'Serial1 RX', 'D17':'LIS2DH12 CS', 'D18':'LIS2DH12 SDA',
    'D19':'LLCC68 NRST', 'D20':'DS18B20 pin (no native driver added)',
    'D21':'Recovery/DFU button', 'D22':'LIS2DH12 INT1', 'D23':'LIS2DH12 INT2',
    'D28':'Battery ADC', 'D29':'Analog/free', 'D30':'Analog/free', 'D31':'Analog/free'
  }
}
board['_css'] = ''

def get_pins():
  pins = pinutils.generate_pins(0,31)
  pinutils.findpin(pins,'PD0',True)['functions']['XL1']=0
  pinutils.findpin(pins,'PD1',True)['functions']['XL2']=0
  pinutils.findpin(pins,'PD15',True)['functions']['TXD']=0
  pinutils.findpin(pins,'PD16',True)['functions']['RXD']=0
  for p,ch in [(2,0),(3,1),(4,2),(5,3),(28,4),(29,5),(30,6),(31,7)]:
    pinutils.findpin(pins,'PD%d'%p,True)['functions']['ADC1_IN%d'%ch]=0
  for pin in pins: pin['functions']['3.3']=0
  return pins
