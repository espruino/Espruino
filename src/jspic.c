#ifdef SDCC

#include "pic18f2620.h"
//#include "pic18f8627.h"
/*
code char at __CONFIG1H CONFIG1H = _IESO_OFF_1H & _FCMEN_OFF_1H & _OSC_INTIO67_1H;
code char at __CONFIG2L CONFIG2L = _PWRT_OFF_2L & _BOREN_OFF_2L;
code char at __CONFIG2H CONFIG2H = _WDT_OFF_2H;
code char at __CONFIG3H CONFIG3H = _MCLRE_ON_3H & _LPT1OSC_OFF_3H & _PBADEN_OFF_3H;
//code char at __CONFIG4L CONFIG4L = _DEBUG_ON_4L & _XINST_ON_4L & _LVP_ON_4L & _STVREN_OFF_4L;
code char at __CONFIG4L CONFIG4L = _DEBUG_OFF_4L & _XINST_OFF_4L & _LVP_OFF_4L & _STVREN_OFF_4L;
code char at __CONFIG5L CONFIG5L = _CP0_OFF_5L & _CP1_OFF_5L & _CP2_OFF_5L & _CP3_OFF_5L;
code char at __CONFIG5H CONFIG5H = _CPB_OFF_5H & _CPD_OFF_5H;
code char at __CONFIG6L CONFIG6L = _WRT0_OFF_6L & _WRT1_OFF_6L & _WRT2_OFF_6L & _WRT3_OFF_6L;
code char at __CONFIG6H CONFIG6H = _WRTC_OFF_6H & _WRTB_OFF_6H & _WRTD_OFF_6H;
code char at __CONFIG7L CONFIG7L = _EBTR0_OFF_7L & _EBTR1_OFF_7L & _EBTR2_OFF_7L & _EBTR3_OFF_7L;
code char at __CONFIG7H CONFIG7H = _EBTRB_OFF_7H;
*/
/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */

void putchar (char c) {
  while (!TXSTAbits.TRMT);
  TXREG = c;
}

char getchar (void) {
  while (!PIR1bits.RCIF);
  return RCREG;
}

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
#define INLINE_FUNC inline

#include "jsvar.c"
#include "jslex.c"
#include "jsparse.c"
#include "jsutils.c"
#include "jsfunctions.c"

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */

void main() {
  // RS232 USART. Set bits up as if FOsc = 32Mhz  
  TXSTAbits.BRGH = 1; BAUDCONbits.BRG16 = 1; SPBRGH = 0; SPBRG = 138; // 57600 (ish!) 
  //TXSTAbits.BRGH = 0; BAUDCONbits.BRG16 = 1; SPBRGH = 0; SPBRG = 104; // 19200 (ish!) 

  TXSTAbits.SYNC = 0; // async
  RCSTAbits.SPEN = 1; // general enable 
  TXSTAbits.TXEN = 1; // tx enable
  RCSTAbits.CREN = 1; // rx enable 



}

#endif // SDCC
