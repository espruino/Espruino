/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2012  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.16 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to Energy Micro AS whose registered office
is situated at  Sandakerveien 118, N-0484 Oslo, NORWAY solely
for  the  purposes  of  creating  libraries  for Energy Micros ARM Cortex-M3, M4F
processor-based  devices,  sublicensed  and distributed  under the terms and
conditions  of  the   End  User  License Agreement supplied by Energy Micro AS.
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : LCDConf.c
Purpose     : Display controller configuration (single layer)
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"

#include "GUIDRV_Lin.h"
#include "bsp.h"
#include "em_cmu.h"
#include "em_ebi.h"
#include "tftdirect.h"
#include "bsp_trace.h"
#include "touch.h"
#include "LCDConf.h"

/*********************************************************************
*
*       Layer configuration (to be modified)
*
**********************************************************************
*/
//
// Physical display size
//
#define XSIZE_PHYS 320
#define YSIZE_PHYS 240
#define FRAME_BUFFER_SIZE (XSIZE_PHYS * YSIZE_PHYS * 2)

//
// Color conversion
//
#define COLOR_CONVERSION GUICC_M565

//
// Display driver
//
#define DISPLAY_DRIVER GUIDRV_LIN_16

//
// Buffers / VScreens
//
#define NUM_BUFFERS  1 // Number of multiple buffers to be used
#define NUM_VSCREENS 4 // Number of virtual screens to be used

/*********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#ifndef   VRAM_ADDR
  #define VRAM_ADDR (void *)EBI_BankAddress(EBI_BANK2)
#endif
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   COLOR_CONVERSION
  #error Color conversion not defined!
#endif
#ifndef   DISPLAY_DRIVER
  #error No display driver defined!
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/* Configure TFT direct drive from EBI BANK2 */
static const EBI_TFTInit_TypeDef tftInit =
{ ebiTFTBank2,                  /* Select EBI Bank 2 */
  ebiTFTWidthHalfWord,          /* Select 2-byte (16-bit RGB565) increments */
  ebiTFTColorSrcMem,            /* Use memory as source for mask/blending */
  ebiTFTInterleaveUnlimited,    /* unlimited interleaved accesses allowed */
  ebiTFTFrameBufTriggerVSync,   /* VSYNC as frame buffer update trigger */
  false,                        /* Drive DCLK from negative edge of internal clock */
  ebiTFTMBDisabled,             /* No masking and alpha blending enabled */
  ebiTFTDDModeExternal,         /* Drive from external memory */
  ebiActiveLow,                 /* CS Active Low polarity */
  ebiActiveHigh,                /* DCLK Active High polarity */
  ebiActiveLow,                 /* DATAEN Active Low polarity */
  ebiActiveLow,                 /* HSYNC Active Low polarity */
  ebiActiveLow,                 /* VSYNC Active Low polarity */
  320,                          /* Horizontal size in pixels */
  20,                            /* Horizontal Front Porch */
  30,                           /* Horizontal Back Porch */
  2,                            /* Horizontal Synchronization Pulse Width */
  240,                          /* Vertical size in pixels */
  2,                            /* Vertical Front Porch */
  4,                            /* Vertical Back Porch */
  2,                            /* Vertical Synchronization Pulse Width */
  0x0000,                       /* Frame Address pointer offset to EBI memory base */
  5,                            /* DCLK Period */
  0,                            /* DCLK Start cycles */
  0,                            /* DCLK Setup cycles */
  0,                            /* DCLK Hold cycles */
};

static uint32_t VRAM_pointer = 0;
static int orgx, orgy;
volatile int pendingFrameIndex = -1;

/* This function is call from TOUCH driver every time pen state is changed */
/* (up/down) or is moved. There is translation to emWin structure done.    */
void LCD_TOUCH_Upcall(TOUCH_Pos_TypeDef *pos)
{ static GUI_PID_STATE gui_pos;

  if(pos->x < LCD_GetXSize()) gui_pos.x = pos->x;
    else gui_pos.x = LCD_GetXSize()-1;
  if(pos->y < LCD_GetYSize()) gui_pos.y = pos->y;
    else gui_pos.y = LCD_GetYSize()-1;
  gui_pos.Pressed = pos->pen;
  GUI_TOUCH_StoreStateEx(&gui_pos);
}


/*********************************************************************
*
*       _InitController
*
* Purpose:
*   Should initialize the display controller
*/
static void _InitController(void) {
  /* Initialize EBI banks (Board Controller, external PSRAM, ..) */
  BSP_Init(BSP_INIT_DK_EBI);
  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Configure for 48MHz HFXO operation of core clock */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

  CMU_ClockEnable( cmuClock_GPIO, true);
  CMU_ClockEnable( cmuClock_ADC0, true);
  orgx = orgy = 0;
  LCD_InitializeLCD();
}

/*********************************************************************
*
*       _SetVRAMAddr
*
* Purpose:
*   Should set the frame buffer base address
*/
static void _SetVRAMAddr(void * pVRAM) {
    VRAM_pointer = (uint32_t)pVRAM - EBI_BankAddress(EBI_BANK2);
    EBI_TFTFrameBaseSet(VRAM_pointer);
}

/*********************************************************************
*
*       _SetOrg
*
* Purpose:
*   Should set the origin of the display typically by modifying the
*   frame buffer base address register
*/
static void _SetOrg(int xPos, int yPos) {
    orgx = xPos;
    orgy = yPos;
    EBI_TFTFrameBaseSet(VRAM_pointer+2*( (XSIZE_PHYS*yPos) + xPos ) );
}

