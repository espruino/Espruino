/*
The MIT License (MIT)
Copyright (c) 2020 Rohm Semiconductor

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __KX126_REGISTERS_H__
#define __KX126_REGISTERS_H__
/* registers */
#define KX126_MAN_ID 0x00
#define KX126_PART_ID 0x01
// x - hp filter output
#define KX126_XHP_L 0x02
#define KX126_XHP_H 0x03
// y - hp filter output
#define KX126_YHP_L 0x04
#define KX126_YHP_H 0x05
// z - hpfilteroutput
#define KX126_ZHP_L 0x06
#define KX126_ZHP_H 0x07
// output register x
#define KX126_XOUT_L 0x08
#define KX126_XOUT_H 0x09
// output register y
#define KX126_YOUT_L 0x0A
#define KX126_YOUT_H 0x0B
// output register z
#define KX126_ZOUT_L 0x0C
#define KX126_ZOUT_H 0x0D
// 16bit pedometer step counter register
#define KX126_PED_STP_L 0x0E
#define KX126_PED_STP_H 0x0F
// Command Test Response
#define KX126_COTR 0x10
// This register can be used for supplier recognition
#define KX126_WHO_AM_I 0x11
// This registers report current position data that is updated at the user-defined ODR frequency determined by OTP<1:0> in CNTL3
#define KX126_TSCP 0x12
// This register report previous and current position data that is updated at the user-defined ODR frequency determined by OTP<1:0> in CNTL3
#define KX126_TSPP 0x13
// This register contains 2 step counter interrupts and contains the tap/double tap axis specific interrupts. Data is updated at the ODR settings determined by OTDT<2:0> in CNTL3.
#define KX126_INS1 0x14
// This Register tells which function caused an interrupt.
#define KX126_INS2 0x15
// This register reports the axis and direction of detected motion and wake-up + back to sleep interrupts
#define KX126_INS3 0x16
// Status register
#define KX126_STAT 0x17
#define KX126_INT_REL 0x19
// Control register 1. Read/write control register that controls the main feature set.
#define KX126_CNTL1 0x1A
// Control settings 2. Read/write control register that primarily controls tilt position state enabling.
#define KX126_CNTL2 0x1B
// Control settings 3. Read/write control register that provides more feature set control.
#define KX126_CNTL3 0x1C
// Control settings 4
#define KX126_CNTL4 0x1D
// Control settings 5
#define KX126_CNTL5 0x1E
// This register is responsible for configuring ODR (output data rate) and filter settings
#define KX126_ODCNTL 0x1F
// Interrupt control 1. This register controls the settings for the physical interrupt pin INT1
#define KX126_INC1 0x20
// Interrupt control 2. This register controls which axis and direction of detected motion can cause an interrupt.
#define KX126_INC2 0x21
// Interrupt control 3. This register controls which axis and direction of tap/double tap can cause an interrupt.
#define KX126_INC3 0x22
// Interrupt control 4. This register controls routing of an interrupt reporting to physical interrupt pin INT1
#define KX126_INC4 0x23
// Interrupt control 5. This register controls the settings for the physical interrupt pin INT2.
#define KX126_INC5 0x24
// Interrupt control 6. This register controls routing of interrupt reporting to physical interrupt pin INT2
#define KX126_INC6 0x25
// Interrupt control 7This register controls routing of interrupt reporting to physical interrupt pins INT1 and INT2
#define KX126_INC7 0x26
#define KX126_TILT_TIMER 0x27
// Tap/Double Tap report control. This register is responsible for enableing/disabling reporting of Tap/Double Tap. Reset applied for any write to TDTRC with TDTE enabled
#define KX126_TDTRC 0x28
#define KX126_TDTC 0x29
#define KX126_TTH 0x2A
#define KX126_TTL 0x2B
#define KX126_FTD 0x2C
#define KX126_STD 0x2D
#define KX126_TLT 0x2E
#define KX126_TWS 0x2F
#define KX126_FFTH 0x30
#define KX126_FFC 0x31
// Freefall interrupt control.
#define KX126_FFCNTL 0x32
#define KX126_TILT_ANGLE_LL 0x34
#define KX126_TILT_ANGLE_HL 0x35
#define KX126_HYST_SET 0x36
#define KX126_LP_CNTL 0x37
#define KX126_WUFTH 0x3C
// Additional threshold bits for WUF and BTS. Resolution is 8g/2^11=3.9mg/cnt    Assuming the engine gets 12bit signed data (ADC 10bits + 2bits for 16x Oversampling). Reset applied for any write to BTSWUFTH with WUFE or BTSE enabled
#define KX126_BTSWUFTH 0x3D
#define KX126_BTSTH 0x3E
#define KX126_BTSC 0x3F
#define KX126_WUFC 0x40
#define KX126_PED_STPWM_L 0x41
#define KX126_PED_STPWM_H 0x42
// Pedometer control register 1
#define KX126_PED_CNTL1 0x43
// Pedometer control register 2.
#define KX126_PED_CNTL2 0x44
// Pedometer control register 3
#define KX126_PED_CNTL3 0x45
// Pedometer control register 4
#define KX126_PED_CNTL4 0x46
#define KX126_PED_CNTL5 0x47
#define KX126_PED_CNTL6 0x48
#define KX126_PED_CNTL7 0x49
#define KX126_PED_CNTL8 0x4A
#define KX126_PED_CNTL9 0x4B
#define KX126_PED_CNTL10 0x4C
// Self test initiation
#define KX126_SELF_TEST 0x4D
#define KX126_BUF_CNTL1 0x5A
// Read/write control register that controls sample buffer operation
#define KX126_BUF_CNTL2 0x5B
#define KX126_BUF_STATUS_1 0x5C
// This register reports the status of the sample buffer trigger function
#define KX126_BUF_STATUS_2 0x5D
#define KX126_BUF_CLEAR 0x5E
#define KX126_BUF_READ 0x5F
/* registers bits */
// before set
#define KX126_COTR_DCSTR_BEFORE (0x55 << 0)
// after set
#define KX126_COTR_DCSTR_AFTER (0xAA << 0)
// WAI value for KX126
#define KX126_WHO_AM_I_WAI_ID (0x38 << 0)
// LE - Left state X' negative (x-)
#define KX126_TSCP_LE (0x01 << 5)
// RI - Right state X' positive (x+)
#define KX126_TSCP_RI (0x01 << 4)
// DO - Down state Y' negative (y-)
#define KX126_TSCP_DO (0x01 << 3)
// UP - Up state Y' positive (y+)
#define KX126_TSCP_UP (0x01 << 2)
// FD - Face Down state Z negative (z-)
#define KX126_TSCP_FD (0x01 << 1)
// FU - Face Up Z positive (z+)
#define KX126_TSCP_FU (0x01 << 0)
// LE - Left state X' negative (x-)
#define KX126_TSPP_LE (0x01 << 5)
// RI - Right state X' positive (x+)
#define KX126_TSPP_RI (0x01 << 4)
// DO - Down state Y' negative (y-)
#define KX126_TSPP_DO (0x01 << 3)
// UP - Up state Y' positive (y+)
#define KX126_TSPP_UP (0x01 << 2)
// FD - Face Down state Z negative (z-)
#define KX126_TSPP_FD (0x01 << 1)
// FU - Face Up Z positive (z+)
#define KX126_TSPP_FU (0x01 << 0)
// STPOVI - Step counter Overflow interrupt
#define KX126_INS1_STPOVI (0x01 << 7)
// STPWMI - Step counter Watermark Interrupt
#define KX126_INS1_STPWMI (0x01 << 6)
// TLE - X' negative (x-)
#define KX126_INS1_TLE (0x01 << 5)
// TRI - X' positive (x+)
#define KX126_INS1_TRI (0x01 << 4)
// TDO - Y' negative (y-)
#define KX126_INS1_TDO (0x01 << 3)
// TUP - Y' positive (y+)
#define KX126_INS1_TUP (0x01 << 2)
// TFD - Z  negative (z-)
#define KX126_INS1_TFD (0x01 << 1)
// TFU - Z  positive (z+)
#define KX126_INS1_TFU (0x01 << 0)
// FFS - Freefall, 0=not in freefall state, 1=freefall is detected. FFS is released to 0 when INL is read.
#define KX126_INS2_FFS (0x01 << 7)
// BFI - indicates buffer full interrupt.  Automatically cleared when buffer is read.
#define KX126_INS2_BFI (0x01 << 6)
// WMI - Watermark interrupt, bit is set to one when FIFO has filled up to the value stored in the sample bits.This bit is automatically cleared when FIFO/FILO is read and the content returns to a value below the value stored in the sample bits.
#define KX126_INS2_WMI (0x01 << 5)
// DRDY - indicates that new acceleration data((00h,06h) to (00h,0Bh)) is available.  This bit is cleared when acceleration data is read or the interrupt release register (INL (00h,17h)) is read. 0= new acceleration data not available, 1= new acceleration data available
#define KX126_INS2_DRDY (0x01 << 4)
// 00 = no tap
#define KX126_INS2_TDTS_NOTAP (0x00 << 2)
// 01 = single tap
#define KX126_INS2_TDTS_SINGLE (0x01 << 2)
// 10 = double tap
#define KX126_INS2_TDTS_DOUBLE (0x02 << 2)
// 11 = does not exist
#define KX126_INS2_TDTS_NA (0x03 << 2)
// STPINCI - Step counter increment interrupt
#define KX126_INS2_STPINCI (0x01 << 1)
// TPS - Tilt Position status.  0=state not changed, 1=state changed.  TPS is released to 0 when INL is read.
#define KX126_INS2_TPS (0x01 << 0)
// WUFS - Wake up, This bit is cleared when the interrupt source latch register (INL (00h,1Ah)) is read. 1=Motion has activated the interrupt,  0= No motion
#define KX126_INS3_WUFS (0x01 << 7)
// BTS - Back to sleep interrupt
#define KX126_INS3_BTS (0x01 << 6)
// XNWU - X' negative (x-)
#define KX126_INS3_XNWU (0x01 << 5)
// XPWU - X' positive (x+)
#define KX126_INS3_XPWU (0x01 << 4)
// YNWU - Y' negative (y-)
#define KX126_INS3_YNWU (0x01 << 3)
// YPWU - Y' positive (y+)
#define KX126_INS3_YPWU (0x01 << 2)
// ZNWU - Z  negative (z-)
#define KX126_INS3_ZNWU (0x01 << 1)
// ZPWU - Z  positive (z+)
#define KX126_INS3_ZPWU (0x01 << 0)
// INT - reports the combined (OR) interrupt information of all features.  0= no interrupt event, 1= interrupt event has occurred.  When BFI and WMI in INS2 are 0, the INT bit is released to 0 when INL is read.  If WMI or BFI is 1, INT bit remains at 1 until they are cleared
#define KX126_STAT_INT (0x01 << 4)
// wake - reports the wake or sleep state, 0=sleep , 1=wake
#define KX126_STAT_WAKE (0x01 << 0)
// PC1 - controls the operating mode.  0= stand-by mode,  1= operating mode.
#define KX126_CNTL1_PC1 (0x01 << 7)
// RES - enables full power mode
#define KX126_CNTL1_RES (0x01 << 6)
// DRDYE - enables the reporting of the availability of new acceleration data ((00h,06h) to (00h,0Bh)) on the interrupt pin. 0= availability of new acceleration data not reflected on interrupt pin, 1= availability of new acceleration data reflected on interrupt pin.
#define KX126_CNTL1_DRDYE (0x01 << 5)
// 00 = +/- 2g
#define KX126_CNTL1_GSEL_2G (0x00 << 3)
// 01 = +/- 4g
#define KX126_CNTL1_GSEL_4G (0x01 << 3)
// 1X = +/- 8g
#define KX126_CNTL1_GSEL_8G (0x02 << 3)
// 1X = +/- 8g
#define KX126_CNTL1_GSEL_8G_2 (0x03 << 3)
// TDTE - enables the Tap / Double Tap function. 0 = disabled, 1 = enabled.
#define KX126_CNTL1_TDTE (0x01 << 2)
// PDE - enables Pedometer function
#define KX126_CNTL1_PDE (0x01 << 1)
// TPE - enables the Tilt Position function. 0=disabled, 1 = enabled
#define KX126_CNTL1_TPE (0x01 << 0)
// SRST - Soft Reset performs the POR routine. 0= no action. 1= start POR routine.
#define KX126_CNTL2_SRST (0x01 << 7)
// COTC - Command test control. 0= no action, 1 sets AAh to STR @ 0Ch register, when STR register is read COTC is cleared and STR=55h.
#define KX126_CNTL2_COTC (0x01 << 6)
// LEM - Tilt Left state mask
#define KX126_CNTL2_LEM (0x01 << 5)
// RIM - Tilt Right state mask
#define KX126_CNTL2_RIM (0x01 << 4)
// DOM - Tilt Down state mask
#define KX126_CNTL2_DOM (0x01 << 3)
// UPM - Tilt Up state mask
#define KX126_CNTL2_UPM (0x01 << 2)
// FDM - Tilt Face Down state mask
#define KX126_CNTL2_FDM (0x01 << 1)
// FUM - Tilt Face Up state mask
#define KX126_CNTL2_FUM (0x01 << 0)
// 1.5Hz
#define KX126_CNTL3_OTP_1P563 (0x00 << 6)
// 6.25Hz
#define KX126_CNTL3_OTP_6P25 (0x01 << 6)
// 12.5Hz
#define KX126_CNTL3_OTP_12P5 (0x02 << 6)
// 50Hz
#define KX126_CNTL3_OTP_50 (0x03 << 6)
// 50Hz
#define KX126_CNTL3_OTDT_50 (0x00 << 3)
// 100Hz
#define KX126_CNTL3_OTDT_100 (0x01 << 3)
// 200Hz
#define KX126_CNTL3_OTDT_200 (0x02 << 3)
// 400Hz
#define KX126_CNTL3_OTDT_400 (0x03 << 3)
// 12.5Hz
#define KX126_CNTL3_OTDT_12P5 (0x04 << 3)
// 25Hz
#define KX126_CNTL3_OTDT_25 (0x05 << 3)
// 800Hz
#define KX126_CNTL3_OTDT_800 (0x06 << 3)
// 1600Hz
#define KX126_CNTL3_OTDT_1600 (0x07 << 3)
// 0.78Hz
#define KX126_CNTL3_OWUF_0P781 (0x00 << 0)
// 1.563Hz
#define KX126_CNTL3_OWUF_1P563 (0x01 << 0)
// 3.125Hz
#define KX126_CNTL3_OWUF_3P125 (0x02 << 0)
// 6.25Hz
#define KX126_CNTL3_OWUF_6P25 (0x03 << 0)
// 12.5Hz
#define KX126_CNTL3_OWUF_12P5 (0x04 << 0)
// 25Hz
#define KX126_CNTL3_OWUF_25 (0x05 << 0)
// 50Hz
#define KX126_CNTL3_OWUF_50 (0x06 << 0)
// 100Hz
#define KX126_CNTL3_OWUF_100 (0x07 << 0)
// c_mode - Define debounce counter clear mode 0: clear 1: decrement
#define KX126_CNTL4_C_MODE (0x01 << 7)
// th_mode: 0: absolute threshold 1: relative threshold (default)
#define KX126_CNTL4_TH_MODE (0x01 << 6)
// WUFE - enables the Wake Up (motion detect) function that will detect a general motion event. 0= disabled, 1= enabled.
#define KX126_CNTL4_WUFE (0x01 << 5)
// BTSE - enables the Back to sleep function
#define KX126_CNTL4_BTSE (0x01 << 4)
// HPE - High-pass enable
#define KX126_CNTL4_HPE (0x01 << 3)
// 000 = 0.78125Hz
#define KX126_CNTL4_OBTS_0P781 (0x00 << 0)
// 001 = 1.5625Hz
#define KX126_CNTL4_OBTS_1P563 (0x01 << 0)
// 010 = 3.125Hz
#define KX126_CNTL4_OBTS_3P125 (0x02 << 0)
// 011 = 6.25Hz
#define KX126_CNTL4_OBTS_6P25 (0x03 << 0)
// 100 = 12.5Hz
#define KX126_CNTL4_OBTS_12P5 (0x04 << 0)
// 101 = 25Hz
#define KX126_CNTL4_OBTS_25 (0x05 << 0)
// 110 = 50Hz
#define KX126_CNTL4_OBTS_50 (0x06 << 0)
// 111 = 100Hz
#define KX126_CNTL4_OBTS_100 (0x07 << 0)
// man_wake - manual wake mode overwrite (forces ASIC into wake mode)
#define KX126_CNTL5_MAN_WAKE (0x01 << 1)
// man_sleep - manual sleep mode overwrite (forces ASIC into sleep mode)
#define KX126_CNTL5_MAN_SLEEP (0x01 << 0)
// IIR_BYPASS - IIR filter bypass mode for debugging averaging filter.
#define KX126_ODCNTL_IIR_BYPASS (0x01 << 7)
// LPRO - Low pass filter roll off control, 0=ODR/9, 1=ODR/2
#define KX126_ODCNTL_LPRO (0x01 << 6)
// 0000 = 12.5Hz Low power mode available
#define KX126_ODCNTL_OSA_12P5 (0x00 << 0)
// 0001 = 25Hz  Low power mode available
#define KX126_ODCNTL_OSA_25 (0x01 << 0)
// 0010 = 50Hz  Low power mode available
#define KX126_ODCNTL_OSA_50 (0x02 << 0)
// 0011 = 100Hz  Low power mode available
#define KX126_ODCNTL_OSA_100 (0x03 << 0)
// 0100 = 200Hz  Low power mode available
#define KX126_ODCNTL_OSA_200 (0x04 << 0)
// 0101 = 400Hz
#define KX126_ODCNTL_OSA_400 (0x05 << 0)
// 0110 = 800Hz
#define KX126_ODCNTL_OSA_800 (0x06 << 0)
// 0111 = 1600Hz
#define KX126_ODCNTL_OSA_1600 (0x07 << 0)
// 1000 = 0.781Hz  Low power mode available
#define KX126_ODCNTL_OSA_0P781 (0x08 << 0)
// 1001 = 1.563Hz  Low power mode available
#define KX126_ODCNTL_OSA_1P563 (0x09 << 0)
// 1010 = 3.125Hz  Low power mode available
#define KX126_ODCNTL_OSA_3P125 (0x0A << 0)
// 1011 = 6.25Hz  Low power mode available
#define KX126_ODCNTL_OSA_6P25 (0x0B << 0)
// 1100 = 3200Hz
#define KX126_ODCNTL_OSA_3200 (0x0C << 0)
// 1101 = 6400Hz
#define KX126_ODCNTL_OSA_6400 (0x0D << 0)
// 1110 = 12800Hz
#define KX126_ODCNTL_OSA_12800 (0x0E << 0)
// 1111 = 25600Hz
#define KX126_ODCNTL_OSA_25600 (0x0F << 0)
// 0 : default width - 50us(10us when ODR>1600Hz)
#define KX126_INC1_PW1_50US_10US (0x00 << 6)
// width 1*ODR period
#define KX126_INC1_PW1_1XODR (0x01 << 6)
// width 2*ODR period
#define KX126_INC1_PW1_2XODR (0x02 << 6)
// width 4*ODR period
#define KX126_INC1_PW1_4XODR (0x03 << 6)
// IEN1 - Enable/disable physical interrupt pin 1, 0=disable, 1=enable.
#define KX126_INC1_IEN1 (0x01 << 5)
// IEA1 - Interrupt active level control for interrupt pin 1, 0=active low, 1=active high.
#define KX126_INC1_IEA1 (0x01 << 4)
// IEL1 - Interrupt latch control for interrupt pin 1, 0=latched, 1=one pulse
#define KX126_INC1_IEL1 (0x01 << 3)
// STPOL - ST polarity, This bit is ignored when STNULL is set.
#define KX126_INC1_STPOL (0x01 << 1)
// SPI3E - 3-wired SPI interface, 0=disable, 1=enable.
#define KX126_INC1_SPI3E (0x01 << 0)
// 0=Or combination of selected directions
#define KX126_INC2_AOI_OR (0x00 << 6)
// 1=And combination of selected axes
#define KX126_INC2_AOI_AND (0x01 << 6)
// XNWUE - x negative (x-) mask for WUF, 0=disable, 1=enable.
#define KX126_INC2_XNWUE (0x01 << 5)
// XPWUE - x positive (x+) mask for WUF, 0=disable, 1=enable.
#define KX126_INC2_XPWUE (0x01 << 4)
// YNWUE - y negative (y-) mask for WUF, 0=disable, 1=enable.
#define KX126_INC2_YNWUE (0x01 << 3)
// YPWUE - y positive (y+) mask for WUF, 0=disable, 1=enable.
#define KX126_INC2_YPWUE (0x01 << 2)
// ZNWUE - z negative (z-) mask for WUF, 0=disable, 1=enable.
#define KX126_INC2_ZNWUE (0x01 << 1)
// ZPWUE - z positive (z+) mask for WUF, 0=disable, 1=enable.
#define KX126_INC2_ZPWUE (0x01 << 0)
// enables/disables alternate tap masking scheme
#define KX126_INC3_TMEM (0x01 << 6)
// x negative (x-): 0 = disabled, 1 = enabled
#define KX126_INC3_TLEM (0x01 << 5)
// x positive (x+): 0 = disabled, 1 = enabled
#define KX126_INC3_TRIM (0x01 << 4)
// y negative (y-): 0 = disabled, 1 = enabled
#define KX126_INC3_TDOM (0x01 << 3)
// y positive (y+): 0 = disabled, 1 = enabled
#define KX126_INC3_TUPM (0x01 << 2)
// z negative (z-): 0 = disabled, 1 = enabled
#define KX126_INC3_TFDM (0x01 << 1)
// z positive (z+): 0 = disabled, 1 = enabled
#define KX126_INC3_TFUM (0x01 << 0)
// FFI1 - Freefall interrupt reported on physical interrupt pin 1, 0=disable, 1=enable (and IEN1=1).
#define KX126_INC4_FFI1 (0x01 << 7)
// BFI1 - Buffer full interrupt reported on physical interrupt pin 1, 0=disable, 1=enable (and IEN1=1).
#define KX126_INC4_BFI1 (0x01 << 6)
// WMI1 - Watermark interrupt reported on physical interrupt pin 1, 0=disable, 1=enable (and IEN1=1).
#define KX126_INC4_WMI1 (0x01 << 5)
// DRDYI1 - Data ready interrupt reported on physical interrupt pin 1, 0=disable, 1=enable (and IEN1=1).
#define KX126_INC4_DRDYI1 (0x01 << 4)
// BTSI1 - Back to sleep interrupt reported in interrupt pin 1
#define KX126_INC4_BTSI1 (0x01 << 3)
// TDTI1 - Tap/Double Tap interrupt reported on physical interrupt pin 1, 0=disable, 1=enable (and IEN1=1).
#define KX126_INC4_TDTI1 (0x01 << 2)
// WUFI1 - Wake Up (motion detect) interrupt reported pn physical interrupt pin 1, 0=disable, 1=enable (and IEN1=1).
#define KX126_INC4_WUFI1 (0x01 << 1)
// TPI1 - Tilt position interrupt reported on physical interrupt pin 1, 0=disable, 1=enable (and IEN1=1).
#define KX126_INC4_TPI1 (0x01 << 0)
// 0 : default width - 50us(10us when ODR>1600Hz)
#define KX126_INC5_PW2_50US_10US (0x00 << 6)
// width 1*ODR period
#define KX126_INC5_PW2_1XODR (0x01 << 6)
// width 2*ODR period
#define KX126_INC5_PW2_2XODR (0x02 << 6)
// width 4*ODR period
#define KX126_INC5_PW2_4XODR (0x03 << 6)
// IEN2 - Enable/disable physical interrupt pin 2, 0=disable, 1=enable.
#define KX126_INC5_IEN2 (0x01 << 5)
// IEA2 - Interrupt active level control for interrupt pin 2, 0=active low, 1=active high.
#define KX126_INC5_IEA2 (0x01 << 4)
// IEL2 - Interrupt latch control for interrupt pin 2, 0=latched, 1=one pulse
#define KX126_INC5_IEL2 (0x01 << 3)
// ACLR2 - Auto interrupt clear(same as INL) for the following event, only available in pulse interrupt mode, 0=disable, 1=enable.
#define KX126_INC5_ACLR2 (0x01 << 1)
// ACLR1 - Auto interrupt clear(same as INL) for the following event, only available in pulse interrupt mode, 0=disable, 1=enable.
#define KX126_INC5_ACLR1 (0x01 << 0)
// FFI2 - Freefall interrupt reported on physical interrupt pin 2, 0=disable, 1=enable (and IEN2=1).
#define KX126_INC6_FFI2 (0x01 << 7)
// BFI2 - Buffer full interrupt reported on physical interrupt pin 2, 0=disable, 1=enable (and IEN2=1).
#define KX126_INC6_BFI2 (0x01 << 6)
// WMI2 - Watermark interrupt reported on physical interrupt pin 2, 0=disable, 1=enable (and IEN2=1).
#define KX126_INC6_WMI2 (0x01 << 5)
// DRDYI2 - Data ready interrupt reported on physical interrupt pin 2, 0=disable, 1=enable (and IEN2=1).
#define KX126_INC6_DRDYI2 (0x01 << 4)
// BTSI2 - Back to sleep interrupt reported in interrupt pin 2
#define KX126_INC6_BTSI2 (0x01 << 3)
// TDTI2 - Tap/Double Tap interrupt reported on physical interrupt pin 2, 0=disable, 1=enable (and IEN2=1).
#define KX126_INC6_TDTI2 (0x01 << 2)
// WUFI2 - Wake Up (motion detect) interrupt reported pn physical interrupt pin 2, 0=disable, 1=enable (and IEN2=1).
#define KX126_INC6_WUFI2 (0x01 << 1)
// TPI2 - Tilt position interrupt reported on physical interrupt pin 2, 0=disable, 1=enable (and IEN2=1).
#define KX126_INC6_TPI2 (0x01 << 0)
// STPOVI2 - Step counter overflow interrupt on interrupt pin 2
#define KX126_INC7_STPOVI2 (0x01 << 6)
// STPWMI2 - Step counter watermark interrupt on interrupt pin 2
#define KX126_INC7_STPWMI2 (0x01 << 5)
// STPINCI2 - Step counter increment interrupt on interrupt pin 2
#define KX126_INC7_STPINCI2 (0x01 << 4)
// STPOVI1 - Step counter overflow interrupt on interrupt pin 1
#define KX126_INC7_STPOVI1 (0x01 << 2)
// STPWMI1 - Step counter watermark interrupt on interrupt pin 1
#define KX126_INC7_STPWMI1 (0x01 << 1)
// STPINCI1 - Step counter increment interrupt on interrupt pin 1
#define KX126_INC7_STPINCI1 (0x01 << 0)
// DTRE - Double tap report Enable.  When DTRE is set to 1, update INS1 and DTDS in INS2 with double tap events. When DTRE is set to 0, do not update INS1 or DTDS if double tap occurs.
#define KX126_TDTRC_DTRE (0x01 << 1)
// STRE - Single tap report Enable.  When STRE is set to 1, update INS1 and DTDS in INS2 single tap events. When DTRE is set to 0, do not update INS1 or DTDS if single tap occurs.
#define KX126_TDTRC_STRE (0x01 << 0)
// FFIE - Freefall engine enable, 0=disabled, 1=enabled.
#define KX126_FFCNTL_FFIE (0x01 << 7)
// ULMODE - Interrupt latch/un-latch control, 0=latched, 1=unlatched.
#define KX126_FFCNTL_ULMODE (0x01 << 6)
// DCRM - Debounce methodology control, 0=count up/down, 1=count up/reset.
#define KX126_FFCNTL_DCRM (0x01 << 3)
// 000 = 0.781Hz
#define KX126_FFCNTL_OFFI_0P781 (0x00 << 0)
// 001 = 1.563Hz
#define KX126_FFCNTL_OFFI_1P563 (0x01 << 0)
// 010 = 3.125Hz
#define KX126_FFCNTL_OFFI_3P125 (0x02 << 0)
// 011 = 6.25Hz
#define KX126_FFCNTL_OFFI_6P25 (0x03 << 0)
// 100 = 12.5Hz
#define KX126_FFCNTL_OFFI_12P5 (0x04 << 0)
// 101 = 25Hz
#define KX126_FFCNTL_OFFI_25 (0x05 << 0)
// 110 = 50Hz
#define KX126_FFCNTL_OFFI_50 (0x06 << 0)
// 111 = 100Hz
#define KX126_FFCNTL_OFFI_100 (0x07 << 0)
// No Averaging
#define KX126_LP_CNTL_AVC_NO_AVG (0x00 << 4)
// 2 Samples Averaged
#define KX126_LP_CNTL_AVC_2_SAMPLE_AVG (0x01 << 4)
// 4 Samples Averaged
#define KX126_LP_CNTL_AVC_4_SAMPLE_AVG (0x02 << 4)
// 8 Samples Averaged
#define KX126_LP_CNTL_AVC_8_SAMPLE_AVG (0x03 << 4)
// 16 Samples Averaged (default)
#define KX126_LP_CNTL_AVC_16_SAMPLE_AVG (0x04 << 4)
// 32 Samples Averaged
#define KX126_LP_CNTL_AVC_32_SAMPLE_AVG (0x05 << 4)
// 64 Samples Averaged
#define KX126_LP_CNTL_AVC_64_SAMPLE_AVG (0x06 << 4)
// 128 Samples Averaged
#define KX126_LP_CNTL_AVC_128_SAMPLE_AVG (0x07 << 4)
// No threshold count for start counting
#define KX126_PED_CNTL1_STP_TH_NO_STEP (0x00 << 4)
// 2 step threshold for start counting
#define KX126_PED_CNTL1_STP_TH_STEP_2 (0x01 << 4)
// 4 step threshold for start counting
#define KX126_PED_CNTL1_STP_TH_STEP_4 (0x02 << 4)
// 6 step threshold for start counting
#define KX126_PED_CNTL1_STP_TH_STEP_6 (0x03 << 4)
// 8 step threshold for start counting
#define KX126_PED_CNTL1_STP_TH_STEP_8 (0x04 << 4)
// 10 step threshold for start counting
#define KX126_PED_CNTL1_STP_TH_STEP_10 (0x05 << 4)
// 12 step threshold for start counting
#define KX126_PED_CNTL1_STP_TH_STEP_12 (0x06 << 4)
// 14 step threshold for start counting
#define KX126_PED_CNTL1_STP_TH_STEP_14 (0x07 << 4)
// Pedometer ODR 50Hz
#define KX126_PED_CNTL2_PED_ODR_50 (0x06 << 0)
// Pedometer ODR 100Hz
#define KX126_PED_CNTL2_PED_ODR_100 (0x0C << 0)
// Scaling factor 1
#define KX126_PED_CNTL3_FCA_1 (0x00 << 0)
// Scaling factor 2
#define KX126_PED_CNTL3_FCA_2 (0x01 << 0)
// Scaling factor 4
#define KX126_PED_CNTL3_FCA_4 (0x02 << 0)
// Scaling factor 8
#define KX126_PED_CNTL3_FCA_8 (0x03 << 0)
// Scaling factor 16
#define KX126_PED_CNTL3_FCA_16 (0x04 << 0)
// Scaling factor 32
#define KX126_PED_CNTL3_FCA_32 (0x05 << 0)
// Scaling factor 64
#define KX126_PED_CNTL3_FCA_64 (0x06 << 0)
// Scaling factor 128
#define KX126_PED_CNTL3_FCA_128 (0x07 << 0)
// controls activation of the sample buffer
#define KX126_BUF_CNTL2_BUFE (0x01 << 7)
// determines the resolution of the acceleration data samples collected by the sample
#define KX126_BUF_CNTL2_BRES (0x01 << 6)
// buffer full interrupt enable bit
#define KX126_BUF_CNTL2_BFIE (0x01 << 5)
// The buffer collects 681 sets of 8-bit low resolution values or 339 sets of 16-bit high resolution values and then stops collecting data, collecting new data only when the buffer is not full
#define KX126_BUF_CNTL2_BUF_BM_FIFO (0x00 << 0)
// The buffer holds the last 681 sets of 8-bit low resolution values or 339 sets of 16-bit high resolution values. Once the buffer is full, the oldest data is discarded to make room for newer data.
#define KX126_BUF_CNTL2_BUF_BM_STREAM (0x01 << 0)
// When a trigger event occurs, the buffer holds the last data set of SMP[9:0] samples before the trigger event and then continues to collect data until full. New data is collected only when the buffer is not full.
#define KX126_BUF_CNTL2_BUF_BM_TRIGGER (0x02 << 0)
// The buffer holds the last 681 sets of 8-bit low resolution values or 339 sets of 16-bit high resolution values. Once the buffer is full, the oldest data is discarded to make room for newer data. Reading from the buffer in this mode will return the most recent data first.
#define KX126_BUF_CNTL2_BUF_BM_FILO (0x03 << 0)
// reports the status of the buffers trigger function if this mode has been selected
#define KX126_BUF_STATUS_2_BUF_TRIG (0x01 << 7)
 /*registers bit masks */
