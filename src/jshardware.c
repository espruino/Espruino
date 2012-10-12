/*
 * jshardware.c
 *
 *  Created on: 8 Aug 2012
 *      Author: gw
 */

#ifndef ARM
 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>
 #include <sys/time.h>
 #include <sys/select.h>
 #include <termios.h>
#endif//ARM

#include "jshardware.h"
#include "jsutils.h"
#include "jsparse.h"

#ifdef ARM
 #if USB
  #include "usb_regs.h"
 #endif
#endif

// ----------------------------------------------------------------------------
//                                                                     BUFFERS

// ----------------------------------------------------------------------------
//                                                         DATA TRANSMIT BUFFER
#ifdef ARM
typedef struct {
  IOEventFlags flags; // Where this data should be transmitted
  unsigned char data;         // data to transmit
} PACKED_FLAGS TxBufferItem;

TxBufferItem txBuffer[TXBUFFERMASK+1];
volatile unsigned char txHead=0, txTail=0;
#endif
// ----------------------------------------------------------------------------

// Queue a character for transmission
void jshTransmit(IOEventFlags device, unsigned char data) {
#ifdef ARM
  unsigned char txHeadNext = (txHead+1)&TXBUFFERMASK;
  while (txHeadNext==txTail) ; // wait for send to finish as buffer is about to overflow
  txBuffer[txHead].flags = device;
  txBuffer[txHead].data = (char)data;
  txHead = txHeadNext;

  switch (device) {
    case EV_USART1: USART_ITConfig(USART1, USART_IT_TXE, ENABLE); break; // enable interrupt -> start transmission
    case EV_USART2: USART_ITConfig(USART2, USART_IT_TXE, ENABLE); break; // enable interrupt -> start transmission
  }
#else // if PC, just put to stdout
  fputc(data, stdout);
  fflush(stdout);
#endif
}

#ifdef ARM
// Try and get a character for transmission - could just return -1 if nothing
int jshGetCharToTransmit(IOEventFlags device) {
  unsigned char ptr = txTail;
  while (txHead != ptr) {
    if (IOEVENTFLAGS_GETTYPE(txBuffer[ptr].flags) == device) {
      unsigned char data = txBuffer[ptr].data;
      if (ptr != txTail) { // so we weren't right at the back of the queue
        // we need to work back from ptr (until we hit tail), shifting everything forwards
        unsigned char this = ptr;
        unsigned char last = (this+TXBUFFERMASK)&TXBUFFERMASK;
        while (this!=txTail) { // if this==txTail, then last is before it, so stop here
          txBuffer[this] = txBuffer[last];
          this = last;
          last = (this+TXBUFFERMASK)&TXBUFFERMASK;
        }
      }
      txTail = (txTail+1)&TXBUFFERMASK; // advance the tail
      return data; // return data
    }
    ptr = (ptr+1)&TXBUFFERMASK;
  }
  return -1; // no data :(
}
#endif

void jshTransmitFlush() {
#ifdef ARM
  while (txHead != txTail) ; // wait for send to finish
#endif
}

// ----------------------------------------------------------------------------
//                                                              IO EVENT BUFFER
IOEvent ioBuffer[IOBUFFERMASK+1];
volatile unsigned char ioHead=0, ioTail=0;
// ----------------------------------------------------------------------------


void jshIOEventOverflowed() {
  // TODO: error here?
#ifdef LED2_PORT
    GPIO_SetBits(LED2_PORT, LED2_PIN);
#endif
}


void jshPushIOCharEvent(IOEventFlags channel, char charData) {
  if (charData==3) {
    // Ctrl-C - force interrupt
    // TODO - check if this is our Console port?
    jspSetInterrupted(true);
    return;
  }
  // Check for existing buffer
  if (ioHead!=ioTail) {
    // we can do this because we only read in main loop, and we're in an interrupt here
    unsigned char lastHead = (ioHead+IOBUFFERMASK) & IOBUFFERMASK;
    if (IOEVENTFLAGS_GETTYPE(ioBuffer[lastHead].flags) == channel &&
        IOEVENTFLAGS_GETCHARS(ioBuffer[lastHead].flags) < IOEVENT_MAXCHARS) {
      // last event was for this event type, and it has chars left
      unsigned char c = IOEVENTFLAGS_GETCHARS(ioBuffer[lastHead].flags);
      ioBuffer[lastHead].data.chars[c] = charData;
      IOEVENTFLAGS_SETCHARS(ioBuffer[lastHead].flags, c+1);
      return;
    }
  }
  // Make new buffer
  unsigned char nextHead = (ioHead+1) & IOBUFFERMASK;
  if (ioTail == nextHead) {
    jshIOEventOverflowed();
    return; // queue full - dump this event!
  }
  ioBuffer[ioHead].flags = channel;
  IOEVENTFLAGS_SETCHARS(ioBuffer[ioHead].flags, 1);
  ioBuffer[ioHead].data.chars[0] = charData;  
  ioHead = nextHead;
}

void jshPushIOEvent(IOEventFlags channel, JsSysTime time) {
  unsigned char nextHead = (ioHead+1) & IOBUFFERMASK;
  if (ioTail == nextHead) {
    jshIOEventOverflowed();
    return; // queue full - dump this event!
  }
  ioBuffer[ioHead].flags = channel;
  ioBuffer[ioHead].data.time = time;
  ioHead = nextHead;
}

// returns true on success
bool jshPopIOEvent(IOEvent *result) {
  if (ioHead==ioTail) return false;
  *result = ioBuffer[ioTail];
  ioTail = (ioTail+1) & IOBUFFERMASK;
  return true;
}

bool jshHasEvents() {
  return ioHead!=ioTail;
}


// ----------------------------------------------------------------------------
//                                                                        PINS
#ifdef ARM


typedef struct IOPin {
  uint16_t pin;      // GPIO_Pin_1
  GPIO_TypeDef *gpio; // GPIOA
  uint8_t adc; // 0xFF or ADC_Channel_1
  /* timer - bits 0-1 = channel
             bits 2-5 = timer (NOTE - so only 0-15!)
             bit 6    = remap - for F1 part crazy stuff
             bit 7    = negated */
  uint8_t timer;
} PACKED_FLAGS IOPin;

#define TIMER(TIM,CH) (TIM<<2)|(CH-1)
#define TIMERN(TIM,CH) (TIM<<2)|(CH-1)|0x80
#define TIMNONE 0
#define TIMREMAP 64
#define TIMER_REMAP(X) ((X)&64)
#define TIMER_CH(X) (((X)&3)+1)
#define TIMER_TMR(X) (((X)>>2)&15)
#define TIMER_NEG(X) (((X)>>7)&1)

