/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: i2c_master.c
 *
 * Description: i2c master API
 *
 * Modification history:
 *     2014/3/12, v1.0 create this file.
 *     2015/10/11, modified to work with any pin for Espruino
 *     2016/05/23, modified to work with clock streching and bitrate
 *
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the Software), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "espmissingincludes.h"

#include "i2c_master.h"

#define I2C_MASTER_SDA_SCL(sda, scl) do {\
  uint32 set = ((sda&1)<<pinSDA) | ((scl&1)<<pinSCL);\
  uint32 both = (1<<pinSDA) | (1<<pinSCL);\
  gpio_output_set(set, set^both, both, 0);\
} while(0)

#define i2c_master_wait(x)    os_delay_us(x*y)

#define i2c_master_getDC(void) ((GPIO_REG_READ(GPIO_IN_ADDRESS) >> pinSDA) & 1)
#define i2c_master_getDD(void) ((GPIO_REG_READ(GPIO_IN_ADDRESS) >> pinSCL) & 1)

LOCAL uint8 pinSDA, pinSCL; // actual gpio pins used
LOCAL uint8 y=1;
/******************************************************************************
 * FunctionName : i2c_master_setDC
 * Description  : Internal used function -
 *                    set i2c SDA and SCL bit value for half clk cycle
 * Parameters   : uint8 SDA
 *                uint8 SCL
 * Returns      : NONE
*******************************************************************************/
LOCAL void CALLED_FROM_INTERRUPT
i2c_master_setDC(uint8 SDA, uint8 SCL)
{
    SDA &= 0x01;
    SCL &= 0x01;

    I2C_MASTER_SDA_SCL(SDA, SCL);
  
}

/******************************************************************************
 * FunctionName : i2c_master_getDC
 * Description  : Internal used function -
 *                    get i2c SDA bit value
 * Parameters   : NONE
 * Returns      : uint8 - SDA bit value
*******************************************************************************/
#if 0
LOCAL uint8 CALLED_FROM_INTERRUPT
i2c_master_getDC(void)
{
    return (GPIO_REG_READ(GPIO_IN_ADDRESS) >> pinSDA) & 1;
}
#endif

