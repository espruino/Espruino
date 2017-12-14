/**
 * Include file for the basic libraries  -  Arduino DUE with SAM3X8E
 *
 *  - It defines __SAM3X8E__, MAIN_OSC_FREQ (main oscillator frequency),
 *    MASTER_CLK_FREQ (the master clock frequency).
 *
 *  - It includes the peripheral specific libsam and sam.h
 *
 * @file due_sam3x.h
 * @author stfwi
 */
#ifndef __ARDUINO_DUE_X_H__INCLUDED__
#define	__ARDUINO_DUE_X_H__INCLUDED__
#ifdef	__cplusplus
extern "C" {
#endif

#define __SAM3X8E__

#ifdef __SAM3X8E__
#define MAIN_OSC_FREQ (12000000)
#define MASTER_CLK_FREQ (84000000)
#endif

#include "../sam/libsam/chip.h"
#include "../sam/CMSIS/Device/ATMEL/sam.h"


extern void init_controller(void);

#ifdef	__cplusplus
}
#endif
#endif