#ifdef STM32F4
 #define IOPINS 82 // 16*5+2
 // some of these use the same timer!
 const IOPin IOPIN_DATA[IOPINS] = {
 { GPIO_Pin_0,  GPIOA, ADC_Channel_0, TIMER(5,1) },
 { GPIO_Pin_1,  GPIOA, ADC_Channel_1, TIMER(5,2) },
 { GPIO_Pin_2,  0, 0xFF, TIMNONE },//A2 - Serial
 { GPIO_Pin_3,  0, 0xFF, TIMNONE },//A3 - Serial
 { GPIO_Pin_4,  GPIOA, ADC_Channel_4, TIMNONE },
 { GPIO_Pin_5,  GPIOA, ADC_Channel_5, TIMNONE },
 { GPIO_Pin_6,  GPIOA, ADC_Channel_6, TIMER(13,1) }, //+ TIMER(3,1)
 { GPIO_Pin_7,  GPIOA, ADC_Channel_7, TIMER(14,1) }, //+ TIMERN(8,1) + TIMER(3,2) + TIMERN(1,1)
 { GPIO_Pin_8,  GPIOA, 0xFF, TIMER(1,1) },
 { GPIO_Pin_9,  GPIOA, 0xFF, TIMER(1,2) }, 
 { GPIO_Pin_10, GPIOA, 0xFF, TIMER(1,3) }, 
 { GPIO_Pin_11, GPIOA, 0xFF, TIMER(1,4) },
 { GPIO_Pin_12, GPIOA, 0xFF, TIMNONE },
 { GPIO_Pin_13, GPIOA, 0xFF, TIMNONE },
 { GPIO_Pin_14, GPIOA, 0xFF, TIMNONE },
 { GPIO_Pin_15, GPIOA, 0xFF, TIMNONE },

 { GPIO_Pin_0,  GPIOB, ADC_Channel_8, TIMER(3,3) }, //+ TIMERN(8,2) + TIMERN(1,2)
 { GPIO_Pin_1,  GPIOB, ADC_Channel_9, TIMER(3,4) }, //+ TIMERN(8,3) + TIMERN(1,3)
 { GPIO_Pin_2,  GPIOB, 0xFF, TIMNONE },
 { GPIO_Pin_3,  GPIOB, 0xFF, TIMER(2,2) },
 { GPIO_Pin_4,  GPIOB, 0xFF, TIMER(3,1) },
 { GPIO_Pin_5,  GPIOB, 0xFF, TIMER(3,2) },
 { GPIO_Pin_6,  GPIOB, 0xFF, TIMER(4,1) },
 { GPIO_Pin_7,  GPIOB, 0xFF, TIMER(4,2) },
 { GPIO_Pin_8,  GPIOB, 0xFF, TIMER(4,3) }, //+TIMER(10,1)
 { GPIO_Pin_9,  GPIOB, 0xFF, TIMER(4,4) }, //+TIMER(11,1)
 { GPIO_Pin_10, GPIOB, 0xFF, TIMER(2,3) },
 { GPIO_Pin_11, GPIOB, 0xFF, TIMER(2,4) },
 { GPIO_Pin_12, GPIOB, 0xFF, TIMNONE },
 { GPIO_Pin_13, GPIOB, 0xFF, TIMERN(1,1) },
 { GPIO_Pin_14, GPIOB, 0xFF, TIMER(12,1) }, //+TIMERN(1,2)+TIMERN(8,2)
 { GPIO_Pin_15, GPIOB, 0xFF, TIMER(12,2) }, //+TIMERN(1,3)+TIMERN(8,3)

 { GPIO_Pin_0,  GPIOC, ADC_Channel_10, TIMNONE },
 { GPIO_Pin_1,  GPIOC, ADC_Channel_11, TIMNONE },
 { GPIO_Pin_2,  GPIOC, ADC_Channel_12, TIMNONE },
 { GPIO_Pin_3,  GPIOC, ADC_Channel_13, TIMNONE },
 { GPIO_Pin_4,  GPIOC, ADC_Channel_14, TIMNONE },
 { GPIO_Pin_5,  GPIOC, ADC_Channel_15, TIMNONE },
 { GPIO_Pin_6,  GPIOC, 0xFF, TIMER(8,1) }, //+, TIMER(3,1)
 { GPIO_Pin_7,  GPIOC, 0xFF, TIMER(8,2) },//+, TIMER(3,2)
 { GPIO_Pin_8,  GPIOC, 0xFF, TIMER(8,3) },//+, TIMER(3,3)
 { GPIO_Pin_9,  GPIOC, 0xFF, TIMER(8,4) }, //+, TIMER(3,4)
 { GPIO_Pin_10, GPIOC, 0xFF, TIMNONE },
 { GPIO_Pin_11, GPIOC, 0xFF, TIMNONE },
 { GPIO_Pin_12, GPIOC, 0xFF, TIMNONE },
 { GPIO_Pin_13, GPIOC, 0xFF, TIMNONE },
 { GPIO_Pin_14, GPIOC, 0xFF, TIMNONE },
 { GPIO_Pin_15, GPIOC, 0xFF, TIMNONE },

 { GPIO_Pin_0,  GPIOD, 0xFF, TIMNONE },
 { GPIO_Pin_1,  GPIOD, 0xFF, TIMNONE },
 { GPIO_Pin_2,  GPIOD, 0xFF, TIMNONE },
 { GPIO_Pin_3,  GPIOD, 0xFF, TIMNONE },
 { GPIO_Pin_4,  GPIOD, 0xFF, TIMNONE },
 { GPIO_Pin_5,  GPIOD, 0xFF, TIMNONE },
 { GPIO_Pin_6,  GPIOD, 0xFF, TIMNONE },
 { GPIO_Pin_7,  GPIOD, 0xFF, TIMNONE },
 { GPIO_Pin_8,  GPIOD, 0xFF, TIMNONE },
 { GPIO_Pin_9,  GPIOD, 0xFF, TIMNONE },
 { GPIO_Pin_10, GPIOD, 0xFF, TIMNONE },
 { GPIO_Pin_11, GPIOD, 0xFF, TIMNONE },
 { GPIO_Pin_12, GPIOD, 0xFF, TIMER(4,1) },
 { GPIO_Pin_13, GPIOD, 0xFF, TIMER(4,2) },
 { GPIO_Pin_14, GPIOD, 0xFF, TIMER(4,3) },
 { GPIO_Pin_15, GPIOD, 0xFF, TIMER(4,4) },

 { GPIO_Pin_0,  GPIOE, 0xFF },
 { GPIO_Pin_1,  GPIOE, 0xFF },
 { GPIO_Pin_2,  GPIOE, 0xFF },
 { GPIO_Pin_3,  GPIOE, 0xFF },
 { GPIO_Pin_4,  GPIOE, 0xFF },
 { GPIO_Pin_5,  GPIOE, 0xFF, TIMER(9,1) },
 { GPIO_Pin_6,  GPIOE, 0xFF, TIMER(9,2) },
 { GPIO_Pin_7,  GPIOE, 0xFF },
 { GPIO_Pin_8,  GPIOE, 0xFF, TIMERN(1,1) },
 { GPIO_Pin_9,  GPIOE, 0xFF, TIMER(1,1) },
 { GPIO_Pin_10, GPIOE, 0xFF, TIMERN(1,2) },
 { GPIO_Pin_11, GPIOE, 0xFF, TIMER(1,2) },
 { GPIO_Pin_12, GPIOE, 0xFF, TIMERN(1,3) },
 { GPIO_Pin_13, GPIOE, 0xFF, TIMER(1,3) },
 { GPIO_Pin_14, GPIOE, 0xFF, TIMER(1,4) },
 { GPIO_Pin_15, GPIOE, 0xFF },

 { GPIO_Pin_0,  GPIOH, 0xFF },
 { GPIO_Pin_1,  GPIOH, 0xFF },
};
#else
 #define IOPINS 50 // 16*3+2
 const IOPin IOPIN_DATA[IOPINS] = {
 { GPIO_Pin_0,  GPIOA, ADC_Channel_0, TIMNONE },
 { GPIO_Pin_1,  GPIOA, ADC_Channel_1, TIMER(2,2) },
 { GPIO_Pin_2,  GPIOA, ADC_Channel_2, TIMER(2,3) }, // , TIMER(15,1)
 { GPIO_Pin_3,  GPIOA, ADC_Channel_3, TIMER(2,4) }, //, TIMER(15,2)
 { GPIO_Pin_4,  GPIOA, ADC_Channel_4, TIMNONE },
 { GPIO_Pin_5,  GPIOA, ADC_Channel_5, TIMNONE },
 { GPIO_Pin_6,  GPIOA, ADC_Channel_6, TIMER(3,1) }, //, TIMER(16,1)
 { GPIO_Pin_7,  GPIOA, ADC_Channel_7, TIMER(3,2) }, //, TIMER(17,1)
 { GPIO_Pin_8,  GPIOA, 0xFF, TIMER(1,1) },
 { GPIO_Pin_9,  0,     0xFF, TIMER(1,2) }, // A9 - Serial
 { GPIO_Pin_10, 0,     0xFF, TIMER(1,3) }, // A10 - Serial
 { GPIO_Pin_11, GPIOA, 0xFF, TIMER(1,4) },
 { GPIO_Pin_12, GPIOA, 0xFF, TIMNONE },
 { GPIO_Pin_13, GPIOA, 0xFF, TIMNONE },
 { GPIO_Pin_14, GPIOA, 0xFF, TIMNONE },
 { GPIO_Pin_15, GPIOA, 0xFF, TIMNONE },

 { GPIO_Pin_0,  GPIOB, ADC_Channel_8, TIMER(3,3) }, // , TIMERN(1,2)
 { GPIO_Pin_1,  GPIOB, ADC_Channel_9, TIMER(3,4) }, // , TIMER(1,3)
 { GPIO_Pin_2,  GPIOB, 0xFF, TIMNONE },
 { GPIO_Pin_3,  GPIOB, 0xFF, TIMER(2,2) },
 { GPIO_Pin_4,  GPIOB, 0xFF, TIMER(3,1)|TIMREMAP },
 { GPIO_Pin_5,  GPIOB, 0xFF, TIMER(3,2)|TIMREMAP },
 { GPIO_Pin_6,  GPIOB, 0xFF, TIMER(4,1) },
 { GPIO_Pin_7,  GPIOB, 0xFF, TIMER(4,2) },
 { GPIO_Pin_8,  GPIOB, 0xFF, TIMER(4,3) }, //, TIMER(16,1)
 { GPIO_Pin_9,  GPIOB, 0xFF, TIMER(4,4) }, //, TIMER(17,1)
 { GPIO_Pin_10, GPIOB, 0xFF, TIMER(2,3) },
 { GPIO_Pin_11, GPIOB, 0xFF, TIMER(2,4)|TIMREMAP },
 { GPIO_Pin_12, GPIOB, 0xFF, TIMNONE },
 { GPIO_Pin_13, GPIOB, 0xFF, TIMERN(1,1) },
 { GPIO_Pin_14, GPIOB, 0xFF, TIMER(15,1)|TIMREMAP },// , TIMERN(1,2)
 { GPIO_Pin_15, GPIOB, 0xFF, TIMER(15,2)|TIMREMAP },// , TIMERN(1,3), TIMERN(15,1)

 { GPIO_Pin_0,  GPIOC, ADC_Channel_10, TIMNONE  },
 { GPIO_Pin_1,  GPIOC, ADC_Channel_11, TIMNONE  },
 { GPIO_Pin_2,  GPIOC, ADC_Channel_12, TIMNONE  },
 { GPIO_Pin_3,  GPIOC, ADC_Channel_13, TIMNONE  },
 { GPIO_Pin_4,  GPIOC, ADC_Channel_14, TIMNONE  },
 { GPIO_Pin_5,  GPIOC, ADC_Channel_15, TIMNONE  },
 { GPIO_Pin_6,  GPIOC, 0xFF, TIMER(3,1)|TIMREMAP },
 { GPIO_Pin_7,  GPIOC, 0xFF, TIMER(3,2)|TIMREMAP },
 { GPIO_Pin_8,  GPIOC, 0xFF, TIMER(3,3)|TIMREMAP },
 { GPIO_Pin_9,  GPIOC, 0xFF, TIMER(3,4)|TIMREMAP },
 { GPIO_Pin_10, GPIOC, 0xFF, TIMNONE },
 { GPIO_Pin_11, GPIOC, 0xFF, TIMNONE },
 { GPIO_Pin_12, GPIOC, 0xFF, TIMNONE },
 { GPIO_Pin_13, GPIOC, 0xFF, TIMNONE/*?*/ },
 { GPIO_Pin_14, GPIOC, 0xFF, TIMNONE/*?*/ },
 { GPIO_Pin_15, GPIOC, 0xFF, TIMNONE/*?*/ },

 { GPIO_Pin_0,  GPIOD, 0xFF, TIMNONE/*?*/ },
 { GPIO_Pin_1,  GPIOD, 0xFF, TIMNONE/*?*/ }, 
 // D2 ?
};
#endif

