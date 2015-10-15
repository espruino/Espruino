#ifndef __I2C_MASTER_H__
#define __I2C_MASTER_H__

void i2c_master_gpio_init(uint8 sclPin, uint8 sdaPin, uint32 bitrate);
void i2c_master_init(void);

void i2c_master_stop(void);
void i2c_master_start(void);
void i2c_master_setAck(uint8 level);
uint8 i2c_master_getAck(void);
uint8 i2c_master_readByte(void);
void i2c_master_writeByte(uint8 wrdata);

bool i2c_master_checkAck(void);
void i2c_master_send_ack(void);
void i2c_master_send_nack(void);

#endif
