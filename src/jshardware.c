/*
 * jshardware.c
 *
 *  Created on: 8 Aug 2012
 *      Author: gw
 */

#ifdef ARM
 #ifdef STM32F4
  #include "stm32f4xx_conf.h" // again, added because ST didn't put it here ?
  #include "peripherals/stm32f4xx_adc.h"
  #include "peripherals/stm32f4xx_gpio.h"
  #include "peripherals/stm32f4xx_usart.h"
  #include "peripherals/stm32f4xx_flash.h"
  #include "stm32f4_discovery.h"

  #define MAIN_USART_Pin_RX GPIO_Pin_10
  #define MAIN_USART_Pin_TX GPIO_Pin_9
  #define MAIN_USART_Port GPIOA

  #define STM32vldiscovery_LEDInit STM_EVAL_LEDInit
  #define STM32vldiscovery_LEDOn STM_EVAL_LEDOn
  #define STM32vldiscovery_LEDOff STM_EVAL_LEDOff
  #define STM32vldiscovery_PBInit STM_EVAL_PBInit
  #define GPIO_Mode_IN_FLOATING GPIO_Mode_IN
  #define GPIO_Mode_AF_PP GPIO_Mode_AF
 #else
  #include "stm32f10x.h"
  #include "peripherals/stm32f10x_adc.h"
  #include "peripherals/stm32f10x_gpio.h"
  #include "peripherals/stm32f10x_usart.h"
  #include "peripherals/stm32f10x_flash.h"
  #include "STM32vldiscovery.h"

  #define MAIN_USART_Pin_RX GPIO_Pin_10
  #define MAIN_USART_Pin_TX GPIO_Pin_9
  #define MAIN_USART_Port GPIOA
 #endif
#else//!ARM
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <termios.h>
#endif//ARM

#include "jshardware.h"
#include "jsutils.h"

// ----------------------------------------------------------------------------
//                                                                     BUFFERS
#ifdef ARM
// UART Receive
#define RXBUFFERMASK 31
char rxBuffer[RXBUFFERMASK+1];
volatile unsigned char rxHead=0, rxTail=0;

// UART Transmit
#define TXBUFFERMASK 15
char txBuffer[TXBUFFERMASK+1];
volatile unsigned char txHead=0, txTail=0;

#endif

// IO Events
#define IOBUFFERMASK 7
IOEvent ioBuffer[IOBUFFERMASK+1];
volatile unsigned char ioHead=0, ioTail=0;

void jshPushIOEvent(JsSysTime time, unsigned char channel) {
  unsigned char nextHead = (ioHead+1) & IOBUFFERMASK;
  if (ioTail == nextHead) return; // queue full - dump this event!
  ioBuffer[ioHead].time = time;
  ioBuffer[ioHead].channel = channel;
  ioHead = nextHead;
}

// returns true on success
bool jshPopIOEvent(IOEvent *result) {
  if (ioHead==ioTail) return false;
  *result = ioBuffer[ioTail];
  ioTail = (ioTail+1) & IOBUFFERMASK;
  return true;
}

// ----------------------------------------------------------------------------
//                                                                        PINS
#ifdef ARM
typedef struct IOPin {
  uint16_t pin;      // GPIO_Pin_1
  GPIO_TypeDef *gpio; // GPIOA
  uint8_t adc; // 0xFF or ADC_Channel_1
} IOPin;