uint8_t pinToPinSource(uint16_t pin) {
  if (pin==GPIO_Pin_0 ) return GPIO_PinSource0;
  if (pin==GPIO_Pin_1 ) return GPIO_PinSource1;
  if (pin==GPIO_Pin_2 ) return GPIO_PinSource2;
  if (pin==GPIO_Pin_3 ) return GPIO_PinSource3;
  if (pin==GPIO_Pin_4 ) return GPIO_PinSource4;
  if (pin==GPIO_Pin_5 ) return GPIO_PinSource5;
  if (pin==GPIO_Pin_6 ) return GPIO_PinSource6;
  if (pin==GPIO_Pin_7 ) return GPIO_PinSource7;
  if (pin==GPIO_Pin_8 ) return GPIO_PinSource8;
  if (pin==GPIO_Pin_9 ) return GPIO_PinSource9;
  if (pin==GPIO_Pin_10) return GPIO_PinSource10;
  if (pin==GPIO_Pin_11) return GPIO_PinSource11;
  if (pin==GPIO_Pin_12) return GPIO_PinSource12;
  if (pin==GPIO_Pin_13) return GPIO_PinSource13;
  if (pin==GPIO_Pin_14) return GPIO_PinSource14;
  if (pin==GPIO_Pin_15) return GPIO_PinSource15;
  return GPIO_PinSource0;
}
uint8_t pinToEVEXTI(uint16_t pin) {
  if (pin==GPIO_Pin_0 ) return EV_EXTI0;
  if (pin==GPIO_Pin_1 ) return EV_EXTI1;
  if (pin==GPIO_Pin_2 ) return EV_EXTI2;
  if (pin==GPIO_Pin_3 ) return EV_EXTI3;
  if (pin==GPIO_Pin_4 ) return EV_EXTI4;
  if (pin==GPIO_Pin_5 ) return EV_EXTI5;
  if (pin==GPIO_Pin_6 ) return EV_EXTI6;
  if (pin==GPIO_Pin_7 ) return EV_EXTI7;
  if (pin==GPIO_Pin_8 ) return EV_EXTI8;
  if (pin==GPIO_Pin_9 ) return EV_EXTI9;
  if (pin==GPIO_Pin_10) return EV_EXTI10;
  if (pin==GPIO_Pin_11) return EV_EXTI11;
  if (pin==GPIO_Pin_12) return EV_EXTI12;
  if (pin==GPIO_Pin_13) return EV_EXTI13;
  if (pin==GPIO_Pin_14) return EV_EXTI14;
  if (pin==GPIO_Pin_15) return EV_EXTI15;
  return GPIO_PinSource0;
}
uint8_t portToPortSource(GPIO_TypeDef *port) {
 #ifdef STM32F4
  if (port == GPIOA) return EXTI_PortSourceGPIOA;
  if (port == GPIOB) return EXTI_PortSourceGPIOB;
  if (port == GPIOC) return EXTI_PortSourceGPIOC;
  if (port == GPIOD) return EXTI_PortSourceGPIOD;
  if (port == GPIOE) return EXTI_PortSourceGPIOE;
  if (port == GPIOF) return EXTI_PortSourceGPIOF;
  if (port == GPIOG) return EXTI_PortSourceGPIOG;
  if (port == GPIOH) return EXTI_PortSourceGPIOH;
  return EXTI_PortSourceGPIOA;
#else
  if (port == GPIOA) return GPIO_PortSourceGPIOA;
  if (port == GPIOB) return GPIO_PortSourceGPIOB;
  if (port == GPIOC) return GPIO_PortSourceGPIOC;
  if (port == GPIOD) return GPIO_PortSourceGPIOD;
  if (port == GPIOE) return GPIO_PortSourceGPIOE;
  if (port == GPIOF) return GPIO_PortSourceGPIOF;
  if (port == GPIOG) return GPIO_PortSourceGPIOG;
  return GPIO_PortSourceGPIOA;
#endif
}


