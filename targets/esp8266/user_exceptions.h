//#include "sections.h"
//#include "rom.h"
#include <xtensa/corebits.h>

//#ifndef _SECTIONS_H_
//#define _SECTIONS_H_

#define TEXT_SECTION_ATTR __attribute__((section(".iram0.text")))
//#define RAM_CONST_SECTION_ATTR __attribute((section(".rodata.dram")))

//#endif
struct exception_frame
{
  uint32_t epc;
  uint32_t ps;
  uint32_t sar;
  uint32_t unused;
  union {
    struct {
      uint32_t a0;
      // note: no a1 here!
      uint32_t a2;
      uint32_t a3;
      uint32_t a4;
      uint32_t a5;
      uint32_t a6;
      uint32_t a7;
      uint32_t a8;
      uint32_t a9;
      uint32_t a10;
      uint32_t a11;
      uint32_t a12;
      uint32_t a13;
      uint32_t a14;
      uint32_t a15;
    };
    uint32_t a_reg[15];
  };
  uint32_t cause;
};
/**
 * C-level exception handler callback prototype.
 *
 * Does not need an RFE instruction - it is called through a wrapper which
 * performs state capture & restore, as well as the actual RFE.
 *
 * @param ef An exception frame containing the relevant state from the
 *           exception triggering. This state may be manipulated and will
 *           be applied on return.
 * @param cause The exception cause number.
 */
typedef void (*exception_handler_fn) (struct exception_frame *ef, uint32_t cause);

/**
 * Sets the exception handler function for a particular exception cause.
 * @param handler The exception handler to install.
 *                If NULL, reverts to the XTOS default handler.
 * @returns The previous exception handler, or NULL if none existed prior.
 */
exception_handler_fn _xtos_set_exception_handler (uint32_t cause, exception_handler_fn handler);




void load_non_32_wide_handler (struct exception_frame *ef, uint32_t cause) TEXT_SECTION_ATTR;
void __real__xtos_set_exception_handler (uint32_t cause, exception_handler_fn fn);