#define KX126_COTR_DCSTR_MASK 0xFF

#define KX126_WHO_AM_I_WAI_MASK 0xFF
#define KX126_INS1_TP_MASK 0x3F
// TDTS(1,0) - status of tap/double tap, bit is released when interrupt latch release register (INL (00h,17h)) is read.
#define KX126_INS2_TDTS_MASK 0x0C
#define KX126_INS3_WU_MASK 0x3F
// Gsel - Selectable g-range bits
#define KX126_CNTL1_GSEL_MASK 0x18
#define KX126_CNTL2_TP_MASK 0x3F
// sets the output data rate for the Tilt Position function
#define KX126_CNTL3_OTP_MASK 0xC0
// sets the output data rate for the Directional TapTM function
#define KX126_CNTL3_OTDT_MASK 0x38
// sets the output data rate for the general motion detection function and the high-pass filtered outputs
#define KX126_CNTL3_OWUF_MASK 0x07
// OBTS<2:0> - Back to sleep function output data rate
#define KX126_CNTL4_OBTS_MASK 0x07
// OSA<3:0> - Acceleration Output data rate.* Low power mode available, all other data rates will default to full power mode.
#define KX126_ODCNTL_OSA_MASK 0x0F
// PW1 - Pulse interrupt width on INT1
#define KX126_INC1_PW1_MASK 0xC0
// AOI - And-Or configuration, 0=Or combination of selected directions, 1=And combination of selected axes
#define KX126_INC2_AOI_MASK 0x40
#define KX126_INC2_WUE_MASK 0x3F
#define KX126_INC3_TM_MASK 0x3F
// PW2 - Pulse interrupt width on INT2
#define KX126_INC5_PW2_MASK 0xC0
// OFFI<2:0> - Freefall function output data rate
#define KX126_FFCNTL_OFFI_MASK 0x07
// Averaging Filter Control
#define KX126_LP_CNTL_AVC_MASK 0x70
#define KX126_BTSWUFTH_BTSTH8_10_MASK 0x70
#define KX126_BTSWUFTH_WUFTH8_10_MASK 0x07
// STP_TH<2:0> ; A threshold for discarding counting if not enough steps coming. Values: 0, 1, ..., 7. -> 0, 1, 2, 4, ..., 15. Reset applied for any write to PED_CNTL1 with PDE enabled
#define KX126_PED_CNTL1_STP_TH_MASK 0x70
#define KX126_PED_CNTL1_MAG_SCALE_MASK 0x0F
#define KX126_PED_CNTL2_HPS_MASK 0x70
// The length of the low-pass filter (as ODR 50 or 100Hz)
#define KX126_PED_CNTL2_PED_ODR_MASK 0x0F
#define KX126_PED_CNTL3_FCB_MASK 0x38
// Scaling factor inside high-pass filter
#define KX126_PED_CNTL3_FCA_MASK 0x07
#define KX126_PED_CNTL4_B_CNT_MASK 0x70
#define KX126_PED_CNTL4_A_H_MASK 0x0F
#define KX126_SELF_TEST_MEMS_TEST_MASK 0xFF
#define KX126_BUF_CNTL2_SMP_TH8_9_MASK 0x0C
// selects the operating mode of the sample buffer
#define KX126_BUF_CNTL2_BUF_BM_MASK 0x03
#define KX126_BUF_STATUS_2_SMP_LEV8_11_MASK 0x0F
#endif