#endif
// ----------------------------------------------------------------------------
#ifdef ARM
JsSysTime SysTickMajor = SYSTICK_RANGE;
void jshDoSysTick() {
  SysTickMajor+=SYSTICK_RANGE;
}

#endif//ARM
// ----------------------------------------------------------------------------
#ifndef ARM
// for non-blocking IO
struct termios orig_termios;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        if (c=='\3') exit(0); // ctrl-c
        return c;
    }
}
#endif


void jshInit() {
#ifdef ARM
  /* Enable UART and  GPIOx Clock */
 #ifdef STM32F4
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR |
                         RCC_APB1Periph_USART2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |
                         RCC_AHB1Periph_GPIOB |
                         RCC_AHB1Periph_GPIOC | 
                         RCC_AHB1Periph_GPIOD |
                         RCC_AHB1Periph_GPIOE |
                         RCC_AHB1Periph_GPIOF |
                         RCC_AHB1Periph_GPIOG |
                         RCC_AHB1Periph_GPIOH, ENABLE);
 #else
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_ADC1 |
        RCC_APB2Periph_USART1 |
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_GPIOC |
        RCC_APB2Periph_GPIOD |
        RCC_APB2Periph_AFIO, ENABLE);
 #endif
  /* System Clock */
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
  SysTick_Config(SYSTICK_RANGE-1); // 24 bit
  /* Initialise LEDs LD3&LD4, both on */
  GPIO_InitTypeDef GPIO_InitStructure;
#ifdef STM32F4
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
#else
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
#endif
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#ifdef LED1_PORT
  GPIO_InitStructure.GPIO_Pin = LED1_PIN;  
  GPIO_Init(LED1_PORT, &GPIO_InitStructure);
#endif
#ifdef LED2_PORT
  GPIO_InitStructure.GPIO_Pin = LED2_PIN;  
  GPIO_Init(LED2_PORT, &GPIO_InitStructure);
#endif
#ifdef LED3_PORT
  GPIO_InitStructure.GPIO_Pin = LED3_PIN;  
  GPIO_Init(LED3_PORT, &GPIO_InitStructure);
#endif
#ifdef LED4_PORT
  GPIO_InitStructure.GPIO_Pin = LED4_PIN;  
  GPIO_Init(LED4_PORT, &GPIO_InitStructure);