#define IOPINS 50 // 16*3+2
const IOPin IOPIN_DATA[IOPINS] = {
 { GPIO_Pin_0,  GPIOA, ADC_Channel_0 },
 { GPIO_Pin_1,  GPIOA, ADC_Channel_1 },
 { GPIO_Pin_2,  GPIOA, ADC_Channel_2 },
 { GPIO_Pin_3,  GPIOA, ADC_Channel_3 },
 { GPIO_Pin_4,  GPIOA, ADC_Channel_4 },
 { GPIO_Pin_5,  GPIOA, ADC_Channel_5 },
 { GPIO_Pin_6,  GPIOA, ADC_Channel_6 },
 { GPIO_Pin_7,  GPIOA, ADC_Channel_7 },
 { GPIO_Pin_8,  GPIOA, 0xFF },
 { GPIO_Pin_9,  0,     0xFF }, //A9 - Serial
 { GPIO_Pin_10, 0,     0xFF }, // A10 - Serial
 { GPIO_Pin_11, GPIOA, 0xFF },
 { GPIO_Pin_12, GPIOA, 0xFF },
 { GPIO_Pin_13, GPIOA, 0xFF },
 { GPIO_Pin_14, GPIOA, 0xFF },
 { GPIO_Pin_15, GPIOA, 0xFF },

 { GPIO_Pin_0,  GPIOB, ADC_Channel_8 },
 { GPIO_Pin_1,  GPIOB, ADC_Channel_9 },
 { GPIO_Pin_2,  GPIOB, 0xFF },
 { GPIO_Pin_3,  GPIOB, 0xFF },
 { GPIO_Pin_4,  GPIOB, 0xFF },
 { GPIO_Pin_5,  GPIOB, 0xFF },
 { GPIO_Pin_6,  GPIOB, 0xFF },
 { GPIO_Pin_7,  GPIOB, 0xFF },
 { GPIO_Pin_8,  GPIOB, 0xFF },
 { GPIO_Pin_9,  GPIOB, 0xFF },
 { GPIO_Pin_10, GPIOB, 0xFF },
 { GPIO_Pin_11, GPIOB, 0xFF },
 { GPIO_Pin_12, GPIOB, 0xFF },
 { GPIO_Pin_13, GPIOB, 0xFF },
 { GPIO_Pin_14, GPIOB, 0xFF },
 { GPIO_Pin_15, GPIOB, 0xFF },

 { GPIO_Pin_0,  GPIOC, ADC_Channel_10 },
 { GPIO_Pin_1,  GPIOC, ADC_Channel_11 },
 { GPIO_Pin_2,  GPIOC, ADC_Channel_12 },
 { GPIO_Pin_3,  GPIOC, ADC_Channel_13 },
 { GPIO_Pin_4,  GPIOC, ADC_Channel_14 },
 { GPIO_Pin_5,  GPIOC, ADC_Channel_15 },
 { GPIO_Pin_6,  GPIOC, 0xFF },
 { GPIO_Pin_7,  GPIOC, 0xFF },
 { GPIO_Pin_8,  GPIOC, 0xFF },
 { GPIO_Pin_9,  GPIOC, 0xFF },
 { GPIO_Pin_10, GPIOC, 0xFF },
 { GPIO_Pin_11, GPIOC, 0xFF },
 { GPIO_Pin_12, GPIOC, 0xFF },
 { GPIO_Pin_13, GPIOC, 0xFF },
 { GPIO_Pin_14, GPIOC, 0xFF },
 { GPIO_Pin_15, GPIOC, 0xFF },

 { GPIO_Pin_0,  GPIOD, 0xFF },
 { GPIO_Pin_1,  GPIOD, 0xFF },
};

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
uint8_t portToPortSource(GPIO_TypeDef *port) {
 #ifdef STM32F4
  if (port == GPIOA) return EXTI_PortSourceGPIOA;
  if (port == GPIOB) return EXTI_PortSourceGPIOB;
  if (port == GPIOC) return EXTI_PortSourceGPIOC;
  if (port == GPIOD) return EXTI_PortSourceGPIOD;
  if (port == GPIOE) return EXTI_PortSourceGPIOE;
  if (port == GPIOF) return EXTI_PortSourceGPIOF;
  if (port == GPIOG) return EXTI_PortSourceGPIOG;
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
JsSysTime SysTickMajor = 0;
void SysTick_Handler(void) {
  SysTickMajor+=0x1000000;
}

void USART1_IRQHandler(void) {
 if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
    /* Clear the USART Receive interrupt */
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    /* Read one byte from the receive data register */
    rxBuffer[rxHead] = USART_ReceiveData(USART1);
    rxHead = (rxHead+1)&RXBUFFERMASK;
  }
  if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
    /* Clear the USART Transmit interrupt */
    //USART_ClearITPendingBit(USART1, USART_IT_TXE);
    /* If we have other data to send, send it */
    if (txHead != txTail) {
      USART_SendData(USART1, txBuffer[txTail]);
      txTail = (txTail+1)&TXBUFFERMASK;
    } else
      USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
  }
}