/******************************************************************************
 * FunctionName : i2c_master_init
 * Description  : initilize I2C bus to enable i2c operations
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_init(void)
{
    uint8 i;

    i2c_master_setDC(1, 0);
    i2c_master_wait(2);

    // when SCL = 0, toggle SDA to clear up
    i2c_master_setDC(0, 0) ;
    i2c_master_wait(2);
    i2c_master_setDC(1, 0) ;
    i2c_master_wait(2);

    // set data_cnt to max value
    for (i = 0; i < 28; i++) {
        i2c_master_setDC(1, 0);
        i2c_master_wait(2); // sda 1, scl 0
        i2c_master_setDC(1, 1);
        i2c_master_wait(2); // sda 1, scl 1
    }

    return;
}

/******************************************************************************
 * FunctionName : i2c_master_gpio_init
 * Description  : config SDA and SCL gpio to open-drain output mode,
 *                mux and gpio num defined in i2c_master.h
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_gpio_init(uint8 scl, uint8 sda, uint32 bitrate)
{
    pinSCL = scl;
    pinSDA = sda;
    if(bitrate > 400000) {bitrate=400000;}
    if(bitrate < 100000) {bitrate=100000;}
    y = 400000/bitrate;

#if 0
    ETS_GPIO_INTR_DISABLE();
    GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(sda)),
        GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(sda))) |
        GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS,
        GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << sda));
    GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(scl)),
        GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(scl))) |
        GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS,
        GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << scl));
    I2C_MASTER_SDA_SCL(1, 1);
    ETS_GPIO_INTR_ENABLE() ;
#endif

    I2C_MASTER_SDA_SCL(1, 1);
    i2c_master_init();
}

/******************************************************************************
 * FunctionName : i2c_master_start
 * Description  : set i2c to send state
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_start(void)
{
    i2c_master_setDC(0, 1);
    i2c_master_wait(1); // sda 0, scl 1
    i2c_master_setDC(0, 0);
    i2c_master_wait(1); // sda 0, scl 0
}

/******************************************************************************
 * FunctionName : i2c_master_stop
 * Description  : set i2c to stop sending state
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_stop(void)
{
    i2c_master_setDC(0, 1);
    while (!i2c_master_getDD()) {}; // PB
    i2c_master_setDC(1, 1);
    i2c_master_wait(1); // sda 1, scl 1
}

/******************************************************************************
 * FunctionName : i2c_master_setAck
 * Description  : set ack to i2c bus as level value
 * Parameters   : uint8 level - 0 or 1
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_setAck(uint8 level)
{
    i2c_master_setDC(level, 0);
    i2c_master_wait(1); // sda level, scl 0
    i2c_master_setDC(level, 1);
    while (!i2c_master_getDD()) {}; // PB
    i2c_master_setDC(level, 0);
    i2c_master_wait(1); // sda level, scl 0
    i2c_master_setDC(1, 0);
    i2c_master_wait(1);
}

/******************************************************************************
 * FunctionName : i2c_master_getAck
 * Description  : confirm if peer send ack
 * Parameters   : NONE
 * Returns      : uint8 - ack value, 0 or 1
*******************************************************************************/
uint8 ICACHE_FLASH_ATTR
i2c_master_getAck(void)
{
    uint8 retVal;
    i2c_master_setDC(1, 0);
    i2c_master_wait(1);
    i2c_master_setDC(1, 1);
    while (!i2c_master_getDD()) {}; // PB
    retVal = i2c_master_getDC();
    i2c_master_setDC(1, 0);
    i2c_master_wait(1);

    return !retVal; // 0->true->ACK, 1->false->NACK
}

/******************************************************************************
* FunctionName : i2c_master_checkAck
* Description  : get dev response
* Parameters   : NONE
* Returns      : true : get ack ; false : get nack
*******************************************************************************/
bool ICACHE_FLASH_ATTR
i2c_master_checkAck(void)
{
    if(i2c_master_getAck()){
        return FALSE;
    }else{
        return TRUE;
    }
}

/******************************************************************************
* FunctionName : i2c_master_send_ack
* Description  : response ack
* Parameters   : NONE
* Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_send_ack(void)
{
    i2c_master_setAck(0x0);
}
/******************************************************************************
* FunctionName : i2c_master_send_nack
* Description  : response nack
* Parameters   : NONE
* Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_send_nack(void)
{
    i2c_master_setAck(0x1);
}

/******************************************************************************
 * FunctionName : i2c_master_readByte
 * Description  : read Byte from i2c bus
 * Parameters   : NONE
 * Returns      : uint8 - readed value
*******************************************************************************/
uint8 ICACHE_FLASH_ATTR
i2c_master_readByte(void)
{
    uint8 retVal = 0;
    uint8 i;

    for (i = 0; i < 8; i++) {
        i2c_master_setDC(1, 1);
        while (!i2c_master_getDD()) {}; // PB
        retVal = (retVal<<1) | (i2c_master_getDC()&1);
        i2c_master_setDC(1, 0);
        i2c_master_wait(1); // sda 1, scl 0
    }

    return retVal;
}

/******************************************************************************
 * FunctionName : i2c_master_writeByte
 * Description  : write wrdata value(one byte) into i2c
 * Parameters   : uint8 wrdata - write value
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_writeByte(uint8 wrdata)
{
    uint8 dat;
    sint8 i;
  
    for (i = 7; i >= 0; i--) {
        dat = wrdata >> i;
        i2c_master_setDC(dat, 0);
        i2c_master_setDC(dat, 1);
        while (!i2c_master_getDD()) {}; // PB
        i2c_master_setDC(dat, 0);
        i2c_master_wait(1);
    }
}