#endif

#ifdef STM32F4
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
#else
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
#endif
#ifdef BTN_PORT  
  GPIO_InitStructure.GPIO_Pin = BTN_PIN;  
  GPIO_Init(BTN_PORT, &GPIO_InitStructure);
#endif
  GPIO_SetBits(LED1_PORT,LED1_PIN);
  GPIO_SetBits(LED2_PORT,LED2_PIN);
  volatile int cnt; for (cnt=0;cnt<10000;cnt++); // small delay
  GPIO_ResetBits(LED2_PORT,LED2_PIN);



  GPIO_InitStructure.GPIO_Pin = MAIN_USART_Pin_RX; // RX
#ifdef STM32F4
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // or UP?
#else
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
#endif
  GPIO_Init(MAIN_USART_Port, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = MAIN_USART_Pin_TX; // TX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#ifdef STM32F4
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // alternate fn
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
#else
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
#endif
  GPIO_Init(MAIN_USART_Port, &GPIO_InitStructure);

#ifdef STM32F4
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
#endif

  USART_ClockInitTypeDef USART_ClockInitStructure;
  USART_ClockStructInit(&USART_ClockInitStructure);
  USART_ClockInit(MAIN_USART, &USART_ClockInitStructure);

  USART_InitTypeDef USART_InitStructure;
#ifdef STM32F4
  USART_InitStructure.USART_BaudRate = 9600;//38400; // FIXME wtf
#else
  USART_InitStructure.USART_BaudRate = 9600;//38400;
#endif
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  //Write USART parameters
  USART_Init(MAIN_USART, &USART_InitStructure);


  // enable uart interrupt
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = JOIN_MAIN_USART(_IRQn);
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );
  // Enable RX interrupt (TX is done when we have bytes)
  USART_ClearITPendingBit(MAIN_USART, USART_IT_RXNE);
  USART_ITConfig(MAIN_USART, USART_IT_RXNE, ENABLE);
  //Enable USART
  USART_Cmd(MAIN_USART, ENABLE);
/*
  while (true) {
    STM32vldiscovery_LEDOn(LED5);
    USART_SendData(USART2, (uint8_t) 'G');
    //Loop until the end of transmission
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
   STM32vldiscovery_LEDOn(LED4);
    USART_SendData(USART2, (uint8_t) 'W');
    //Loop until the end of transmission
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);

  }*/


  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_Init(&NVIC_InitStructure);

#ifdef STM32F4
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_CommonStructInit(&ADC_CommonInitStructure);
  ADC_CommonInitStructure.ADC_Mode			= ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler			= ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode		= ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay	        = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);
#endif

  // ADC Structure Initialization
  ADC_InitTypeDef ADC_InitStructure;
  ADC_StructInit(&ADC_InitStructure);

  // Preinit
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
#ifdef STM32F4
  ADC_InitStructure.ADC_ExternalTrigConvEdge		= ADC_ExternalTrigConvEdge_None;
#else
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
#endif
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Left;
  ADC_Init(ADC1, &ADC_InitStructure);

  // Enable the ADC
  ADC_Cmd(ADC1, ENABLE);

#ifdef STM32F4
  // No calibration??
#else
  // Calibrate
  ADC_ResetCalibration(ADC1);
  while(ADC_GetResetCalibrationStatus(ADC1));
  ADC_StartCalibration(ADC1);
  while(ADC_GetCalibrationStatus(ADC1));
#endif

#ifdef ARM
  jsPrintInt(SystemCoreClock/1000000);jsPrint(" Mhz\r\n");
#endif

#else//!ARM
  //
  struct termios new_termios;

  /* take two copies - one for now, one for later */
  tcgetattr(0, &orig_termios);
  memcpy(&new_termios, &orig_termios, sizeof(new_termios));

  /* register cleanup handler, and set the new terminal mode */
  atexit(reset_terminal_mode);
  cfmakeraw(&new_termios);
  tcsetattr(0, TCSANOW, &new_termios);
#endif//ARM
}

void jshKill() {
}


void jshIdle() {
#ifdef ARM
#else
  while (kbhit()) {
    jshPushIOCharEvent(EV_USART1, getch());
  }
#endif
}

#ifdef USB
void USART_To_USB_Send_Data(char ch); // FIXME
#endif

void jshTX(char data) {
  jshTransmit(EV_USART1, data);
  jshTransmit(EV_USBSERIAL, data);
}

void jshTXStr(const char *str) {
  while (*str) {
       if (*str == '\n') jshTX('\r');
       jshTX(*(str++));
  }
}

// ----------------------------------------------------------------------------

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
#ifdef ARM
  return (JsSysTime)((ms*SystemCoreClock)/1000);
#else
  return (JsSysTime)(ms*1000);
#endif
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
#ifdef ARM
  return ((JsVarFloat)time)*1000/SystemCoreClock;
#else
  return ((JsVarFloat)time)/1000;
#endif
}


JsSysTime jshGetSystemTime() {
#ifdef ARM
  JsSysTime t1 = SysTickMajor;
  JsSysTime time = (JsSysTime)SysTick->VAL;
  JsSysTime t2 = SysTickMajor;
  // times are different and systick has rolled over
  if (t1!=t2 && time > (SYSTICK_RANGE>>1)) 
    return t2 - time;
  return t1-time;
#else
  struct timeval tm;
  gettimeofday(&tm, 0);
  return tm.tv_sec*1000000L + tm.tv_usec;
#endif
}

// ----------------------------------------------------------------------------

int jshGetPinFromVar(JsVar *pinv) {
  int pin=-1;
  if (jsvIsString(pinv)) {
#ifdef ARM
    uint16_t gpiopin = 0;
    GPIO_TypeDef *gpioport = 0;
    if (pinv->varData.str[0]=='A') gpioport = GPIOA;
    else if (pinv->varData.str[0]=='B') gpioport = GPIOB;
    else if (pinv->varData.str[0]=='C') gpioport = GPIOC;
    else if (pinv->varData.str[0]=='D') gpioport = GPIOD;
#ifdef STM32F4
    else if (pinv->varData.str[0]=='E') gpioport = GPIOE;
    else if (pinv->varData.str[0]=='H') gpioport = GPIOH;
#endif
    if (gpioport) {
      gpiopin = 1 << stringToInt(&pinv->varData.str[1]);
      int i;
      for (i=0;i<IOPINS;i++)
        if (IOPIN_DATA[i].pin == gpiopin &&
            IOPIN_DATA[i].gpio == gpioport)
                pin = i;
    }
#endif
  } else if (jsvIsInt(pinv)) {
    pin = (int)jsvGetInteger(pinv);
  }
  return pin;
}