void EXTI0_IRQHandler(void) {
  if (EXTI_GetITStatus(EXTI_Line0) == SET) {
    jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource0);
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
  // repeat for EXTI1 etc...
}
void EXTI1_IRQHandler(void) {
  if (EXTI_GetITStatus(EXTI_Line1) == SET) {
    jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource1);
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}
void EXTI2_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line2) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource2);
      EXTI_ClearITPendingBit(EXTI_Line2);
    }
}
void EXTI3_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line3) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource3);
      EXTI_ClearITPendingBit(EXTI_Line3);
    }
}
void EXTI4_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line4) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource4);
      EXTI_ClearITPendingBit(EXTI_Line4);
    }
}
void EXTI9_5_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line5) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource5);
      EXTI_ClearITPendingBit(EXTI_Line5);
    }
    if (EXTI_GetITStatus(EXTI_Line6) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource6);
      EXTI_ClearITPendingBit(EXTI_Line6);
    }
    if (EXTI_GetITStatus(EXTI_Line7) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource7);
      EXTI_ClearITPendingBit(EXTI_Line7);
    }
    if (EXTI_GetITStatus(EXTI_Line8) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource8);
      EXTI_ClearITPendingBit(EXTI_Line8);
    }
    if (EXTI_GetITStatus(EXTI_Line9) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource9);
      EXTI_ClearITPendingBit(EXTI_Line9);
    }
}
void EXTI15_10_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line10) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource10);
      EXTI_ClearITPendingBit(EXTI_Line10);
    }
    if (EXTI_GetITStatus(EXTI_Line11) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource11);
      EXTI_ClearITPendingBit(EXTI_Line11);
    }
    if (EXTI_GetITStatus(EXTI_Line12) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource12);
      EXTI_ClearITPendingBit(EXTI_Line12);
    }
    if (EXTI_GetITStatus(EXTI_Line13) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource13);
      EXTI_ClearITPendingBit(EXTI_Line13);
    }
    if (EXTI_GetITStatus(EXTI_Line14) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource14);
      EXTI_ClearITPendingBit(EXTI_Line14);
    }
    if (EXTI_GetITStatus(EXTI_Line15) == SET) {
      jshPushIOEvent(jshGetSystemTime(), GPIO_PinSource15);
      EXTI_ClearITPendingBit(EXTI_Line15);
    }
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
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 |
                         RCC_APB2Periph_USART1, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |
                         RCC_AHB1Periph_GPIOB |
                         RCC_AHB1Periph_GPIOC | 
                         RCC_AHB1Periph_GPIOD |
                         RCC_AHB1Periph_GPIOE |
                         RCC_AHB1Periph_GPIOF |
                         RCC_AHB1Periph_GPIOG, ENABLE);
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
  SysTick_Config(0xFFFFFF); // 24 bit
  /* Initialise LEDs LD3&LD4, both on */
  STM32vldiscovery_LEDInit(LED3);
  STM32vldiscovery_LEDInit(LED4);
  STM32vldiscovery_PBInit(BUTTON_USER, BUTTON_MODE_GPIO); // See STM32vldiscovery.c (MODE_EXTI) to see how to set up interrupts!
  STM32vldiscovery_LEDOn(LED3);
  STM32vldiscovery_LEDOn(LED4);
  volatile int cnt; for (cnt=0;cnt<10000;cnt++); // small delay
  STM32vldiscovery_LEDOff(LED3);

  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  USART_ClockInitTypeDef USART_ClockInitStructure;


  GPIO_InitStructure.GPIO_Pin = MAIN_USART_Pin_RX; // RX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(MAIN_USART_Port, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // TX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(MAIN_USART_Port, &GPIO_InitStructure);

  USART_ClockStructInit(&USART_ClockInitStructure);
  USART_ClockInit(USART1, &USART_ClockInitStructure);
  USART_InitStructure.USART_BaudRate = 9600;//38400;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  //Write USART1 parameters
  USART_Init(USART1, &USART_InitStructure);

  // enable uart interrupt
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );
  // Enable RX interrupt (TX is done when we have bytes)
  USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  //Enable USART1
  USART_Cmd(USART1, ENABLE);

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

  // ADC Structure Initialization
  ADC_InitTypeDef ADC_InitStructure;
  ADC_StructInit(&ADC_InitStructure);

  // Preinit
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Left;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  // Enable the ADC
  ADC_Cmd(ADC1, ENABLE);

  // Calibrate
  ADC_ResetCalibration(ADC1);
  while(ADC_GetResetCalibrationStatus(ADC1));
  ADC_StartCalibration(ADC1);
  while(ADC_GetCalibrationStatus(ADC1));

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


int jshRX() {
#ifdef ARM
  if (rxHead == rxTail) return -1;
  char ch = rxBuffer[rxTail];
  rxTail = (rxTail+1) & RXBUFFERMASK;
  return ch;
#else
  if (!kbhit()) return -1;
  return getch();
#endif
}

