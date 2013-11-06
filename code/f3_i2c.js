/*
#define ACC_I2C_ADDRESS                      0x32
#define MAG_I2C_ADDRESS                      0x3C

#define LSM303_CTRL_REG1_A       0x20
#define LSM303_CTRL_REG2_A       0x21
#define LSM303_CTRL_REG3_A       0x22
#define LSM303_CTRL_REG4_A       0x23
#define LSM303_CTRL_REG5_A       0x24
#define LSM303_CTRL_REG6_A       0x25 // DLHC only
#define LSM303_HP_FILTER_RESET_A 0x25 // DLH, DLM only
#define LSM303_REFERENCE_A       0x26
#define LSM303_STATUS_REG_A      0x27

#define LSM303_OUT_X_L_A         0x28
#define LSM303_OUT_X_H_A         0x29
#define LSM303_OUT_Y_L_A         0x2A
#define LSM303_OUT_Y_H_A         0x2B
#define LSM303_OUT_Z_L_A         0x2C
#define LSM303_OUT_Z_H_A         0x2D

writeAccReg(LSM303_CTRL_REG1_A, 0x27);

*/

I2C1.setup({scl:B6, sda:B7});var ACC=0x32>>1;

I2C1.writeTo(ACC, [0x20, 0x27])
I2C1.writeTo(ACC, 0x28 | 0x80);I2C1.readFrom(ACC, 6)

peek32(0x40005418) // ISR


#define PERIPH_BASE           ((uint32_t)0x40000000)
#define I2C1_BASE             (APB1PERIPH_BASE + 0x00005400)
#define I2C_Register_ISR                ((uint8_t)0x18)


I2C1.setup({scl:B6, sda:B7});
var ACC=0x32>>1;


I2C1.writeTo(ACC, [0x20, 0x27])
I2C1.writeTo(ACC, 0x28 | 0x80);I2C1.readFrom(ACC, 6)


I2C.prototype.writeAccReg = function(reg,val) {
  this.writeTo(0x32>>1, [reg,val]);
}
I2C.prototype.readAccReg = function(reg,count) {
  this.writeTo(0x32>>1, reg | 0x80);
  return this.readFrom(0x32>>1, count);
}
I2C.prototype.readAcc = function(reg,count) {
  var d = this.readAccReg(0x28, 6);
  // reconstruct 16 bit data
  var a = [d[0] | (d[1]<<8), d[2] | (d[3]<<8), d[4] | (d[5]<<8)];
  // deal with sign bit
  if (a[0]>=32767) a[0]-=65536;
  if (a[1]>=32767) a[1]-=65536;
  if (a[2]>=32767) a[2]-=65536;
  return a;
}
I2C1.writeAccReg(0x20, 0x27); // turn on
I2C1.readAccReg(0x28, 6); // return array of data

I2C1.setup({scl:B6, sda:B7}); // Setup I2C
I2C1.writeAccReg(0x20, 0x27); // turn Accelerometer on
I2C1.readAcc() // Return acceleration data