bool jshPinInput(int pin) {
  bool value = false;
#ifdef ARM
  if (pin>=0 && pin < IOPINS && IOPIN_DATA[pin].gpio) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = IOPIN_DATA[pin].pin;
#ifdef STM32F4    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    
#else
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
#endif
    GPIO_Init(IOPIN_DATA[pin].gpio, &GPIO_InitStructure);


    value = GPIO_ReadInputDataBit(IOPIN_DATA[pin].gpio, IOPIN_DATA[pin].pin) ? 1 : 0;
  } else jsError("Invalid pin!");
#endif
  return value;
}

JsVarFloat jshPinAnalog(int pin) {
  JsVarFloat value = 0;
#ifdef ARM
  if (pin>=0 && pin < IOPINS && IOPIN_DATA[pin].gpio && IOPIN_DATA[pin].adc!=0xFF) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = IOPIN_DATA[pin].pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
#ifdef STM32F4    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    
#endif
    GPIO_Init(IOPIN_DATA[pin].gpio, &GPIO_InitStructure);

    // Configure chanel
#ifdef STM32F4    
    ADC_RegularChannelConfig(ADC1, IOPIN_DATA[pin].adc, 1, ADC_SampleTime_480Cycles);
#else
    ADC_RegularChannelConfig(ADC1, IOPIN_DATA[pin].adc, 1, ADC_SampleTime_239Cycles5/*ADC_SampleTime_55Cycles5*/);
#endif

    // Start the conversion
#ifdef STM32F4    
    ADC_SoftwareStartConv(ADC1);
#else
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
#endif

    // Wait until conversion completion
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

    // Get the conversion value
    value = ADC_GetConversionValue(ADC1) / (JsVarFloat)65535;
  } else jsError("Invalid analog pin!");
#endif
  return value;
}

#ifdef ARM
static inline void jshSetPinValue(int pin, bool value) {
#ifdef STM32F4 
    if (value)
      GPIO_SetBits(IOPIN_DATA[pin].gpio, IOPIN_DATA[pin].pin);
    else
      GPIO_ResetBits(IOPIN_DATA[pin].gpio, IOPIN_DATA[pin].pin);
#else
    if (value)
      IOPIN_DATA[pin].gpio->BSRR = IOPIN_DATA[pin].pin;
    else
      IOPIN_DATA[pin].gpio->BRR = IOPIN_DATA[pin].pin;
#endif
}
#endif

void jshPinOutput(int pin, bool value) {
#ifdef ARM
  if (pin>=0 && pin < IOPINS && IOPIN_DATA[pin].gpio) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = IOPIN_DATA[pin].pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#ifdef STM32F4 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
#else
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
#endif
    GPIO_Init(IOPIN_DATA[pin].gpio, &GPIO_InitStructure);

    jshSetPinValue(pin, value);
  } else jsError("Invalid pin!");
#endif
}

void jshPinAnalogOutput(int pin, JsVarFloat value) {
  if (value<0) value=0;
  if (value>1) value=1;
#ifdef ARM
  if (pin>=0 && pin < IOPINS && IOPIN_DATA[pin].gpio && IOPIN_DATA[pin].timer!=TIMNONE) {
    TIM_TypeDef* TIMx;
#ifdef STM32F4
 #define STM32F4ONLY(X) X
    uint8_t afmap;
#else
 #define STM32F4ONLY(X)
#endif

    if (TIMER_TMR(IOPIN_DATA[pin].timer)==1) {
      TIMx = TIM1;
      STM32F4ONLY(afmap=GPIO_AF_TIM1);
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);  
    } else if (TIMER_TMR(IOPIN_DATA[pin].timer)==2)  {
      TIMx = TIM2;
      STM32F4ONLY(afmap=GPIO_AF_TIM2);
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); 
    } else if (TIMER_TMR(IOPIN_DATA[pin].timer)==3)  {
      TIMx = TIM3;
      STM32F4ONLY(afmap=GPIO_AF_TIM3);
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  
    } else if (TIMER_TMR(IOPIN_DATA[pin].timer)==4)  {
      TIMx = TIM4;
      STM32F4ONLY(afmap=GPIO_AF_TIM4);
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  
    } else if (TIMER_TMR(IOPIN_DATA[pin].timer)==5) {
      TIMx = TIM5;
      STM32F4ONLY(afmap=GPIO_AF_TIM5);
       RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);  
