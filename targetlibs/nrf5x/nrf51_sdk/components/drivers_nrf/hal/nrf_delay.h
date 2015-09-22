#ifndef _NRF_DELAY_H
#define _NRF_DELAY_H

#include "nrf.h"

/*lint --e{438, 522} "Variable not used" "Function lacks side-effects" */
#if defined ( __CC_ARM   )
static __ASM void __INLINE nrf_delay_us(uint32_t volatile number_of_us)
{
loop
        SUBS    R0, R0, #1
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        BNE    loop
        BX     LR
}
#elif defined ( __ICCARM__ )
static void __INLINE nrf_delay_us(uint32_t volatile number_of_us)
{
__ASM (
"loop:\n\t"
       " SUBS R0, R0, #1\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " BNE loop\n\t");
}
#elif defined   (  __GNUC__  )

static void __INLINE nrf_delay_us(uint32_t volatile number_of_us) __attribute__((always_inline));
static void __INLINE nrf_delay_us(uint32_t volatile number_of_us)
{
register uint32_t delay asm ("r0") = number_of_us;
__ASM volatile (
".syntax unified\n"
    "1:\n"
    " SUBS %0, %0, #1\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"   
    " NOP\n"  
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " BNE 1b\n"
    ".syntax divided\n"  
    : "+r" (delay));
}
#endif

void nrf_delay_ms(uint32_t volatile number_of_ms);

#endif
