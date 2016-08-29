/******************************************************************************
 * Copyright 2015 Espressif Systems
 *
 * Description: Assembly routines for the gdbstub
 *
 * License: ESPRESSIF MIT License
 *******************************************************************************/


#include <xtensa/config/specreg.h>
#include <xtensa/config/core-isa.h>
#include <xtensa/corebits.h>

.global gdbstub_savedRegs

	.text
.literal_position

	.text
	.align	4

/*
The savedRegs struct:
	uint32_t pc;
	uint32_t ps;
	uint32_t sar;
	uint32_t vpri;
	uint32_t a0;
	uint32_t a[14]; //a2..a15
	uint32_t litbase;
	uint32_t sr176;
	uint32_t sr208;
	uint32_t a1;
	uint32_t reason;
	uint32_t excvaddr;
*/

	.global gdbstub_save_extra_sfrs_for_exception
	.align 4
//The Xtensa OS HAL does not save all the special function register things. This bit of assembly
//fills the gdbstub_savedRegs struct with them.
gdbstub_save_extra_sfrs_for_exception:
	movi	a2, gdbstub_savedRegs
	rsr		a3, LITBASE
	s32i	a3, a2, 0x4C
	rsr		a3, 176
	s32i	a3, a2, 0x50
	rsr		a3, 208
	s32i	a3, a2, 0x54
	rsr		a3, EXCCAUSE
	s32i	a3, a2, 0x5C
	rsr		a3, EXCVADDR
	s32i	a3, a2, 0x60
	ret