/*    } else if (TIMER_TMR(IOPIN_DATA[pin].timer)==6)  { // Not used for outputs
      TIMx = TIM6;
      STM32F4ONLY(afmap=GPIO_AF_TIM6);
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);  
    } else if (TIMER_TMR(IOPIN_DATA[pin].timer)==7)  { // Not used for outputs
      TIMx = TIM7;
      STM32F4ONLY(afmap=GPIO_AF_TIM7);
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE); */
    } else if (TIMER_TMR(IOPIN_DATA[pin].timer)==8) {
      TIMx = TIM8;
      STM32F4ONLY(afmap=GPIO_AF_TIM8);
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);  
    } else if (TIMER_TMR(IOPIN_DATA[pin].timer)==9)  {
      TIMx = TIM9;
      STM32F4ONLY(afmap=GPIO_AF_TIM9);
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);  
    } else if (TIMER_TMR(IOPIN_DATA[pin].timer)==10)  {
      TIMx = TIM10;
      STM32F4ONLY(afmap=GPIO_AF_TIM10);
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE); 
    } else if (TIMER_TMR(IOPIN_DATA[pin].timer)==11)  {
      TIMx = TIM11;
      STM32F4ONLY(afmap=GPIO_AF_TIM11);
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11, ENABLE); 
    } else if (TIMER_TMR(IOPIN_DATA[pin].timer)==12)  {
      TIMx = TIM12;
      STM32F4ONLY(afmap=GPIO_AF_TIM12);
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE); 
    } else if (TIMER_TMR(IOPIN_DATA[pin].timer)==13)  {
      TIMx = TIM13;
      STM32F4ONLY(afmap=GPIO_AF_TIM13);
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE); 
    } else if (TIMER_TMR(IOPIN_DATA[pin].timer)==14)  {
      TIMx = TIM14;
      STM32F4ONLY(afmap=GPIO_AF_TIM14);
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE); 
    } else return; // eep!
    //   /* Compute the prescaler value */
  

  /* Time base configuration */
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
//  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1;
//  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration*/
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  TIM_OCStructInit(&TIM_OCInitStructure);
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = (uint32_t)(value*TIM_TimeBaseStructure.TIM_Period);
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  if (TIMER_CH(IOPIN_DATA[pin].timer)==1) {
    TIM_OC1Init(TIMx, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Enable);
  } else if (TIMER_CH(IOPIN_DATA[pin].timer)==2) {
    TIM_OC2Init(TIMx, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable);
  } else if (TIMER_CH(IOPIN_DATA[pin].timer)==3) {
    TIM_OC3Init(TIMx, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Enable);
  } else if (TIMER_CH(IOPIN_DATA[pin].timer)==4) {
    TIM_OC4Init(TIMx, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Enable);
  }
  TIM_ARRPreloadConfig(TIMx, ENABLE); // ARR = Period. Not sure if we need preloads?

  // enable the timer
  TIM_Cmd(TIMx, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = IOPIN_DATA[pin].pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#ifdef STM32F4 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ; // required?
#else
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
#endif
    GPIO_Init(IOPIN_DATA[pin].gpio, &GPIO_InitStructure);
#ifdef STM32F4
    // connect timer pin up
    GPIO_PinAFConfig(IOPIN_DATA[pin].gpio, pinToPinSource(IOPIN_DATA[pin].pin), afmap);
#else
    if (TIMER_REMAP(IOPIN_DATA[pin].timer)) {
      if (TIMER_TMR(IOPIN_DATA[pin].timer)==1) GPIO_PinRemapConfig( GPIO_FullRemap_TIM1, ENABLE );
      else if (TIMER_TMR(IOPIN_DATA[pin].timer)==2) GPIO_PinRemapConfig( GPIO_FullRemap_TIM2, ENABLE );
      else if (TIMER_TMR(IOPIN_DATA[pin].timer)==3) GPIO_PinRemapConfig( GPIO_FullRemap_TIM3, ENABLE );
      else if (TIMER_TMR(IOPIN_DATA[pin].timer)==4) GPIO_PinRemapConfig( GPIO_Remap_TIM4, ENABLE );
      else if (TIMER_TMR(IOPIN_DATA[pin].timer)==15) GPIO_PinRemapConfig( GPIO_Remap_TIM15, ENABLE );
      else jsError("(internal) Remap needed, but unknown timer."); 
    }
#endif

  } else jsError("Invalid pin, or pin not capable of analog output!");
#endif
}

void jshPinPulse(int pin, bool value, JsVarFloat time) {
 JsSysTime ticks = jshGetTimeFromMilliseconds(time);
 //jsPrintInt(ticks);jsPrint("\n");
#ifdef ARM
  if (pin>=0 && pin < IOPINS && IOPIN_DATA[pin].gpio) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = IOPIN_DATA[pin].pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#ifdef STM32F4 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
#else
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
#endif
    GPIO_Init(IOPIN_DATA[pin].gpio, &GPIO_InitStructure);


    jshSetPinValue(pin, value);
    JsSysTime starttime = jshGetSystemTime();
    JsSysTime endtime = starttime + ticks;
    //jsPrint("----------- ");jsPrintInt(endtime>>16);jsPrint("\n");
    JsSysTime stime = jshGetSystemTime();
    while (stime>=starttime && stime<endtime && !jspIsInterrupted()) { // this stops rollover issue
      stime = jshGetSystemTime();
      //jsPrintInt(stime>>16);jsPrint("\n");
    }
    jshSetPinValue(pin, !value);
  } else jsError("Invalid pin!");
#endif
}

void jshPinWatch(int pin, bool shouldWatch) {
#ifdef ARM
  if (pin>=0 && pin < IOPINS && IOPIN_DATA[pin].gpio) {
    // TODO: check for DUPs, also disable interrupt
    int idx = pinToPinSource(IOPIN_DATA[pin].pin);
    /*if (pinInterrupt[idx].pin>PININTERRUPTS) jsError("Interrupt already used");
    pinInterrupt[idx].pin = pin;
    pinInterrupt[idx].fired = false;
    pinInterrupt[idx].callbacks = ...;*/

    // set as input
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = IOPIN_DATA[pin].pin;
#ifdef STM32F4    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    
#else
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
#endif
    GPIO_Init(IOPIN_DATA[pin].gpio, &GPIO_InitStructure);

#ifdef STM32F4 
    SYSCFG_EXTILineConfig(portToPortSource(IOPIN_DATA[pin].gpio), pinToPinSource(IOPIN_DATA[pin].pin));
#else
    GPIO_EXTILineConfig(portToPortSource(IOPIN_DATA[pin].gpio), pinToPinSource(IOPIN_DATA[pin].pin));
#endif

    EXTI_InitTypeDef s;
    EXTI_StructInit(&s);
    s.EXTI_Line = IOPIN_DATA[pin].pin; //EXTI_Line0
    s.EXTI_Mode =  EXTI_Mode_Interrupt;
    s.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    s.EXTI_LineCmd = ENABLE;
    EXTI_Init(&s);
  } else jsError("Invalid pin!");
#endif
}

bool jshIsEventForPin(IOEvent *event, int pin) {
#ifdef ARM
  return IOEVENTFLAGS_GETTYPE(event->flags) == pinToEVEXTI(IOPIN_DATA[pin].pin);
#else
  return false;
#endif
}

#ifdef ARM
 #if defined(STM32F4)
  #define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */
  #define FLASH_MEMORY_SIZE (1024*1024)
  #define FLASH_PAGE_SIZE (128*1024)
  #define FLASH_PAGES 1
 #elif defined(OLIMEXINO_STM32)
  #define FLASH_MEMORY_SIZE (128*1024)
  #define FLASH_PAGE_SIZE 1024
  #define FLASH_PAGES 14
 #else
  #define FLASH_MEMORY_SIZE (128*1024)
  #define FLASH_PAGE_SIZE 1024
  #define FLASH_PAGES 6
 #endif
#else
 #define FLASH_MEMORY_SIZE (128*1024)
 #define FLASH_PAGE_SIZE 1024
 #define FLASH_PAGES 32
#endif

#define FLASH_LENGTH (FLASH_PAGE_SIZE*FLASH_PAGES)
#if FLASH_LENGTH < 8+JSVAR_CACHE_SIZE*20
#error NOT ENOUGH ROOM IN FLASH - UNLESS WE ARE ONLY USING 16 bytes forJsVarRef ? FLASH_PAGES pages at FLASH_PAGE_SIZE bytes
#endif

