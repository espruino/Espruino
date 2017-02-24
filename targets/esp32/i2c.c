/*
 * This file is designed to support i2c functions in Espruino,
 * a JavaScript interpreter for Microcontrollers designed by Gordon Williams
 *
 * Copyright (C) 2016 by Rhys Williams (wilberforce)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP32 board specific functions.
 * ----------------------------------------------------------------------------
 */
 
#include "i2c.h"
#include "driver/i2c.h"

#define ACK_CHECK_EN   0x1   /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS  0x0   /*!< I2C master will not check ack from slave */

#define ACK_VAL  0x0     /*!< I2C ack value */
#define NACK_VAL   0x1     /*!< I2C nack value */

/* To do: 
 support both i2c ports - done
 Test!
 Stop bits param -  bool sendStop
  https://esp-idf.readthedocs.io/en/latest/api/i2c.html
  
 */

static esp_err_t checkError( char * caller, esp_err_t ret ) {
  switch(ret) {
    case ESP_OK: break;
  case ESP_ERR_INVALID_ARG: {
    jsError(  "%s:, Parameter error\n", caller );
    break;
  }
  case ESP_FAIL: {
    jsError(  "%s:, slave doesn't ACK the transfer.\n", caller );
    break;
  }
  case ESP_ERR_TIMEOUT: {
    jsError(  "%s:, Operation timeout because the bus is busy.\n", caller );
    break;
  }
  default: {
    jsError(  "%s:, unknown error code %d, \n", caller, ret );
    break;
  }
  }
  return ret;
}

int getI2cFromDevice( IOEventFlags device  ) {
  switch(device) {
  case EV_I2C1: return I2C_NUM_0;
  case EV_I2C2: return I2C_NUM_1;
  default: return -1;
  }
}

/** Set-up I2C master for ESP32, default pins are SCL:21, SDA:22. Only device I2C1 is supported
 *  and only master mode. */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *info) {
  int i2c_master_port = getI2cFromDevice(device);
  if (i2c_master_port == -1) {
    jsError("Only I2C1 and I2C2 supported"); 
    return;
  }
  Pin scl;
  Pin sda;
  if ( i2c_master_port == I2C_NUM_0 ) {
    scl = info->pinSCL != PIN_UNDEFINED ? info->pinSCL : 21;
    sda = info->pinSDA != PIN_UNDEFINED ? info->pinSDA : 22;
  }
  // Unsure on what to default these pins to?
  if ( i2c_master_port == I2C_NUM_1 ) {
    scl = info->pinSCL != PIN_UNDEFINED ? info->pinSCL : 16;
    sda = info->pinSDA != PIN_UNDEFINED ? info->pinSDA : 17;
  }  

  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = pinToESP32Pin(sda);
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = pinToESP32Pin(scl);
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = info->bitrate;
  esp_err_t err=i2c_param_config(i2c_master_port, &conf);
  if ( err == ESP_ERR_INVALID_ARG ) {
    jsError("jshI2CSetup: Invalid arguments"); 
  return;
  }
  err=i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
  if ( err == ESP_OK ) {
	jsWarn("jshI2CSetup: driver installed, sda: %d sdl: %d freq: %d, \n", sda, scl, info->bitrate);
  } else {
    checkError("jshI2CSetup",err); 
  }
}

void jshI2CWrite(IOEventFlags device,
  unsigned char address,
  int nBytes,
  const unsigned char *data,
  bool sendStop) {
  
  int i2c_master_port = getI2cFromDevice(device);
  if (i2c_master_port == -1) {
    jsError("Only I2C1 and I2C2 supported"); 
    return;
  }
  esp_err_t ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  ret=i2c_master_start(cmd);
  ret=i2c_master_write_byte(cmd, address << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
  ret=i2c_master_write(cmd, data, nBytes, ACK_CHECK_EN);  
  if ( sendStop ) ret=i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(i2c_master_port, cmd, 1000 / portTICK_RATE_MS); // 1000 seems very large for ticks_to_wait???
  i2c_cmd_link_delete(cmd);
  checkError(  "jshI2CWrite", ret);
}

void jshI2CRead(IOEventFlags device,
  unsigned char address,
  int nBytes,
  unsigned char *data,
  bool sendStop) {
  
  if (nBytes <= 0) {
    return;
  }
  int i2c_master_port = getI2cFromDevice(device);
  if (i2c_master_port == -1) {
  jsError("Only I2C1 and I2C2 supported"); 
  return;
  }  
  esp_err_t ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  ret=i2c_master_start(cmd);
  ret=i2c_master_write_byte(cmd, ( i2c_master_port << 1 ) | I2C_MASTER_READ, ACK_CHECK_EN);
  if (nBytes > 1) {
    ret=i2c_master_read(cmd, data, nBytes - 1, ACK_VAL);
  }
  ret=i2c_master_read_byte(cmd, data + nBytes - 1, NACK_VAL);
  if ( sendStop ) ret=i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(i2c_master_port, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  checkError(  "jshI2CRead", ret);  
}
