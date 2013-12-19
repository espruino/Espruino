/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Graphics Backend for 16 bit parallel LCDs (ILI9325 and similar)
 *
 * Loosely based on example code that comes with 'HY' branded STM32 boards,
 * original Licence unknown.
 * ----------------------------------------------------------------------------
 */

#include "platform_config.h"
#include "jshardware.h"
#include "jsinteractive.h" // for debug
#include "graphics.h"

/*
const unsigned int DELAY_LONG = 0xAFFFFf;
const unsigned int DELAY_SHORT = 10;*/
const unsigned int DELAY_LONG = 0xFFFFF;
const unsigned int DELAY_SHORT = 10;


void LCD_DELAY(__IO uint32_t nCount) {
  for(; nCount != 0; nCount--) ;//n++;
}

void delay_ms(__IO uint32_t mSec) {
  mSec *= 10000;
  for(; mSec != 0; mSec--) ;//n++;
}

static uint8_t LCD_Code;
#define  ILI9320    0  /* 0x9320 */
#define  ILI9325    1  /* 0x9325 */
#define  ILI9328    2  /* 0x9328 */
#define  ILI9331    3  /* 0x9331 */
#define  SSD1298    4  /* 0x8999 */
#define  SSD1289    5  /* 0x8989 */
#define  ST7781     6  /* 0x7783 */
#define  LGDP4531   7  /* 0x4531 */
#define  SPFD5408B  8  /* 0x5408 */
#define  R61505U    9  /* 0x1505 0x0505 */
#define  HX8346A    10 /* 0x0046 */  
#define  HX8347D    11 /* 0x0047 */
#define  HX8347A    12 /* 0x0047 */	
#define  LGDP4535   13 /* 0x4535 */  
#define  SSD2119    14 /* 3.5 LCD 0x9919 */


static inline void LCD_WR_CMD(unsigned int index,unsigned int val);
static inline unsigned int LCD_RD_CMD(unsigned int index);

#ifdef ILI9325_BITBANG
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr))
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C
#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)

#define LCD_CS  PCout(8)
#define LCD_RS  PCout(9)
#define LCD_WR  PCout(10)
#define LCD_RD  PCout(11)

static inline void LCD_WR_REG(unsigned int index) {
  LCD_CS = 0;
  LCD_RS = 0;
  GPIOC->ODR = (GPIOC->ODR&0xff00)|(index&0x00ff);
  GPIOB->ODR = (GPIOB->ODR&0x00ff)|(index&0xff00);
  LCD_WR = 0;
  LCD_WR = 1;
  LCD_CS = 1;
}

static inline unsigned int LCD_RD_Data(void) {
  uint16_t temp;

  GPIOB->CRH = (GPIOB->CRH & 0x00000000) | 0x44444444;
  GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x44444444;
  LCD_CS = 0;
  LCD_RS = 1;
  LCD_RD = 0;
  temp = ((GPIOB->IDR&0xff00)|(GPIOC->IDR&0x00ff));
  LCD_RD = 1;
  LCD_CS = 1;
  GPIOB->CRH = (GPIOB->CRH & 0x00000000) | 0x33333333;
  GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x33333333;

  return temp;
}

static inline void LCD_WR_Data(unsigned int val) {
  LCD_CS = 0;
  LCD_RS = 1;
  GPIOC->ODR = (GPIOC->ODR&0xff00)|(val&0x00ff);
  GPIOB->ODR = (GPIOB->ODR&0x00ff)|(val&0xff00);
  LCD_WR = 0;
  LCD_WR = 1;
  LCD_CS = 1;
}

static inline void LCD_WR_Data_multi(unsigned int val, unsigned int count) {
  LCD_CS = 0;
  LCD_RS = 1;
  GPIOC->ODR = (GPIOC->ODR&0xff00)|(val&0x00ff);
  GPIOB->ODR = (GPIOB->ODR&0x00ff)|(val&0xff00);
  int i;
  for (i==0;i<count;i++) {
    LCD_WR = 0;
    LCD_WR = 1;
  }
  LCD_CS = 1;
}

void LCD_init_hardware() {
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC,ENABLE);

  /* ÅäÖÃÊýŸÝIO Á¬œÓµœGPIOB *********************/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11
                              | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   // ÍÆÍìÊä³ö·œÊœ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // Êä³öIO¿Ú×îŽó×îËÙÎª50MHZ
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* ÅäÖÃ¿ØÖÆIO Á¬œÓµœPD12.PD13.PD14.PD15 *********************/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3
                              | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7
                              | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   // ÍÆÍìÊä³ö·œÊœ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // Êä³öIO¿Ú×îŽó×îËÙÎª50MHZ
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}

#else // NOT ILI9325_BITBANG ------------------------------------------------------------------------------------------------

#if defined(HYSTM32_24)
  #define LCD_RESET (Pin)(JSH_PORTE_OFFSET + 1)
#elif defined(HYSTM32_32)
#else
  #error Unsupported board for ILI9325 LCD 
#endif

#define LCD_REG              (*((volatile unsigned short *) 0x60000000)) /* RS = 0 */
#define LCD_RAM              (*((volatile unsigned short *) 0x60020000)) /* RS = 1 */


static inline void LCD_WR_REG(unsigned int index) {
  LCD_REG = (uint16_t)index;
}

static inline unsigned int LCD_RD_Data(void) {
    return LCD_RAM;
}

static inline void LCD_WR_Data(unsigned int val) {
  LCD_RAM = (uint16_t)val;
}

static inline void LCD_WR_Data_multi(unsigned int val, unsigned int count) {
  int i;
  for (i=0;i<count;i++)
    LCD_RAM = (uint16_t)val;
}


void LCD_init_hardware() {
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE); /* Enable the FSMC Clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
                   RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE , ENABLE);

  /* Enable the FSMC pins for LCD control */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | 
                          GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 |
                          GPIO_Pin_15 | GPIO_Pin_7 /*NE1*/ |  GPIO_Pin_11/*RS*/;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
                          GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
                          GPIO_Pin_15;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  p;
  p.FSMC_AddressSetupTime = 0x02;
  p.FSMC_AddressHoldTime = 0x00;
  p.FSMC_DataSetupTime = 0x05;
  p.FSMC_BusTurnAroundDuration = 0x00;
  p.FSMC_CLKDivision = 0x00;
  p.FSMC_DataLatency = 0x00;
  p.FSMC_AccessMode = FSMC_AccessMode_B;
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;

  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;
  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

  /* Enable FSMC Bank1_SRAM Bank */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);

  // Toggle LCD reset pin
#ifdef LCD_RESET
  jshPinSetState(LCD_RESET, JSHPINSTATE_GPIO_OUT);
  jshPinSetValue(LCD_RESET, 0); //RESET=0
  LCD_DELAY(DELAY_LONG);
  jshPinSetValue(LCD_RESET, 1); //RESET=1
#endif
}

#endif // NOT ILI9325_BITBANG

static inline void LCD_WR_CMD(unsigned int index,unsigned int val) {
  LCD_WR_REG(index);
  LCD_WR_Data(val);
}

static inline unsigned int LCD_RD_CMD(unsigned int index) {
  LCD_WR_REG(index);
  return LCD_RD_Data();
}