/*********************************************************************
*
*       _SetLUTEntry
*
* Purpose:
*   Should set the desired LUT entry
*/
static void _SetLUTEntry(LCD_COLOR Color, U8 Pos) {
  /* TBD by customer */
  (void)Color;              /* Unused parameter */
  (void)Pos;                /* Unused parameter */
}
/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/* Initialize LCD in Direct mode and touch panel */
void LCD_InitializeLCD(void)
{ TOUCH_Config_TypeDef touch_init = TOUCH_INIT_DEFAULT;

  TFT_DirectInit(&tftInit);
  _SetVRAMAddr((void *)(VRAM_pointer + EBI_BankAddress(EBI_BANK2)));
  _SetOrg(orgx, orgy);
  /* Initialize touch panel driver and register upcall function */
  TOUCH_Init(&touch_init);
  TOUCH_RegisterUpcall(LCD_TOUCH_Upcall);
}

void LCD_InitializeDriver(void)
{ LCD_InitializeLCD();
}

/* VSYNC interrupt handler. Will send confirmation to emWin that frame buffer
 * addres is changed. Frame buffer address is automatically reloaded during VSYNC.
 */
void EBI_IRQHandler(void) {
  uint32_t flags;

  flags = EBI_IntGet();
  EBI_IntClear(flags);

  if ( flags & EBI_IF_VFPORCH ) {
    if ( pendingFrameIndex >= 0 ) {

      /* Send a confirmation that the buffer is visible now */
      GUI_MULTIBUF_Confirm(pendingFrameIndex);

      pendingFrameIndex = -1;
    }
  }
}

void EnableVsyncInterrupt(void) {
  /* Disable all EBI interrupts */
  EBI_IntDisable(_EBI_IF_MASK);

  /* Clear previous interrupts */
  EBI_IntClear(_EBI_IF_MASK);

  /* Enable VSYNC interrupt */
  EBI_IntEnable(EBI_IF_VFPORCH);

  /* Enable EBI interrupts in core */
  NVIC_ClearPendingIRQ(EBI_IRQn);
  NVIC_EnableIRQ(EBI_IRQn);
}


/*********************************************************************
*
*       LCD_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*
*/
void LCD_X_Config(void) {

  GUI_MULTIBUF_Config(NUM_BUFFERS);
  //
  // Set display driver and color conversion for 1st layer
  //
  GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER, COLOR_CONVERSION, 0, 0);
  //
  // Display driver configuration, required for Lin-driver
  //
  if (LCD_GetSwapXY()) {
    LCD_SetSizeEx (0, YSIZE_PHYS, XSIZE_PHYS);
    LCD_SetVSizeEx(0, YSIZE_PHYS, XSIZE_PHYS);
  } else {
    LCD_SetSizeEx (0, XSIZE_PHYS, YSIZE_PHYS);
    LCD_SetVSizeEx(0, XSIZE_PHYS, YSIZE_PHYS);
  }
  LCD_SetVRAMAddrEx(0, (void *)VRAM_ADDR);
  //
  // Set user palette data (only required if no fixed palette is used)
  //
  #if defined(PALETTE)
    LCD_SetLUTEx(0, PALETTE);
  #endif
}

/*********************************************************************
*
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r;

  (void)LayerIndex;                   /* Unused parameter */

  switch (Cmd) {
  //
  // Required
  //
  case LCD_X_INITCONTROLLER: {
    //
    // Called during the initialization process in order to set up the
    // display controller and put it into operation. If the display
    // controller is not initialized by any external routine this needs
    // to be adapted by the customer...
    //
    _InitController();
    EnableVsyncInterrupt();
    return 0;
  }
  case LCD_X_SETVRAMADDR: {
    //
    // Required for setting the address of the video RAM for drivers
    // with memory mapped video RAM which is passed in the 'pVRAM' element of p
    //
    LCD_X_SETVRAMADDR_INFO * p;
    p = (LCD_X_SETVRAMADDR_INFO *)pData;
    _SetVRAMAddr(p->pVRAM);
    return 0;
  }
  case LCD_X_SETORG: {
    //
    // Required for setting the display origin which is passed in the 'xPos' and 'yPos' element of p
    //
    LCD_X_SETORG_INFO * p;
    p = (LCD_X_SETORG_INFO *)pData;
    _SetOrg(p->xPos, p->yPos);
    return 0;
  }
  case LCD_X_SETLUTENTRY: {
    //
    // Required for setting a lookup table entry which is passed in the 'Pos' and 'Color' element of p
    //
    LCD_X_SETLUTENTRY_INFO * p;
    p = (LCD_X_SETLUTENTRY_INFO *)pData;
    _SetLUTEntry(p->Color, p->Pos);
    return 0;
  }
  case LCD_X_ON: {
    //
    // Required if the display controller should support switching on and off
    //
    return 0;
  }
  case LCD_X_OFF: {
    //
    // Required if the display controller should support switching on and off
    //
    // ...
    return 0;
  }
  /* This command is received every time the GUI is done drawing a frame */
  case LCD_X_SHOWBUFFER:
  {
    /* Get the data object */
    LCD_X_SHOWBUFFER_INFO * p;
    p = (LCD_X_SHOWBUFFER_INFO *)pData;

    /* Set frame buffer to the new frame, will be automatically reloaded on VSYNC */
    EBI_TFTFrameBaseSet(FRAME_BUFFER_SIZE * p->Index);
    /* Save the pending buffer index. ISR will send confirmation on next VSYNC. */
    pendingFrameIndex = p->Index;

    return 0;
  }

  default:
    r = -1;
  }
  return r;
}

/*************************** End of file ****************************/
