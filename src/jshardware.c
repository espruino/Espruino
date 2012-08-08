/*
 * jshardware.c
 *
 *  Created on: 8 Aug 2012
 *      Author: gw
 */

#include "jshardware.h"

#ifdef ARM
#include "stm32f10x.h"
#include "peripherals/stm32f10x_adc.h"
#include "peripherals/stm32f10x_usart.h"
#include "STM32vldiscovery.h"
#else//!ARM
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <termios.h>
#endif//ARM

// ----------------------------------------------------------------------------
//                                                                     BUFFERS
#ifdef ARM
#define RXBUFFERMASK 31
char rxBuffer[RXBUFFERMASK+1];
volatile unsigned char rxHead=0, rxTail=0;

#define TXBUFFERMASK 15
char txBuffer[TXBUFFERMASK+1];
volatile unsigned char txHead=0, txTail=0;
#endif
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
 { GPIO_Pin_0, GPIOA, ADC_Channel_0 },
 { GPIO_Pin_1, GPIOA, ADC_Channel_1 },
 { GPIO_Pin_2, GPIOA, ADC_Channel_2 },
 { GPIO_Pin_3, GPIOA, ADC_Channel_3 },
 { GPIO_Pin_4, GPIOA, ADC_Channel_4 },
 { GPIO_Pin_5, GPIOA, ADC_Channel_5 },
 { GPIO_Pin_6, GPIOA, ADC_Channel_6 },
 { GPIO_Pin_7, GPIOA, ADC_Channel_7 },
 { GPIO_Pin_8, GPIOA, 0xFF },
 { GPIO_Pin_9, 0, 0xFF }, //A9 - Serial
 { GPIO_Pin_10, 0, 0xFF }, // A10 - Serial
 { GPIO_Pin_11, GPIOA, 0xFF },
 { GPIO_Pin_12, GPIOA, 0xFF },
 { GPIO_Pin_13, GPIOA, 0xFF },
 { GPIO_Pin_14, GPIOA, 0xFF },
 { GPIO_Pin_15, GPIOA, 0xFF },

 { GPIO_Pin_0, GPIOB, ADC_Channel_8 },
 { GPIO_Pin_1, GPIOB, ADC_Channel_9 },
 { GPIO_Pin_2, GPIOB, 0xFF },
 { GPIO_Pin_3, GPIOB, 0xFF },
 { GPIO_Pin_4, GPIOB, 0xFF },
 { GPIO_Pin_5, GPIOB, 0xFF },
 { GPIO_Pin_6, GPIOB, 0xFF },
 { GPIO_Pin_7, GPIOB, 0xFF },
 { GPIO_Pin_8, GPIOB, 0xFF },
 { GPIO_Pin_9, GPIOB, 0xFF },
 { GPIO_Pin_10, GPIOB, 0xFF },
 { GPIO_Pin_11, GPIOB, 0xFF },
 { GPIO_Pin_12, GPIOB, 0xFF },
 { GPIO_Pin_13, GPIOB, 0xFF },
 { GPIO_Pin_14, GPIOB, 0xFF },
 { GPIO_Pin_15, GPIOB, 0xFF },

 { GPIO_Pin_0, GPIOC, ADC_Channel_10 },
 { GPIO_Pin_1, GPIOC, ADC_Channel_11 },
 { GPIO_Pin_2, GPIOC, ADC_Channel_12 },
 { GPIO_Pin_3, GPIOC, ADC_Channel_13 },
 { GPIO_Pin_4, GPIOC, ADC_Channel_14 },
 { GPIO_Pin_5, GPIOC, ADC_Channel_15 },
 { GPIO_Pin_6, GPIOC, 0xFF },
 { GPIO_Pin_7, GPIOC, 0xFF },
 { GPIO_Pin_8, GPIOC, 0xFF },
 { GPIO_Pin_9, GPIOC, 0xFF },
 { GPIO_Pin_10, GPIOC, 0xFF },
 { GPIO_Pin_11, GPIOC, 0xFF },
 { GPIO_Pin_12, GPIOC, 0xFF },
 { GPIO_Pin_13, GPIOC, 0xFF },
 { GPIO_Pin_14, GPIOC, 0xFF },
 { GPIO_Pin_15, GPIOC, 0xFF },

 { GPIO_Pin_0, GPIOD, 0xFF },
 { GPIO_Pin_1, GPIOD, 0xFF },
};

typedef struct IOPinState {
  bool value; // the value of this pin
  JsVarRef callbacks; // an array of callbacks, or 0
} IOPinState;

IOPinState ioPinState[IOPINS]; // the state of IO pins and callbacks for if they change
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
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_USART1 |
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_GPIOC |
        RCC_APB2Periph_GPIOD |
        RCC_APB2Periph_AFIO, ENABLE);
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

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // RX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // TX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

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
  jsvUnLock(pinv);
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
  }
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
  }
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
    JsSysTime endtime = getTime() + time;
    while (jshGetSystemTime()<endtime);
    if (!value)
      IOPIN_DATA[pin].gpio->BSRR = IOPIN_DATA[pin].pin;
    else
      IOPIN_DATA[pin].gpio->BRR = IOPIN_DATA[pin].pin;
  }
#endif
}