#define FLASH_START (0x08000000 + FLASH_MEMORY_SIZE - FLASH_LENGTH)
#define FLASH_MAGIC_LOCATION (FLASH_START+FLASH_LENGTH-8)
#define FLASH_MAGIC 0xDEADBEEF


void jshSaveToFlash() {
#ifdef ARM
#ifdef STM32F4 
  FLASH_Unlock();
#else
  FLASH_UnlockBank1();
#endif

  int i;
  /* Clear All pending flags */
#ifdef STM32F4 
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
#else
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
#endif

  jsPrint("Erasing Flash...");
#ifdef STM32F4 
  FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
#else
  /* Erase the FLASH pages */
  for(i=0;i<FLASH_PAGES;i++) {
    FLASH_ErasePage(FLASH_START + (FLASH_PAGE_SIZE * i));
    jsPrint(".");
  }
#endif
  jsPrint("\nProgramming ");
  jsPrintInt(jsvGetVarDataSize());
  jsPrint(" Bytes...");

  int *basePtr = jsvGetVarDataPointer();
#ifdef STM32F4
  for (i=0;i<jsvGetVarDataSize();i+=4) {
      while (FLASH_ProgramWord(FLASH_START+i, basePtr[i>>2]) != FLASH_COMPLETE);
      if ((i&1023)==0) jsPrint(".");
  }
  while (FLASH_ProgramWord(FLASH_MAGIC_LOCATION, FLASH_MAGIC) != FLASH_COMPLETE);
#else
  /* Program Flash Bank1 */  
  for (i=0;i<jsvGetVarDataSize();i+=4) {
      FLASH_ProgramWord(FLASH_START+i, basePtr[i>>2]);
      if ((i&1023)==0) jsPrint(".");
  }
  FLASH_ProgramWord(FLASH_MAGIC_LOCATION, FLASH_MAGIC);
  FLASH_WaitForLastOperation(0x2000);
#endif
#ifdef STM32F4 
  FLASH_Lock();
#else
  FLASH_LockBank1();
#endif
  jsPrint("\nChecking...");

  int errors = 0;
  for (i=0;i<jsvGetVarDataSize();i+=4)
    if ((*(int*)(FLASH_START+i)) != basePtr[i>>2])
      errors++;

  if (errors) {
      jsPrint("\nThere were ");
      jsPrintInt(errors);
      jsPrint(" errors!\n>");
  } else
      jsPrint("\nDone!\n>");

//  This is nicer, but also broken!
//  FLASH_UnlockBank1();
//  /* Clear All pending flags */
//  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
//
//  size_t varDataSize = jsvGetVarDataSize();
//  int *basePtr = jsvGetVarDataPointer();
//
//  int page;
//  for(page=0;page<FLASH_PAGES;page++) {
//    jsPrint("Flashing Page ");jsPrintInt(page);jsPrint("...\n");
//    size_t pageOffset = (FLASH_PAGE_SIZE * page);
//    size_t pagePtr = FLASH_START + pageOffset;
//    size_t pageSize = varDataSize-pageOffset;
//    if (pageSize>FLASH_PAGE_SIZE) pageSize = FLASH_PAGE_SIZE;
//    jsPrint("Offset ");jsPrintInt(pageOffset);jsPrint(", Size ");jsPrintInt(pageSize);jsPrint(" bytes\n");
//    bool first = true;
//    int errors = 0;
//    int i;
//    for (i=pageOffset;i<pageOffset+pageSize;i+=4)
//      if ((*(int*)(FLASH_START+i)) != basePtr[i>>2])
//        errors++;
//    while (errors && !jspIsInterrupted()) {
//      if (!first) { jsPrintInt(errors);jsPrint(" errors - retrying...\n"); }
//      first = false;
//      /* Erase the FLASH page */
//      FLASH_ErasePage(pagePtr);
//      /* Program Flash Bank1 */
//      for (i=pageOffset;i<pageOffset+pageSize;i+=4)
//          FLASH_ProgramWord(FLASH_START+i, basePtr[i>>2]);
//      FLASH_WaitForLastOperation(0x20000);
//    }
//  }
//  // finally flash magic byte
//  FLASH_ProgramWord(FLASH_MAGIC_LOCATION, FLASH_MAGIC);
//  FLASH_WaitForLastOperation(0x20000);
//  FLASH_LockBank1();
//  if (*(int*)FLASH_MAGIC_LOCATION != FLASH_MAGIC)
//    jsPrint("Flash magic word not flashed correctly!\n");
//  jsPrint("Flashing Complete\n");

  /*jsPrint("Magic contains ");
  jsPrintInt(*(int*)FLASH_MAGIC_LOCATION);
  jsPrint(" we want ");
  jsPrintInt(FLASH_MAGIC);
  jsPrint("\n");*/
#else
  FILE *f = fopen("TinyJSC.state","wb");
  if (f) {
    jsPrint("\nSaving ");
    jsPrintInt(jsvGetVarDataSize());
    jsPrint(" bytes...");
    fwrite(jsvGetVarDataPointer(),1,jsvGetVarDataSize(),f);
    fclose(f);
    jsPrint("\nDone!\n>");
  } else {
    jsPrint("\nFile Open Failed... \n>");
  }
#endif
}

void jshLoadFromFlash() {
#ifdef ARM
  jsPrint("\nLoading ");
  jsPrintInt(jsvGetVarDataSize());
  jsPrint(" bytes from flash...");
  memcpy(jsvGetVarDataPointer(), (int*)FLASH_START, jsvGetVarDataSize());
  jsPrint("\nDone!\n>");
#else
  FILE *f = fopen("TinyJSC.state","rb");
  if (f) {
    jsPrint("\nLoading ");
    jsPrintInt(jsvGetVarDataSize());
    jsPrint(" bytes...\n>");
    fread(jsvGetVarDataPointer(),1,jsvGetVarDataSize(),f);
    fclose(f);
  } else {
    jsPrint("\nFile Open Failed... \n>");
  }
#endif
}

bool jshFlashContainsCode() {
#ifdef ARM
  /*jsPrint("Magic contains ");
  jsPrintInt(*(int*)FLASH_MAGIC_LOCATION);
  jsPrint("we want");
  jsPrintInt(FLASH_MAGIC);
  jsPrint("\n");*/
  return (*(int*)FLASH_MAGIC_LOCATION) == FLASH_MAGIC;
#else
  FILE *f = fopen("TinyJSC.state","rb");
  if (f) fclose(f);
  return f!=0;
#endif
}