void jshTX(char data) {
#ifdef ARM
  unsigned char txHeadNext = (txHead+1)&TXBUFFERMASK;
  while (txHeadNext==txTail) ; // wait for send to finish as buffer is about to overflow
  txBuffer[txHead] = (char)data;
  txHead = txHeadNext;
  USART_ITConfig(USART1, USART_IT_TXE, ENABLE); // enable interrupt -> start transmission
#else
  fputc(data, stdout);
  fflush(stdout);
#endif
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

JsSysTime jshGetSystemTime() {
#ifdef ARM
  return SysTickMajor - (JsSysTime)SysTick->VAL;
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
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
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
    GPIO_Init(IOPIN_DATA[pin].gpio, &GPIO_InitStructure);

    // Configure chanel
    ADC_RegularChannelConfig(ADC1, IOPIN_DATA[pin].adc, 1, ADC_SampleTime_239Cycles5/*ADC_SampleTime_55Cycles5*/);

    // Start the conversion
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    // Wait until conversion completion
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

    // Get the conversion value
    value = ADC_GetConversionValue(ADC1) / (JsVarFloat)65535;
  } else jsError("Invalid analog pin!");
#endif
  return value;
}

void jshPinOutput(int pin, bool value) {
#ifdef ARM
  if (pin>=0 && pin < IOPINS && IOPIN_DATA[pin].gpio) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = IOPIN_DATA[pin].pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(IOPIN_DATA[pin].gpio, &GPIO_InitStructure);

    if (value)
      IOPIN_DATA[pin].gpio->BSRR = IOPIN_DATA[pin].pin;
    else
      IOPIN_DATA[pin].gpio->BRR = IOPIN_DATA[pin].pin;
  } else jsError("Invalid pin!");
#endif
}

void jshPinPulse(int pin, bool value, JsVarFloat time) {
 JsSysTime ticks = jshGetTimeFromMilliseconds(time);
#ifdef ARM
  if (pin>=0 && pin < IOPINS && IOPIN_DATA[pin].gpio) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = IOPIN_DATA[pin].pin;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(IOPIN_DATA[pin].gpio, &GPIO_InitStructure);


    if (value)
      IOPIN_DATA[pin].gpio->BSRR = IOPIN_DATA[pin].pin;
    else
      IOPIN_DATA[pin].gpio->BRR = IOPIN_DATA[pin].pin;
    JsSysTime endtime = jshGetSystemTime() + time;
    while (jshGetSystemTime()<endtime);
    if (!value)
      IOPIN_DATA[pin].gpio->BSRR = IOPIN_DATA[pin].pin;
    else
      IOPIN_DATA[pin].gpio->BRR = IOPIN_DATA[pin].pin;
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
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(IOPIN_DATA[pin].gpio, &GPIO_InitStructure);

    GPIO_EXTILineConfig(portToPortSource(IOPIN_DATA[pin].gpio), pinToPinSource(IOPIN_DATA[pin].pin));

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
  return event->channel == pinToPinSource(IOPIN_DATA[pin].pin);
#else
  return false;
#endif
}


#define FLASH_MEMORY_SIZE (128*1024)
#define FLASH_PAGE_SIZE 1024
#define FLASH_PAGES 4
#define FLASH_LENGTH (1024*FLASH_PAGES)
#define FLASH_START (0x08000000 + FLASH_MEMORY_SIZE - FLASH_LENGTH)
#define FLASH_MAGIC_LOCATION (FLASH_START+FLASH_LENGTH-8)
#define FLASH_MAGIC 0xDEADBEEF

void jshSaveToFlash() {
#ifdef ARM
  FLASH_UnlockBank1();

  int i;
  /* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

  jsPrint("Erasing Flash...");
  /* Erase the FLASH pages */
  for(i=0;i<FLASH_PAGES;i++) {
    FLASH_ErasePage(FLASH_START + (FLASH_PAGE_SIZE * i));
    jsPrint(".");
  }
  jsPrint("\nProgramming ");
  jsPrintInt(jsvGetVarDataSize());
  jsPrint(" Bytes...");

  /* Program Flash Bank1 */
  int *basePtr = jsvGetVarDataPointer();
  for (i=0;i<jsvGetVarDataSize();i+=4) {
      FLASH_ProgramWord(FLASH_START+i, basePtr[i>>2]);
      if ((i&(FLASH_PAGE_SIZE-1))==0) jsPrint(".");
  }
  FLASH_ProgramWord(FLASH_MAGIC_LOCATION, FLASH_MAGIC);
  FLASH_WaitForLastOperation(0x2000);
  FLASH_LockBank1();
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