void LCD_init_panel() {
	uint16_t DeviceCode;
	delay_ms(100);
	DeviceCode = LCD_RD_CMD(0x0000);	
//        jsiConsolePrintf("LCDA %d\n", DeviceCode);

        if (DeviceCode == 0x4532) { // For the 2.4" LCD boards
                LCD_Code = ILI9325;
                LCD_WR_CMD(0x0000,0x0001);
                LCD_DELAY(DELAY_SHORT);

                LCD_WR_CMD(0x0015,0x0030);
                LCD_WR_CMD(0x0011,0x0040);
                LCD_WR_CMD(0x0010,0x1628);
                LCD_WR_CMD(0x0012,0x0000);
                LCD_WR_CMD(0x0013,0x104d);
                LCD_DELAY(DELAY_SHORT);
                LCD_WR_CMD(0x0012,0x0010);
                LCD_DELAY(DELAY_SHORT);
                LCD_WR_CMD(0x0010,0x2620);
                LCD_WR_CMD(0x0013,0x344d); //304d
                LCD_DELAY(DELAY_SHORT);

                LCD_WR_CMD(0x0001,0x0100);
                LCD_WR_CMD(0x0002,0x0300);
                LCD_WR_CMD(0x0003,0x1030); // ORG is 0
                LCD_WR_CMD(0x0008,0x0604);
                LCD_WR_CMD(0x0009,0x0000);
                LCD_WR_CMD(0x000A,0x0008);

                LCD_WR_CMD(0x0041,0x0002);
                LCD_WR_CMD(0x0060,0x2700);
                LCD_WR_CMD(0x0061,0x0001);
                LCD_WR_CMD(0x0090,0x0182);
                LCD_WR_CMD(0x0093,0x0001);
                LCD_WR_CMD(0x00a3,0x0010);
                LCD_DELAY(DELAY_SHORT);

                //################# void Gamma_Set(void) ####################//
                LCD_WR_CMD(0x30,0x0000);
                LCD_WR_CMD(0x31,0x0502);
                LCD_WR_CMD(0x32,0x0307);
                LCD_WR_CMD(0x33,0x0305);
                LCD_WR_CMD(0x34,0x0004);
                LCD_WR_CMD(0x35,0x0402);
                LCD_WR_CMD(0x36,0x0707);
                LCD_WR_CMD(0x37,0x0503);
                LCD_WR_CMD(0x38,0x1505);
                LCD_WR_CMD(0x39,0x1505);
                LCD_DELAY(DELAY_SHORT);

                //################## void Display_ON(void) ####################//
                LCD_WR_CMD(0x0007,0x0001);
                LCD_DELAY(DELAY_SHORT);
                LCD_WR_CMD(0x0007,0x0021);
                LCD_WR_CMD(0x0007,0x0023);
                LCD_DELAY(DELAY_SHORT);
                LCD_WR_CMD(0x0007,0x0033);
                LCD_DELAY(DELAY_SHORT);
                LCD_WR_CMD(0x0007,0x0133);
        }
	else if( DeviceCode == 0x9325 || DeviceCode == 0x9328 )	
	{
		LCD_Code = ILI9325;
		LCD_WR_CMD(0x00e7,0x0010);      
		LCD_WR_CMD(0x0000,0x0001);  	/* start internal osc */
		LCD_WR_CMD(0x0001,0x0100);     
		LCD_WR_CMD(0x0002,0x0700); 	/* power on sequence */
		LCD_WR_CMD(0x0003,(1<<12)|(1<<5)|(1<<4)|(0<<3) ); 	/* importance */
		LCD_WR_CMD(0x0004,0x0000);                                   
		LCD_WR_CMD(0x0008,0x0207);	           
		LCD_WR_CMD(0x0009,0x0000);         
		LCD_WR_CMD(0x000a,0x0000); 	/* display setting */        
		LCD_WR_CMD(0x000c,0x0001);	/* display setting */        
		LCD_WR_CMD(0x000d,0x0000); 			        
		LCD_WR_CMD(0x000f,0x0000);
		/* Power On sequence */
		LCD_WR_CMD(0x0010,0x0000);   
		LCD_WR_CMD(0x0011,0x0007);
		LCD_WR_CMD(0x0012,0x0000);                                                                 
		LCD_WR_CMD(0x0013,0x0000);                 
		delay_ms(50);  /* delay 50 ms */		
		LCD_WR_CMD(0x0010,0x1590);   
		LCD_WR_CMD(0x0011,0x0227);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WR_CMD(0x0012,0x009c);                  
		delay_ms(50);  /* delay 50 ms */		
		LCD_WR_CMD(0x0013,0x1900);   
		LCD_WR_CMD(0x0029,0x0023);
		LCD_WR_CMD(0x002b,0x000e);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WR_CMD(0x0020,0x0000);                                                            
		LCD_WR_CMD(0x0021,0x0000);           
		delay_ms(50);  /* delay 50 ms */		
		LCD_WR_CMD(0x0030,0x0007); 
		LCD_WR_CMD(0x0031,0x0707);   
		LCD_WR_CMD(0x0032,0x0006);
		LCD_WR_CMD(0x0035,0x0704);
		LCD_WR_CMD(0x0036,0x1f04); 
		LCD_WR_CMD(0x0037,0x0004);
		LCD_WR_CMD(0x0038,0x0000);        
		LCD_WR_CMD(0x0039,0x0706);     
		LCD_WR_CMD(0x003c,0x0701);
		LCD_WR_CMD(0x003d,0x000f);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WR_CMD(0x0050,0x0000);        
		LCD_WR_CMD(0x0051,0x00ef);   
		LCD_WR_CMD(0x0052,0x0000);     
		LCD_WR_CMD(0x0053,0x013f);
		LCD_WR_CMD(0x0060,0xa700);        
		LCD_WR_CMD(0x0061,0x0001); 
		LCD_WR_CMD(0x006a,0x0000);
		LCD_WR_CMD(0x0080,0x0000);
		LCD_WR_CMD(0x0081,0x0000);
		LCD_WR_CMD(0x0082,0x0000);
		LCD_WR_CMD(0x0083,0x0000);
		LCD_WR_CMD(0x0084,0x0000);
		LCD_WR_CMD(0x0085,0x0000);
		  
		LCD_WR_CMD(0x0090,0x0010);     
		LCD_WR_CMD(0x0092,0x0000);  
		LCD_WR_CMD(0x0093,0x0003);
		LCD_WR_CMD(0x0095,0x0110);
		LCD_WR_CMD(0x0097,0x0000);        
		LCD_WR_CMD(0x0098,0x0000);  
		/* display on sequence */    
		LCD_WR_CMD(0x0007,0x0133);
		
		LCD_WR_CMD(0x0020,0x0000);  /* ÐÐÊ×Ö·0 */                                                          
		LCD_WR_CMD(0x0021,0x0000);  /* ÁÐÊ×Ö·0 */     
	}
	else if( DeviceCode == 0x9320 || DeviceCode == 0x9300 )
	{
	    LCD_Code = ILI9320;
	    LCD_WR_CMD(0x00,0x0000);
		LCD_WR_CMD(0x01,0x0100);	/* Driver Output Contral */
		LCD_WR_CMD(0x02,0x0700);	/* LCD Driver Waveform Contral */
		LCD_WR_CMD(0x03,0x1018);	/* Entry Mode Set */
		
		LCD_WR_CMD(0x04,0x0000);	/* Scalling Contral */
	    LCD_WR_CMD(0x08,0x0202);	/* Display Contral */
		LCD_WR_CMD(0x09,0x0000);	/* Display Contral 3.(0x0000) */
		LCD_WR_CMD(0x0a,0x0000);	/* Frame Cycle Contal.(0x0000) */
	    LCD_WR_CMD(0x0c,(1<<0));	/* Extern Display Interface Contral */
		LCD_WR_CMD(0x0d,0x0000);	/* Frame Maker Position */
		LCD_WR_CMD(0x0f,0x0000);	/* Extern Display Interface Contral 2. */
		
	    delay_ms(100);  /* delay 100 ms */		
		LCD_WR_CMD(0x07,0x0101);	/* Display Contral */
	    delay_ms(100);  /* delay 100 ms */		
	
		LCD_WR_CMD(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	/* Power Control 1.(0x16b0)	*/
		LCD_WR_CMD(0x11,0x0007);								/* Power Control 2 */
		LCD_WR_CMD(0x12,(1<<8)|(1<<4)|(0<<0));				/* Power Control 3.(0x0138)	*/
		LCD_WR_CMD(0x13,0x0b00);								/* Power Control 4 */
		LCD_WR_CMD(0x29,0x0000);								/* Power Control 7 */
		
		LCD_WR_CMD(0x2b,(1<<14)|(1<<4));
			
		LCD_WR_CMD(0x50,0);       /* Set X Start */
		LCD_WR_CMD(0x51,239);	    /* Set X End */
		LCD_WR_CMD(0x52,0);	    /* Set Y Start */
		LCD_WR_CMD(0x53,319);	    /* Set Y End */
		
		LCD_WR_CMD(0x60,0x2700);	/* Driver Output Control */
		LCD_WR_CMD(0x61,0x0001);	/* Driver Output Control */
		LCD_WR_CMD(0x6a,0x0000);	/* Vertical Srcoll Control */
		
		LCD_WR_CMD(0x80,0x0000);	/* Display Position? Partial Display 1 */
		LCD_WR_CMD(0x81,0x0000);	/* RAM Address Start? Partial Display 1 */
		LCD_WR_CMD(0x82,0x0000);	/* RAM Address End-Partial Display 1 */
		LCD_WR_CMD(0x83,0x0000);	/* Displsy Position? Partial Display 2 */
		LCD_WR_CMD(0x84,0x0000);	/* RAM Address Start? Partial Display 2 */
		LCD_WR_CMD(0x85,0x0000);	/* RAM Address End? Partial Display 2 */
		
	    LCD_WR_CMD(0x90,(0<<7)|(16<<0));	/* Frame Cycle Contral.(0x0013)	*/
		LCD_WR_CMD(0x92,0x0000);	/* Panel Interface Contral 2.(0x0000) */
		LCD_WR_CMD(0x93,0x0001);	/* Panel Interface Contral 3. */
	    LCD_WR_CMD(0x95,0x0110);	/* Frame Cycle Contral.(0x0110)	*/
		LCD_WR_CMD(0x97,(0<<8));	
		LCD_WR_CMD(0x98,0x0000);	/* Frame Cycle Contral */
	
	    LCD_WR_CMD(0x07,0x0173);
	}
#ifndef SAVE_ON_FLASH
	else if( DeviceCode == 0x9331 )
	{
	    LCD_Code = ILI9331;
		LCD_WR_CMD(0x00E7, 0x1014);
		LCD_WR_CMD(0x0001, 0x0100);   /* set SS and SM bit */
		LCD_WR_CMD(0x0002, 0x0200);   /* set 1 line inversion */
		LCD_WR_CMD(0x0003, 0x1030);   /* set GRAM write direction and BGR=1 */
		LCD_WR_CMD(0x0008, 0x0202);   /* set the back porch and front porch */
	    LCD_WR_CMD(0x0009, 0x0000);   /* set non-display area refresh cycle ISC[3:0] */
		LCD_WR_CMD(0x000A, 0x0000);   /* FMARK function */
		LCD_WR_CMD(0x000C, 0x0000);   /* RGB interface setting */
		LCD_WR_CMD(0x000D, 0x0000);   /* Frame marker Position */
		LCD_WR_CMD(0x000F, 0x0000);   /* RGB interface polarity */
		/* Power On sequence */
		LCD_WR_CMD(0x0010, 0x0000);   /* SAP, BT[3:0], AP, DSTB, SLP, STB	*/
		LCD_WR_CMD(0x0011, 0x0007);   /* DC1[2:0], DC0[2:0], VC[2:0] */
		LCD_WR_CMD(0x0012, 0x0000);   /* VREG1OUT voltage	*/
		LCD_WR_CMD(0x0013, 0x0000);   /* VDV[4:0] for VCOM amplitude */
	    delay_ms(200);  /* delay 200 ms */		
		LCD_WR_CMD(0x0010, 0x1690);   /* SAP, BT[3:0], AP, DSTB, SLP, STB	*/
		LCD_WR_CMD(0x0011, 0x0227);   /* DC1[2:0], DC0[2:0], VC[2:0] */
	    delay_ms(50);  /* delay 50 ms */		
		LCD_WR_CMD(0x0012, 0x000C);   /* Internal reference voltage= Vci	*/
	    delay_ms(50);  /* delay 50 ms */		
		LCD_WR_CMD(0x0013, 0x0800);   /* Set VDV[4:0] for VCOM amplitude */
		LCD_WR_CMD(0x0029, 0x0011);   /* Set VCM[5:0] for VCOMH */
		LCD_WR_CMD(0x002B, 0x000B);   /* Set Frame Rate */
	    delay_ms(50);  /* delay 50 ms */		
		LCD_WR_CMD(0x0020, 0x0000);   /* GRAM horizontal Address */
		LCD_WR_CMD(0x0021, 0x0000);   /* GRAM Vertical Address */
		/* Adjust the Gamma Curve */
		LCD_WR_CMD(0x0030, 0x0000);
		LCD_WR_CMD(0x0031, 0x0106);
		LCD_WR_CMD(0x0032, 0x0000);
		LCD_WR_CMD(0x0035, 0x0204);
		LCD_WR_CMD(0x0036, 0x160A);
		LCD_WR_CMD(0x0037, 0x0707);
		LCD_WR_CMD(0x0038, 0x0106);
		LCD_WR_CMD(0x0039, 0x0707);
		LCD_WR_CMD(0x003C, 0x0402);
		LCD_WR_CMD(0x003D, 0x0C0F);
		/* Set GRAM area */
		LCD_WR_CMD(0x0050, 0x0000);   /* Horizontal GRAM Start Address */
		LCD_WR_CMD(0x0051, 0x00EF);   /* Horizontal GRAM End Address */
		LCD_WR_CMD(0x0052, 0x0000);   /* Vertical GRAM Start Address */
		LCD_WR_CMD(0x0053, 0x013F);   /* Vertical GRAM Start Address */
		LCD_WR_CMD(0x0060, 0x2700);   /* Gate Scan Line */
		LCD_WR_CMD(0x0061, 0x0001);   /*  NDL,VLE, REV */
		LCD_WR_CMD(0x006A, 0x0000);   /* set scrolling line */
		/* Partial Display Control */
		LCD_WR_CMD(0x0080, 0x0000);
		LCD_WR_CMD(0x0081, 0x0000);
		LCD_WR_CMD(0x0082, 0x0000);
		LCD_WR_CMD(0x0083, 0x0000);
		LCD_WR_CMD(0x0084, 0x0000);
		LCD_WR_CMD(0x0085, 0x0000);
		/* Panel Control */
		LCD_WR_CMD(0x0090, 0x0010);
		LCD_WR_CMD(0x0092, 0x0600);
		LCD_WR_CMD(0x0007,0x0021);		
	    delay_ms(50);  /* delay 50 ms */		
		LCD_WR_CMD(0x0007,0x0061);
	    delay_ms(50);  /* delay 50 ms */		
		LCD_WR_CMD(0x0007,0x0133);    /* 262K color and display ON */
	}
	else if( DeviceCode == 0x9919 )
	{
	    LCD_Code = SSD2119;
	    /* POWER ON &RESET DISPLAY OFF */
		LCD_WR_CMD(0x28,0x0006);
		LCD_WR_CMD(0x00,0x0001);		
		LCD_WR_CMD(0x10,0x0000);		
		LCD_WR_CMD(0x01,0x72ef);
		LCD_WR_CMD(0x02,0x0600);
		LCD_WR_CMD(0x03,0x6a38);	
		LCD_WR_CMD(0x11,0x6874);
		LCD_WR_CMD(0x0f,0x0000);    /* RAM WRITE DATA MASK */
		LCD_WR_CMD(0x0b,0x5308);    /* RAM WRITE DATA MASK */
		LCD_WR_CMD(0x0c,0x0003);
		LCD_WR_CMD(0x0d,0x000a);
		LCD_WR_CMD(0x0e,0x2e00);  
		LCD_WR_CMD(0x1e,0x00be);
		LCD_WR_CMD(0x25,0x8000);
		LCD_WR_CMD(0x26,0x7800);
		LCD_WR_CMD(0x27,0x0078);
		LCD_WR_CMD(0x4e,0x0000);
		LCD_WR_CMD(0x4f,0x0000);
		LCD_WR_CMD(0x12,0x08d9);
		/* Adjust the Gamma Curve */
		LCD_WR_CMD(0x30,0x0000);
		LCD_WR_CMD(0x31,0x0104);	 
		LCD_WR_CMD(0x32,0x0100);	
	    LCD_WR_CMD(0x33,0x0305);	
	    LCD_WR_CMD(0x34,0x0505);	 
		LCD_WR_CMD(0x35,0x0305);	
	    LCD_WR_CMD(0x36,0x0707);	
	    LCD_WR_CMD(0x37,0x0300);	
		LCD_WR_CMD(0x3a,0x1200);	
		LCD_WR_CMD(0x3b,0x0800);		 
	    LCD_WR_CMD(0x07,0x0033);
	}
	else if( DeviceCode == 0x1505 || DeviceCode == 0x0505 )
	{
		LCD_Code = R61505U;
		/* initializing funciton */   
		LCD_WR_CMD(0xe5,0x8000);  /* Set the internal vcore voltage */   
		LCD_WR_CMD(0x00,0x0001);  /* start OSC */   
		LCD_WR_CMD(0x2b,0x0010);  /* Set the frame rate as 80 when the internal resistor is used for oscillator circuit */   
		LCD_WR_CMD(0x01,0x0100);  /* s720  to  s1 ; G1 to G320 */   
		LCD_WR_CMD(0x02,0x0700);  /* set the line inversion */   
		LCD_WR_CMD(0x03,0x1018);  /* 65536 colors */    
		LCD_WR_CMD(0x04,0x0000);   
		LCD_WR_CMD(0x08,0x0202);  /* specify the line number of front and back porch periods respectively */   
		LCD_WR_CMD(0x09,0x0000);   
		LCD_WR_CMD(0x0a,0x0000);   
		LCD_WR_CMD(0x0c,0x0000);  /* select  internal system clock */  
		LCD_WR_CMD(0x0d,0x0000);   
		LCD_WR_CMD(0x0f,0x0000);    
		LCD_WR_CMD(0x50,0x0000);  /* set windows adress */   
		LCD_WR_CMD(0x51,0x00ef);   
		LCD_WR_CMD(0x52,0x0000);   
		LCD_WR_CMD(0x53,0x013f);   
		LCD_WR_CMD(0x60,0x2700);   
		LCD_WR_CMD(0x61,0x0001);   
		LCD_WR_CMD(0x6a,0x0000);   
		LCD_WR_CMD(0x80,0x0000);   
		LCD_WR_CMD(0x81,0x0000);   
		LCD_WR_CMD(0x82,0x0000);   
		LCD_WR_CMD(0x83,0x0000);   
		LCD_WR_CMD(0x84,0x0000);   
		LCD_WR_CMD(0x85,0x0000);   
		LCD_WR_CMD(0x90,0x0010);   
		LCD_WR_CMD(0x92,0x0000);   
		LCD_WR_CMD(0x93,0x0003);   
		LCD_WR_CMD(0x95,0x0110);   
		LCD_WR_CMD(0x97,0x0000);   
		LCD_WR_CMD(0x98,0x0000);    
		/* power setting function */   
		LCD_WR_CMD(0x10,0x0000);   
		LCD_WR_CMD(0x11,0x0000);   
		LCD_WR_CMD(0x12,0x0000);   
		LCD_WR_CMD(0x13,0x0000);   
		delay_ms(100);   
		LCD_WR_CMD(0x10,0x17b0);   
		LCD_WR_CMD(0x11,0x0004);   
		delay_ms(50);   
		LCD_WR_CMD(0x12,0x013e);   
		delay_ms(50);   
		LCD_WR_CMD(0x13,0x1f00);   
		LCD_WR_CMD(0x29,0x000f);   
		delay_ms(50);   
		LCD_WR_CMD(0x20,0x0000);   
		LCD_WR_CMD(0x21,0x0000);   
		
		/* initializing function */  	
		LCD_WR_CMD(0x30,0x0204);   
		LCD_WR_CMD(0x31,0x0001);   
		LCD_WR_CMD(0x32,0x0000);   
		LCD_WR_CMD(0x35,0x0206);   
		LCD_WR_CMD(0x36,0x0600);   
		LCD_WR_CMD(0x37,0x0500);   
		LCD_WR_CMD(0x38,0x0505);   
		LCD_WR_CMD(0x39,0x0407);   
		LCD_WR_CMD(0x3c,0x0500);   
		LCD_WR_CMD(0x3d,0x0503);   
		
		/* display on */  
		LCD_WR_CMD(0x07,0x0173);	
	}	
	else if( DeviceCode == 0x8989 )
	{
	    LCD_Code = SSD1289;
	    LCD_WR_CMD(0x0000,0x0001);    delay_ms(50);   /* Žò¿ªŸ§Õñ */
	    LCD_WR_CMD(0x0003,0xA8A4);    delay_ms(50);   
	    LCD_WR_CMD(0x000C,0x0000);    delay_ms(50);   
	    LCD_WR_CMD(0x000D,0x080C);    delay_ms(50);   
	    LCD_WR_CMD(0x000E,0x2B00);    delay_ms(50);   
	    LCD_WR_CMD(0x001E,0x00B0);    delay_ms(50);   
	    LCD_WR_CMD(0x0001,0x2B3F);    delay_ms(50);   /* Çý¶¯Êä³ö¿ØÖÆ320*240 0x2B3F */
	    LCD_WR_CMD(0x0002,0x0600);    delay_ms(50);
	    LCD_WR_CMD(0x0010,0x0000);    delay_ms(50);
	    LCD_WR_CMD(0x0011,0x6070);    delay_ms(50);   /* ¶šÒåÊýŸÝžñÊœ 16Î»É« ºáÆÁ 0x6070 */
	    LCD_WR_CMD(0x0005,0x0000);    delay_ms(50);
	    LCD_WR_CMD(0x0006,0x0000);    delay_ms(50);
	    LCD_WR_CMD(0x0016,0xEF1C);    delay_ms(50);
	    LCD_WR_CMD(0x0017,0x0003);    delay_ms(50);
	    LCD_WR_CMD(0x0007,0x0133);    delay_ms(50);         
	    LCD_WR_CMD(0x000B,0x0000);    delay_ms(50);
	    LCD_WR_CMD(0x000F,0x0000);    delay_ms(50);   /* ÉšÃè¿ªÊŒµØÖ· */
	    LCD_WR_CMD(0x0041,0x0000);    delay_ms(50);
	    LCD_WR_CMD(0x0042,0x0000);    delay_ms(50);
	    LCD_WR_CMD(0x0048,0x0000);    delay_ms(50);
	    LCD_WR_CMD(0x0049,0x013F);    delay_ms(50);
	    LCD_WR_CMD(0x004A,0x0000);    delay_ms(50);
	    LCD_WR_CMD(0x004B,0x0000);    delay_ms(50);
	    LCD_WR_CMD(0x0044,0xEF00);    delay_ms(50);
	    LCD_WR_CMD(0x0045,0x0000);    delay_ms(50);
	    LCD_WR_CMD(0x0046,0x013F);    delay_ms(50);
	    LCD_WR_CMD(0x0030,0x0707);    delay_ms(50);
	    LCD_WR_CMD(0x0031,0x0204);    delay_ms(50);
	    LCD_WR_CMD(0x0032,0x0204);    delay_ms(50);
	    LCD_WR_CMD(0x0033,0x0502);    delay_ms(50);
	    LCD_WR_CMD(0x0034,0x0507);    delay_ms(50);
	    LCD_WR_CMD(0x0035,0x0204);    delay_ms(50);
	    LCD_WR_CMD(0x0036,0x0204);    delay_ms(50);
	    LCD_WR_CMD(0x0037,0x0502);    delay_ms(50);
	    LCD_WR_CMD(0x003A,0x0302);    delay_ms(50);
	    LCD_WR_CMD(0x003B,0x0302);    delay_ms(50);
	    LCD_WR_CMD(0x0023,0x0000);    delay_ms(50);
	    LCD_WR_CMD(0x0024,0x0000);    delay_ms(50);
	    LCD_WR_CMD(0x0025,0x8000);    delay_ms(50);
	    LCD_WR_CMD(0x004f,0);        /* ÐÐÊ×Ö·0 */
	    LCD_WR_CMD(0x004e,0);        /* ÁÐÊ×Ö·0 */
	}
	else if( DeviceCode == 0x8999 )
	{
		LCD_Code = SSD1298;		
		LCD_WR_CMD(0x0028,0x0006);
		LCD_WR_CMD(0x0000,0x0001);
		LCD_WR_CMD(0x0003,0xaea4);    /* power control 1---line frequency and VHG,VGL voltage */
		LCD_WR_CMD(0x000c,0x0004);    /* power control 2---VCIX2 output voltage */
		LCD_WR_CMD(0x000d,0x000c);    /* power control 3---Vlcd63 voltage */
		LCD_WR_CMD(0x000e,0x2800);    /* power control 4---VCOMA voltage VCOML=VCOMH*0.9475-VCOMA */
		LCD_WR_CMD(0x001e,0x00b5);    /* POWER CONTROL 5---VCOMH voltage */   
		LCD_WR_CMD(0x0001,0x3b3f);     
		LCD_WR_CMD(0x0002,0x0600);
		LCD_WR_CMD(0x0010,0x0000);
		LCD_WR_CMD(0x0011,0x6830);
		LCD_WR_CMD(0x0005,0x0000);
		LCD_WR_CMD(0x0006,0x0000);
		LCD_WR_CMD(0x0016,0xef1c);  
		LCD_WR_CMD(0x0007,0x0033);    /* Display control 1 */
		/* when GON=1 and DTE=0,all gate outputs become VGL */
		/* when GON=1 and DTE=0,all gate outputs become VGH */
		/* non-selected gate wires become VGL */     
		LCD_WR_CMD(0x000b,0x0000);
		LCD_WR_CMD(0x000f,0x0000);
		LCD_WR_CMD(0x0041,0x0000);
		LCD_WR_CMD(0x0042,0x0000);
		LCD_WR_CMD(0x0048,0x0000);
		LCD_WR_CMD(0x0049,0x013f);
		LCD_WR_CMD(0x004a,0x0000);
		LCD_WR_CMD(0x004b,0x0000); 
		LCD_WR_CMD(0x0044,0xef00);	/* Horizontal RAM start and end address */
		LCD_WR_CMD(0x0045,0x0000);	/* Vretical RAM start address */
		LCD_WR_CMD(0x0046,0x013f);	/* Vretical RAM end address */ 
		LCD_WR_CMD(0x004e,0x0000);	/* set GDDRAM x address counter */
		LCD_WR_CMD(0x004f,0x0000);    /* set GDDRAM y address counter */   
		/* y control */
		LCD_WR_CMD(0x0030,0x0707);
		LCD_WR_CMD(0x0031,0x0202);
		LCD_WR_CMD(0x0032,0x0204);
		LCD_WR_CMD(0x0033,0x0502);
		LCD_WR_CMD(0x0034,0x0507);
		LCD_WR_CMD(0x0035,0x0204);
		LCD_WR_CMD(0x0036,0x0204);
		LCD_WR_CMD(0x0037,0x0502);
		LCD_WR_CMD(0x003a,0x0302);
		LCD_WR_CMD(0x003b,0x0302); 
		LCD_WR_CMD(0x0023,0x0000);
		LCD_WR_CMD(0x0024,0x0000);
		LCD_WR_CMD(0x0025,0x8000);
		LCD_WR_CMD(0x0026,0x7000);
		LCD_WR_CMD(0x0020,0xb0eb);
		LCD_WR_CMD(0x0027,0x007c);
	}
	else if( DeviceCode == 0x5408 )
	{
		LCD_Code = SPFD5408B;
		
		LCD_WR_CMD(0x0001,0x0100);	  /* Driver Output Contral Register */ 
		LCD_WR_CMD(0x0002,0x0700);      /* LCD Driving Waveform Contral */
		LCD_WR_CMD(0x0003,0x1030);	  /* Entry ModeÉèÖÃ */
		
		LCD_WR_CMD(0x0004,0x0000);	  /* Scalling Control register */
		LCD_WR_CMD(0x0008,0x0207);	  /* Display Control 2 */
		LCD_WR_CMD(0x0009,0x0000);	  /* Display Control 3 */
		LCD_WR_CMD(0x000A,0x0000);	  /* Frame Cycle Control */
		LCD_WR_CMD(0x000C,0x0000);	  /* External Display Interface Control 1 */
		LCD_WR_CMD(0x000D,0x0000);      /* Frame Maker Position */
		LCD_WR_CMD(0x000F,0x0000);	  /* External Display Interface Control 2 */
		delay_ms(50);
		LCD_WR_CMD(0x0007,0x0101);	  /* Display Control */
		delay_ms(50);
		LCD_WR_CMD(0x0010,0x16B0);      /* Power Control 1 */
		LCD_WR_CMD(0x0011,0x0001);      /* Power Control 2 */
		LCD_WR_CMD(0x0017,0x0001);      /* Power Control 3 */
		LCD_WR_CMD(0x0012,0x0138);      /* Power Control 4 */
		LCD_WR_CMD(0x0013,0x0800);      /* Power Control 5 */
		LCD_WR_CMD(0x0029,0x0009);	  /* NVM read data 2 */
		LCD_WR_CMD(0x002a,0x0009);	  /* NVM read data 3 */
		LCD_WR_CMD(0x00a4,0x0000);  
		LCD_WR_CMD(0x0050,0x0000);	  /* ÉèÖÃ²Ù×÷Ž°¿ÚµÄXÖá¿ªÊŒÁÐ */
		LCD_WR_CMD(0x0051,0x00EF);	  /* ÉèÖÃ²Ù×÷Ž°¿ÚµÄXÖáœáÊøÁÐ */
		LCD_WR_CMD(0x0052,0x0000);	  /* ÉèÖÃ²Ù×÷Ž°¿ÚµÄYÖá¿ªÊŒÐÐ */
		LCD_WR_CMD(0x0053,0x013F);	  /* ÉèÖÃ²Ù×÷Ž°¿ÚµÄYÖáœáÊøÐÐ */
		   
		LCD_WR_CMD(0x0060,0x2700);	  /* Driver Output Control */
										  /* ÉèÖÃÆÁÄ»µÄµãÊýÒÔŒ°ÉšÃèµÄÆðÊŒÐÐ */
		LCD_WR_CMD(0x0061,0x0003);	  /* Driver Output Control */
		LCD_WR_CMD(0x006A,0x0000);	  /* Vertical Scroll Control */
		
		LCD_WR_CMD(0x0080,0x0000);	  /* Display Position šC Partial Display 1 */
		LCD_WR_CMD(0x0081,0x0000);	  /* RAM Address Start šC Partial Display 1 */
		LCD_WR_CMD(0x0082,0x0000);	  /* RAM address End - Partial Display 1 */
		LCD_WR_CMD(0x0083,0x0000);	  /* Display Position šC Partial Display 2 */
		LCD_WR_CMD(0x0084,0x0000);	  /* RAM Address Start šC Partial Display 2 */
		LCD_WR_CMD(0x0085,0x0000);	  /* RAM address End šC Partail Display2 */
		LCD_WR_CMD(0x0090,0x0013);	  /* Frame Cycle Control */
		LCD_WR_CMD(0x0092,0x0000); 	  /* Panel Interface Control 2 */
		LCD_WR_CMD(0x0093,0x0003);	  /* Panel Interface control 3 */
		LCD_WR_CMD(0x0095,0x0110);	  /* Frame Cycle Control */
		LCD_WR_CMD(0x0007,0x0173);
	}
	else if( DeviceCode == 0x4531 )
	{	
		LCD_Code = LGDP4531;
		/* Setup display */
		LCD_WR_CMD(0x00,0x0001);
		LCD_WR_CMD(0x10,0x0628);
		LCD_WR_CMD(0x12,0x0006);
		LCD_WR_CMD(0x13,0x0A32);
		LCD_WR_CMD(0x11,0x0040);
		LCD_WR_CMD(0x15,0x0050);
		LCD_WR_CMD(0x12,0x0016);
		delay_ms(50);
		LCD_WR_CMD(0x10,0x5660);
		delay_ms(50);
		LCD_WR_CMD(0x13,0x2A4E);
		LCD_WR_CMD(0x01,0x0100);
		LCD_WR_CMD(0x02,0x0300);	
		LCD_WR_CMD(0x03,0x1030);		
		LCD_WR_CMD(0x08,0x0202);
		LCD_WR_CMD(0x0A,0x0000);
		LCD_WR_CMD(0x30,0x0000);
		LCD_WR_CMD(0x31,0x0402);
		LCD_WR_CMD(0x32,0x0106);
		LCD_WR_CMD(0x33,0x0700);
		LCD_WR_CMD(0x34,0x0104);
		LCD_WR_CMD(0x35,0x0301);
		LCD_WR_CMD(0x36,0x0707);
		LCD_WR_CMD(0x37,0x0305);
		LCD_WR_CMD(0x38,0x0208);
		LCD_WR_CMD(0x39,0x0F0B);
		delay_ms(50);
		LCD_WR_CMD(0x41,0x0002);
		LCD_WR_CMD(0x60,0x2700);
		LCD_WR_CMD(0x61,0x0001);
		LCD_WR_CMD(0x90,0x0119);
		LCD_WR_CMD(0x92,0x010A);
		LCD_WR_CMD(0x93,0x0004);
		LCD_WR_CMD(0xA0,0x0100);
		delay_ms(50);
		LCD_WR_CMD(0x07,0x0133);
		delay_ms(50);
		LCD_WR_CMD(0xA0,0x0000);
	}
	else if( DeviceCode == 0x4535 )
	{	
		LCD_Code = LGDP4535;	
		LCD_WR_CMD(0x15, 0x0030); 	 /* Set the internal vcore voltage */                                              
		LCD_WR_CMD(0x9A, 0x0010); 	 /* Start internal OSC */
		LCD_WR_CMD(0x11, 0x0020);	     /* set SS and SM bit */
		LCD_WR_CMD(0x10, 0x3428);	     /* set 1 line inversion */
		LCD_WR_CMD(0x12, 0x0002);	     /* set GRAM write direction and BGR=1 */ 
		LCD_WR_CMD(0x13, 0x1038);	     /* Resize register */
		delay_ms(40); 
		LCD_WR_CMD(0x12, 0x0012);	     /* set the back porch and front porch */
		delay_ms(40); 
		LCD_WR_CMD(0x10, 0x3420);	     /* set non-display area refresh cycle ISC[3:0] */
		LCD_WR_CMD(0x13, 0x3045);	     /* FMARK function */
		delay_ms(70); 
		LCD_WR_CMD(0x30, 0x0000);      /* RGB interface setting */
		LCD_WR_CMD(0x31, 0x0402);	     /* Frame marker Position */
		LCD_WR_CMD(0x32, 0x0307);      /* RGB interface polarity */
		LCD_WR_CMD(0x33, 0x0304);      /* SAP, BT[3:0], AP, DSTB, SLP, STB */
		LCD_WR_CMD(0x34, 0x0004);      /* DC1[2:0], DC0[2:0], VC[2:0] */
		LCD_WR_CMD(0x35, 0x0401);      /* VREG1OUT voltage */
		LCD_WR_CMD(0x36, 0x0707);      /* VDV[4:0] for VCOM amplitude */
		LCD_WR_CMD(0x37, 0x0305);      /* SAP, BT[3:0], AP, DSTB, SLP, STB */
		LCD_WR_CMD(0x38, 0x0610);      /* DC1[2:0], DC0[2:0], VC[2:0] */
		LCD_WR_CMD(0x39, 0x0610);      /* VREG1OUT voltage */
		LCD_WR_CMD(0x01, 0x0100);      /* VDV[4:0] for VCOM amplitude */
		LCD_WR_CMD(0x02, 0x0300);      /* VCM[4:0] for VCOMH */
		LCD_WR_CMD(0x03, 0x1030);      /* GRAM horizontal Address */
		LCD_WR_CMD(0x08, 0x0808);      /* GRAM Vertical Address */
		LCD_WR_CMD(0x0A, 0x0008);		
		LCD_WR_CMD(0x60, 0x2700);	     /* Gate Scan Line */
		LCD_WR_CMD(0x61, 0x0001);	     /* NDL,VLE, REV */
		LCD_WR_CMD(0x90, 0x013E);
		LCD_WR_CMD(0x92, 0x0100);
		LCD_WR_CMD(0x93, 0x0100);
		LCD_WR_CMD(0xA0, 0x3000);
		LCD_WR_CMD(0xA3, 0x0010);
		LCD_WR_CMD(0x07, 0x0001);
		LCD_WR_CMD(0x07, 0x0021);
		LCD_WR_CMD(0x07, 0x0023);
		LCD_WR_CMD(0x07, 0x0033);
		LCD_WR_CMD(0x07, 0x0133);
	} 		 		
	else if( DeviceCode == 0x0047 )
	{
		LCD_Code = HX8347D;
		/* Start Initial Sequence */
		LCD_WR_CMD(0xEA,0x00);                          
		LCD_WR_CMD(0xEB,0x20);                                                     
		LCD_WR_CMD(0xEC,0x0C);                                                   
		LCD_WR_CMD(0xED,0xC4);                                                    
		LCD_WR_CMD(0xE8,0x40);                                                     
		LCD_WR_CMD(0xE9,0x38);                                                    
		LCD_WR_CMD(0xF1,0x01);                                                    
		LCD_WR_CMD(0xF2,0x10);                                                    
		LCD_WR_CMD(0x27,0xA3);                                                    
		/* GAMMA SETTING */
		LCD_WR_CMD(0x40,0x01);                           
		LCD_WR_CMD(0x41,0x00);                                                   
		LCD_WR_CMD(0x42,0x00);                                                   
		LCD_WR_CMD(0x43,0x10);                                                    
		LCD_WR_CMD(0x44,0x0E);                                                   
		LCD_WR_CMD(0x45,0x24);                                                  
		LCD_WR_CMD(0x46,0x04);                                                  
		LCD_WR_CMD(0x47,0x50);                                                   
		LCD_WR_CMD(0x48,0x02);                                                    
		LCD_WR_CMD(0x49,0x13);                                                  
		LCD_WR_CMD(0x4A,0x19);                                                  
		LCD_WR_CMD(0x4B,0x19);                                                 
		LCD_WR_CMD(0x4C,0x16);                                                 
		LCD_WR_CMD(0x50,0x1B);                                                   
		LCD_WR_CMD(0x51,0x31);                                                    
		LCD_WR_CMD(0x52,0x2F);                                                     
		LCD_WR_CMD(0x53,0x3F);                                                    
		LCD_WR_CMD(0x54,0x3F);                                                     
		LCD_WR_CMD(0x55,0x3E);                                                     
		LCD_WR_CMD(0x56,0x2F);                                                   
		LCD_WR_CMD(0x57,0x7B);                                                     
		LCD_WR_CMD(0x58,0x09);                                                  
		LCD_WR_CMD(0x59,0x06);                                                 
		LCD_WR_CMD(0x5A,0x06);                                                   
		LCD_WR_CMD(0x5B,0x0C);                                                   
		LCD_WR_CMD(0x5C,0x1D);                                                   
		LCD_WR_CMD(0x5D,0xCC);                                                   
		/* Power Voltage Setting */
		LCD_WR_CMD(0x1B,0x18);                                                    
		LCD_WR_CMD(0x1A,0x01);                                                    
		LCD_WR_CMD(0x24,0x15);                                                    
		LCD_WR_CMD(0x25,0x50);                                                    
		LCD_WR_CMD(0x23,0x8B);                                                   
		LCD_WR_CMD(0x18,0x36);                           
		LCD_WR_CMD(0x19,0x01);                                                    
		LCD_WR_CMD(0x01,0x00);                                                   
		LCD_WR_CMD(0x1F,0x88);                                                    
		delay_ms(50);
		LCD_WR_CMD(0x1F,0x80);                                                  
		delay_ms(50);
		LCD_WR_CMD(0x1F,0x90);                                                   
		delay_ms(50);
		LCD_WR_CMD(0x1F,0xD0);                                                   
		delay_ms(50);
		LCD_WR_CMD(0x17,0x05);                                                    
		LCD_WR_CMD(0x36,0x00);                                                    
		LCD_WR_CMD(0x28,0x38);                                                 
		delay_ms(50);
		LCD_WR_CMD(0x28,0x3C);                                                
	}
	else if( DeviceCode == 0x7783 )
	{
		LCD_Code = ST7781;
		/* Start Initial Sequence */
		LCD_WR_CMD(0x00FF,0x0001);
		LCD_WR_CMD(0x00F3,0x0008);
		LCD_WR_CMD(0x0001,0x0100);
		LCD_WR_CMD(0x0002,0x0700);
		LCD_WR_CMD(0x0003,0x1030);  
		LCD_WR_CMD(0x0008,0x0302);
		LCD_WR_CMD(0x0008,0x0207);
		LCD_WR_CMD(0x0009,0x0000);
		LCD_WR_CMD(0x000A,0x0000);
		LCD_WR_CMD(0x0010,0x0000);  
		LCD_WR_CMD(0x0011,0x0005);
		LCD_WR_CMD(0x0012,0x0000);
		LCD_WR_CMD(0x0013,0x0000);
		delay_ms(50);
		LCD_WR_CMD(0x0010,0x12B0);
		delay_ms(50);
		LCD_WR_CMD(0x0011,0x0007);
		delay_ms(50);
		LCD_WR_CMD(0x0012,0x008B);
		delay_ms(50);	
		LCD_WR_CMD(0x0013,0x1700);
		delay_ms(50);	
		LCD_WR_CMD(0x0029,0x0022);		
		LCD_WR_CMD(0x0030,0x0000);
		LCD_WR_CMD(0x0031,0x0707);
		LCD_WR_CMD(0x0032,0x0505);
		LCD_WR_CMD(0x0035,0x0107);
		LCD_WR_CMD(0x0036,0x0008);
		LCD_WR_CMD(0x0037,0x0000);
		LCD_WR_CMD(0x0038,0x0202);
		LCD_WR_CMD(0x0039,0x0106);
		LCD_WR_CMD(0x003C,0x0202);
		LCD_WR_CMD(0x003D,0x0408);
		delay_ms(50);				
		LCD_WR_CMD(0x0050,0x0000);		
		LCD_WR_CMD(0x0051,0x00EF);		
		LCD_WR_CMD(0x0052,0x0000);		
		LCD_WR_CMD(0x0053,0x013F);		
		LCD_WR_CMD(0x0060,0xA700);		
		LCD_WR_CMD(0x0061,0x0001);
		LCD_WR_CMD(0x0090,0x0033);				
		LCD_WR_CMD(0x002B,0x000B);		
		LCD_WR_CMD(0x0007,0x0133);
	}
	else	/* special ID */
	{
		DeviceCode = LCD_RD_CMD(0x67);
		if( DeviceCode == 0x0046 )
		{
	        LCD_Code = HX8346A;

			/* Gamma for CMO 3.2 */
			LCD_WR_CMD(0x46,0x94);
			LCD_WR_CMD(0x47,0x41);
			LCD_WR_CMD(0x48,0x00);
			LCD_WR_CMD(0x49,0x33);
			LCD_WR_CMD(0x4a,0x23);
			LCD_WR_CMD(0x4b,0x45);
			LCD_WR_CMD(0x4c,0x44);
			LCD_WR_CMD(0x4d,0x77);
			LCD_WR_CMD(0x4e,0x12);
			LCD_WR_CMD(0x4f,0xcc);
			LCD_WR_CMD(0x50,0x46);
			LCD_WR_CMD(0x51,0x82);
			/* 240x320 window setting */
			LCD_WR_CMD(0x02,0x00);
			LCD_WR_CMD(0x03,0x00);
			LCD_WR_CMD(0x04,0x01);
			LCD_WR_CMD(0x05,0x3f);
			LCD_WR_CMD(0x06,0x00);
			LCD_WR_CMD(0x07,0x00);
			LCD_WR_CMD(0x08,0x00); 
			LCD_WR_CMD(0x09,0xef); 
			
			/* Display Setting */
			LCD_WR_CMD(0x01,0x06);						
			LCD_WR_CMD(0x16,0xC8);	/* MY(1) MX(1) MV(0) */

			LCD_WR_CMD(0x23,0x95);
			LCD_WR_CMD(0x24,0x95);
			LCD_WR_CMD(0x25,0xff);
			
			LCD_WR_CMD(0x27,0x02);
			LCD_WR_CMD(0x28,0x02);
			LCD_WR_CMD(0x29,0x02);
			LCD_WR_CMD(0x2a,0x02);
			LCD_WR_CMD(0x2c,0x02);
			LCD_WR_CMD(0x2d,0x02);						
			
			LCD_WR_CMD(0x3a,0x01);
			LCD_WR_CMD(0x3b,0x01);
			LCD_WR_CMD(0x3c,0xf0);
			LCD_WR_CMD(0x3d,0x00);
			delay_ms(2);
			LCD_WR_CMD(0x35,0x38);
			LCD_WR_CMD(0x36,0x78);
			
			LCD_WR_CMD(0x3e,0x38);
			
			LCD_WR_CMD(0x40,0x0f);
			LCD_WR_CMD(0x41,0xf0);
			/* Power Supply Setting */
			LCD_WR_CMD(0x19,0x49);
			LCD_WR_CMD(0x93,0x0f);
			delay_ms(1);
			LCD_WR_CMD(0x20,0x30);
			LCD_WR_CMD(0x1d,0x07);
			LCD_WR_CMD(0x1e,0x00);
			LCD_WR_CMD(0x1f,0x07);
			/* VCOM Setting for CMO 3.2¡± Panel */
			LCD_WR_CMD(0x44,0x4d);
			LCD_WR_CMD(0x45,0x13);
			delay_ms(1);
			LCD_WR_CMD(0x1c,0x04);
			delay_ms(2);
			LCD_WR_CMD(0x43,0x80);
			delay_ms(5);
			LCD_WR_CMD(0x1b,0x08);
			delay_ms(4);
			LCD_WR_CMD(0x1b,0x10);		  
			delay_ms(4);		
			/* Display ON Setting */
			LCD_WR_CMD(0x90,0x7f);
			LCD_WR_CMD(0x26,0x04);
			delay_ms(4);
			LCD_WR_CMD(0x26,0x24);
			LCD_WR_CMD(0x26,0x2c);
			delay_ms(4);
			LCD_WR_CMD(0x26,0x3c);	
			/* Set internal VDDD voltage */
			LCD_WR_CMD(0x57,0x02);
			LCD_WR_CMD(0x55,0x00);
			LCD_WR_CMD(0x57,0x00);
		}
		if( DeviceCode == 0x0047 )
		{
	        LCD_Code = HX8347A;
	        LCD_WR_CMD(0x0042,0x0008);   
	        /* Gamma setting */  
		    LCD_WR_CMD(0x0046,0x00B4); 
		    LCD_WR_CMD(0x0047,0x0043); 
		    LCD_WR_CMD(0x0048,0x0013); 
		    LCD_WR_CMD(0x0049,0x0047); 
		    LCD_WR_CMD(0x004A,0x0014); 
		    LCD_WR_CMD(0x004B,0x0036); 
		    LCD_WR_CMD(0x004C,0x0003); 
		    LCD_WR_CMD(0x004D,0x0046); 
		    LCD_WR_CMD(0x004E,0x0005);  
		    LCD_WR_CMD(0x004F,0x0010);  
		    LCD_WR_CMD(0x0050,0x0008);  
		    LCD_WR_CMD(0x0051,0x000a);  
		    /* Window Setting */
		    LCD_WR_CMD(0x0002,0x0000); 
		    LCD_WR_CMD(0x0003,0x0000); 
		    LCD_WR_CMD(0x0004,0x0000); 
		    LCD_WR_CMD(0x0005,0x00EF); 
		    LCD_WR_CMD(0x0006,0x0000); 
		    LCD_WR_CMD(0x0007,0x0000); 
		    LCD_WR_CMD(0x0008,0x0001); 
		    LCD_WR_CMD(0x0009,0x003F); 
		    delay_ms(10); 
		    LCD_WR_CMD(0x0001,0x0006); 
		    LCD_WR_CMD(0x0016,0x00C8);   
		    LCD_WR_CMD(0x0023,0x0095); 
		    LCD_WR_CMD(0x0024,0x0095); 
		    LCD_WR_CMD(0x0025,0x00FF); 
		    LCD_WR_CMD(0x0027,0x0002); 
		    LCD_WR_CMD(0x0028,0x0002); 
		    LCD_WR_CMD(0x0029,0x0002); 
		    LCD_WR_CMD(0x002A,0x0002); 
		    LCD_WR_CMD(0x002C,0x0002); 
		    LCD_WR_CMD(0x002D,0x0002); 
		    LCD_WR_CMD(0x003A,0x0001);  
		    LCD_WR_CMD(0x003B,0x0001);  
		    LCD_WR_CMD(0x003C,0x00F0);    
		    LCD_WR_CMD(0x003D,0x0000); 
		    delay_ms(20); 
		    LCD_WR_CMD(0x0035,0x0038); 
		    LCD_WR_CMD(0x0036,0x0078); 
		    LCD_WR_CMD(0x003E,0x0038); 
		    LCD_WR_CMD(0x0040,0x000F); 
		    LCD_WR_CMD(0x0041,0x00F0);  
		    LCD_WR_CMD(0x0038,0x0000); 
		    /* Power Setting */ 
		    LCD_WR_CMD(0x0019,0x0049);  
		    LCD_WR_CMD(0x0093,0x000A); 
		    delay_ms(10); 
		    LCD_WR_CMD(0x0020,0x0020);   
		    LCD_WR_CMD(0x001D,0x0003);   
		    LCD_WR_CMD(0x001E,0x0000);  
		    LCD_WR_CMD(0x001F,0x0009);   
		    LCD_WR_CMD(0x0044,0x0053);  
		    LCD_WR_CMD(0x0045,0x0010);   
		    delay_ms(10);  
		    LCD_WR_CMD(0x001C,0x0004);  
		    delay_ms(20); 
		    LCD_WR_CMD(0x0043,0x0080);    
		    delay_ms(5); 
		    LCD_WR_CMD(0x001B,0x000a);    
		    delay_ms(40);  
		    LCD_WR_CMD(0x001B,0x0012);    
		    delay_ms(40);   
		    /* Display On Setting */ 
		    LCD_WR_CMD(0x0090,0x007F); 
		    LCD_WR_CMD(0x0026,0x0004); 
		    delay_ms(40);  
		    LCD_WR_CMD(0x0026,0x0024); 
		    LCD_WR_CMD(0x0026,0x002C); 
		    delay_ms(40);   
		    LCD_WR_CMD(0x0070,0x0008); 
		    LCD_WR_CMD(0x0026,0x003C);  
		    LCD_WR_CMD(0x0057,0x0002); 
		    LCD_WR_CMD(0x0055,0x0000); 
		    LCD_WR_CMD(0x0057,0x0000); 
		}	 
	}  		
#endif				
    delay_ms(50);   /* delay 50 ms */	

}


static inline void lcdSetCursor(JsGraphics *gfx, unsigned short x, unsigned short y) {
  x = (gfx->data.width-1)-x;

  switch( LCD_Code )
  {
     default:		 /* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x7783 0x4531 0x4535 */
          LCD_WR_CMD(0x0020, y );     
          LCD_WR_CMD(0x0021, x );     
	      break; 
#ifndef SAVE_ON_FLASH
     case SSD1298: 	 /* 0x8999 */
     case SSD1289:   /* 0x8989 */
	      LCD_WR_CMD(0x004e, y );      
          LCD_WR_CMD(0x004f, x );
	      break;  

     case HX8346A: 	 /* 0x0046 */
     case HX8347A: 	 /* 0x0047 */
     case HX8347D: 	 /* 0x0047 */
	      LCD_WR_CMD(0x02, y>>8 );                                                  
	      LCD_WR_CMD(0x03, y );  

	      LCD_WR_CMD(0x06, x>>8 );                           
	      LCD_WR_CMD(0x07, x );    
	
	      break;     
     case SSD2119:	 /* 3.5 LCD 0x9919 */
	      break; 
#endif
  }
}

static inline void lcdSetWindow(JsGraphics *gfx, unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2) {
  // x1>=x2  and y1>=y2
  x2 = (gfx->data.width-1)-x2;
  x1 = (gfx->data.width-1)-x1;
  switch (LCD_Code) {
     default:
        LCD_WR_CMD(0x50, y1);
        LCD_WR_CMD(0x51, y2);
        LCD_WR_CMD(0x52, x2);
        LCD_WR_CMD(0x53, x1);
        break;
#ifndef SAVE_ON_FLASH
     case SSD1289:   /* 0x8989 */
        LCD_WR_CMD(0x44, y1 | (y2<<8));
        LCD_WR_CMD(0x45, x2);
        LCD_WR_CMD(0x46, x1);
        break;
     case HX8346A: 
     case HX8347A: 
     case HX8347D: 
        LCD_WR_CMD(0x02,y1>>8);
        LCD_WR_CMD(0x03,y1);
        LCD_WR_CMD(0x04,y2>>8);
        LCD_WR_CMD(0x05,y2);
        LCD_WR_CMD(0x06,x2>>8);
        LCD_WR_CMD(0x07,x2);
        LCD_WR_CMD(0x08,x1>>8); 
        LCD_WR_CMD(0x09,x1); 
        break;
#endif
  }
}

static inline void lcdSetFullWindow(JsGraphics *gfx) {
  lcdSetWindow(gfx,0,0,gfx->data.width-1,gfx->data.height-1);
}



void lcdFillRect_FSMC(JsGraphics *gfx, short x1, short y1, short x2, short y2) {
  if (x1>x2) {
    short l=x1; x1 = x2; x2 = l;
  }
  if (y1>y2) {
    short l=y1; y1 = y2; y2 = l;
  }
  // offscreen
  if (x1>=gfx->data.width || y1>=gfx->data.height || x2<0 || y2<0) return;
  // now clip
  if (x1<0) x1=0;
  if (y1<0) y1=0;
  if (x2>=gfx->data.width) x2=gfx->data.width-1;
  if (y2>=gfx->data.height) y2=gfx->data.height-1;
  // finally!
  if (x1==x2) { // special case for single vertical line - no window needed
    lcdSetCursor(gfx,x2,y1);
    LCD_WR_REG(0x22); // start data tx
    unsigned int i=0, l=(1+y2-y1);
    LCD_WR_Data_multi(gfx->data.fgColor, l);
  } else {
    lcdSetWindow(gfx,x1,y1,x2,y2);
    lcdSetCursor(gfx,x2,y1);
    LCD_WR_REG(0x22); // start data tx
    unsigned int i=0, l=(1+x2-x1)*(1+y2-y1);
    LCD_WR_Data_multi(gfx->data.fgColor, l);
    lcdSetFullWindow(gfx);
  }
}

/* Output a 1 bit bitmap */
void lcdBitmap1bit_FSMC(JsGraphics *gfx, short x1, short y1, unsigned short width, unsigned short height, unsigned char *data) {
  lcdSetWindow(gfx,x1,y1,x1+width-1,y1+height-1);
  lcdSetCursor(gfx,x1+width-1,y1);
  LCD_WR_REG(0x22); // start data tx
  unsigned int x,y;
  for(x=0;x<width;x++) {
    for(y=0;y<height;y++) {
      int bitOffset = x+(y*width);
      LCD_WR_Data(((data[bitOffset>>3]>>(bitOffset&7))&1) ? gfx->data.fgColor : gfx->data.bgColor);
    }
  }
  lcdSetFullWindow(gfx);
}

unsigned int lcdGetPixel_FSMC(JsGraphics *gfx, short x, short y) {
  if (x<0 || y<0 || x>=gfx->data.width || y>=gfx->data.height) return 0;
  lcdSetCursor(gfx,x,y);
  LCD_WR_REG(0x22); // start data tx
  return LCD_RD_Data();
}


void lcdSetPixel_FSMC(JsGraphics *gfx, short x, short y, unsigned int col) {
  if (x<0 || y<0 || x>=gfx->data.width || y>=gfx->data.height) return;
  lcdSetCursor(gfx,x,y);
  LCD_WR_REG(34);
  LCD_WR_Data(col);
}

void lcdInit_FSMC(JsGraphics *gfx) {
  assert(gfx->data.bpp == 16);

  LCD_init_hardware();
  LCD_init_panel();
  lcdSetFullWindow(gfx);
}

void lcdSetCallbacks_FSMC(JsGraphics *gfx) {
  gfx->setPixel = lcdSetPixel_FSMC;
  gfx->getPixel = lcdGetPixel_FSMC;
  gfx->fillRect = lcdFillRect_FSMC;
  gfx->bitmap1bit = lcdBitmap1bit_FSMC;
}

