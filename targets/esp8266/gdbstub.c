/******************************************************************************
 * Copyright 2015 Espressif Systems
 *
 * Description: A stub to make the ESP8266 debuggable by GDB over the serial
 * port.
 *
 * License: ESPRESSIF MIT License
 *******************************************************************************/

#include <stddef.h>
#include "ets_sys.h"
#include "eagle_soc.h"
#include "c_types.h"
#include "gpio.h"
#include "xtensa/corebits.h"

#include "osapi.h"
#include "user_interface.h"
#include "espmissingincludes.h"

//From xtruntime-frames.h
struct XTensa_exception_frame_s {
  uint32_t pc;
  uint32_t ps;
  uint32_t sar;
  uint32_t vpri;
  uint32_t a0;
  uint32_t a[14]; //a2..a15
  // The following are added manually by the exception code; the HAL doesn't set these on an exception.
  uint32_t litbase;
  uint32_t sr176;
  uint32_t sr208;
  uint32_t a1;
  uint32_t reason;
  uint32_t excvaddr;
};

//The asm stub saves the Xtensa registers here when a debugging exception happens.
struct XTensa_exception_frame_s gdbstub_savedRegs;

//Get the value of one of the A registers
static unsigned int getaregval(int reg) {
  if (reg==0) return gdbstub_savedRegs.a0;
  if (reg==1) return gdbstub_savedRegs.a1;
  return gdbstub_savedRegs.a[reg-2];
}

static void print_stack(uint32_t start, uint32_t end) {
  os_printf("\nStack dump:\n");
  for (uint32_t pos = start; pos < end; pos += 0x10) {
    uint32_t* values = (uint32_t*)(pos);
    // rough indicator: stack frames usually have SP saved as the second word
    bool looksLikeStackFrame = (values[2] == pos + 0x10);

    os_printf("%08lx:  %08lx %08lx %08lx %08lx %c\n",
        (long unsigned int) pos, 
        (long unsigned int) values[0], 
        (long unsigned int) values[1],
        (long unsigned int) values[2], 
        (long unsigned int) values[3], 
        (looksLikeStackFrame)?'<':' ');
  }
  os_printf("\n");
}

void _xtos_set_exception_handler(int cause, void (exhandler)(struct XTensa_exception_frame_s *frame));
int os_printf_plus(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));

// Print exception info to console
static void printReason() {
  //register uint32_t sp asm("a1");
  struct XTensa_exception_frame_s *reg = &gdbstub_savedRegs;
  os_printf("\n\n***** Fatal exception %ld\n", (long int) reg->reason);
  os_printf("pc=0x%08lx sp=0x%08lx excvaddr=0x%08lx\n", 
    (long unsigned int) reg->pc, 
    (long unsigned int) reg->a1, 
    (long unsigned int) reg->excvaddr);
  os_printf("ps=0x%08lx sar=0x%08lx vpri=0x%08lx\n", 
    (long unsigned int) reg->ps, 
    (long unsigned int) reg->sar,
    (long unsigned int) reg->vpri);
  for (int i=0; i<16; i++) {
    unsigned int r = getaregval(i);
    os_printf("r%02d: 0x%08x=%10d ", i, r, r);
    if (i%3 == 2) os_printf("\n");
  }
  os_printf("\n");
  //print_stack(reg->pc, sp, 0x3fffffb0);
  print_stack(getaregval(1), 0x3fffffb0);
}

extern void ets_wdt_disable();
extern void ets_wdt_enable();

// Non-OS exception handler. Gets called by the Xtensa HAL.
static void gdb_exception_handler(struct XTensa_exception_frame_s *frame) {
  //Save the extra registers the Xtensa HAL doesn't save
  extern void gdbstub_save_extra_sfrs_for_exception();
  gdbstub_save_extra_sfrs_for_exception();
  //Copy registers the Xtensa HAL did save to gdbstub_savedRegs
  os_memcpy(&gdbstub_savedRegs, frame, 19*4);
  //Credits go to Cesanta for this trick. A1 seems to be destroyed, but because it
  //has a fixed offset from the address of the passed frame, we can recover it.
  //gdbstub_savedRegs.a1=(uint32_t)frame+EXCEPTION_GDB_SP_OFFSET;
  gdbstub_savedRegs.a1=(uint32_t)frame;

  ets_wdt_disable();
  printReason();
  ets_wdt_enable();
  while(1) ;
}

//The OS-less SDK uses the Xtensa HAL to handle exceptions. We can use those functions to catch any
//fatal exceptions and invoke the debugger when this happens.
void gdbstub_init() {
  unsigned int i;
  int exno[]={EXCCAUSE_ILLEGAL, EXCCAUSE_SYSCALL, EXCCAUSE_INSTR_ERROR, EXCCAUSE_LOAD_STORE_ERROR,
      EXCCAUSE_DIVIDE_BY_ZERO, EXCCAUSE_UNALIGNED, EXCCAUSE_INSTR_DATA_ERROR, EXCCAUSE_LOAD_STORE_DATA_ERROR,
      EXCCAUSE_INSTR_ADDR_ERROR, EXCCAUSE_LOAD_STORE_ADDR_ERROR, EXCCAUSE_INSTR_PROHIBITED,
      EXCCAUSE_LOAD_PROHIBITED, EXCCAUSE_STORE_PROHIBITED};
  for (i=0; i<(sizeof(exno)/sizeof(exno[0])); i++) {
    _xtos_set_exception_handler(exno[i], gdb_exception_handler);
  }
}

//extern void gdb_init() __attribute__((weak, alias("gdbstub_init")));
