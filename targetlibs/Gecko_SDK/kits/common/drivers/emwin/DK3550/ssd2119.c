/***************************************************************************//**
 * @file
 * @brief SSD2119 LCD controller driver
 * @version 4.2.1
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#if (!defined(WIN32) || defined(LCD_SIMCONTROLLER))

/*********************************************************************
*
*       Include common code
*
**********************************************************************
*/

#include <stddef.h>
#include <string.h>
#include "em_types.h"
//#include "LCD_Private.h"
#include "GUI_Private.h"
//#include "LCD_SIM.h"
//#include "LCD_ConfDefaults.h"

#include "dmd/ssd2119/dmd_ssd2119_registers.h"
#include "ssd2119.h"

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/

typedef struct
{
  uint32_t VRAMAddr;
  uint32_t BaseAddr;
  volatile uint16_t *commandAddr;
  volatile uint16_t *dataAddr;
  int BufferIndex;
  int xSize, ySize;
  int vxSize, vySize;
  int vxSizePhys;
  int xPos, yPos;
  int Alpha;
  int IsVisible;
  void (* pfFillRect)  (int LayerIndex, int x0, int y0, int x1, int y1, uint32_t PixelIndex);   void (* pfCopyBuffer)(int LayerIndex, int IndexSrc, int IndexDst);
  void (* pfDrawBMP1)  (int LayerIndex, int x, int y, uint8_t const * p, int Diff, int xSize, int ySize, int BytesPerLine, const LCD_PIXELINDEX * pTrans);
  void (* pfCopyRect)  (int LayerIndex, int x0, int y0, int x1, int y1, int xSize, int ySize);
#ifdef SSD2119_REGISTER_ACCESS_HOOKS
  void (* pfWriteRegister) (uint16_t reg);
  uint16_t (* pfReadRegister) (uint16_t reg);
  void (* pfWriteData) (uint16_t value);
  uint16_t (* pfReadData) (void);
#endif
  void (* pfInitialize) (GUI_DEVICE * pDevice);
} DRIVER_CONTEXT;

#ifdef SSD2119_REGISTER_ACCESS_HOOKS
  #define SET_REGISTER(context, reg) (context)->pfWriteRegister(reg)
  #define WRITE_REGISTER(context, reg, val) (context)->pfWriteRegister(reg); (context)->pfWriteData(val)
  #define READ_REGISTER(context, reg) (context)->pfReadRegister(reg)
  #define WRITE_DATA(context, val) (context)->pfWriteData(val)
  #define READ_DATA(context) (context)->pfReadData()
#else
  #define SET_REGISTER(context, reg) *((context)->commandAddr) = (reg)
  #define WRITE_REGISTER(context, reg, val) *((context)->commandAddr) = (reg); *((context)->dataAddr) = (val)
  #define READ_REGISTER(context, reg) *((context)->commandAddr) = (reg); *((context)->dataAddr)
  #define WRITE_DATA(context, val) *((context)->dataAddr) = (val)
  #define READ_DATA(context) *((context)->dataAddr)
#endif

DRIVER_CONTEXT *current_context;

#ifdef SSD2119_REGISTER_ACCESS_HOOKS
static void SSD2119_WriteRegister(uint16_t reg)
{
  *current_context->commandAddr = reg;
  return;
}

static uint16_t SSD2119_ReadRegister(uint16_t reg)
{
  *current_context->commandAddr = reg;
  return *current_context->dataAddr;
}

static void SSD2119_WriteData(uint16_t val)
{
  *current_context->dataAddr = val;
}

static uint16_t SSD2119_ReadData(void)
{
  return *current_context->dataAddr;
}
#endif

inline static void SSD2119_SetCurrentPosition(DRIVER_CONTEXT *context, int x, int y)
{
  /* Set pixel position */
  WRITE_REGISTER(context, DMD_SSD2119_SET_X_ADDRESS_COUNTER, x);
  WRITE_REGISTER(context, DMD_SSD2119_SET_Y_ADDRESS_COUNTER, y );
}

inline static void SSD2119_PrepareDataAccess(DRIVER_CONTEXT *context)
{
  SET_REGISTER(context, DMD_SSD2119_ACCESS_DATA);
}

static void SSD2119_SetClippingArea(DRIVER_CONTEXT *context, int x0, int y0, int x1, int y1)
{ uint16_t vClip;

  /* Set the clipping region in the display */
  WRITE_REGISTER(context, DMD_SSD2119_HORIZONTAL_RAM_ADDRESS_START_POS, x0);
  WRITE_REGISTER(context, DMD_SSD2119_HORIZONTAL_RAM_ADDRESS_END_POS, x1);

  vClip  = y1 << DMD_SSD2119_VERTICAL_RAM_ADDRESS_POS_END_SHIFT;
  vClip |= y0 << DMD_SSD2119_VERTICAL_RAM_ADDRESS_POS_START_SHIFT;
  WRITE_REGISTER(context, DMD_SSD2119_VERTICAL_RAM_ADDRESS_POS, vClip);
}
/** @endcond */

/**************************************************************************//**
*  @brief
*  Initializes the LCD display
*
*  @param pDevice GUI_DEVICE pointer.
******************************************************************************/
void SSD2119_Initialize(GUI_DEVICE * pDevice)
{
  uint16_t data;
  DRIVER_CONTEXT *context = (DRIVER_CONTEXT *)pDevice->u.pContext;

  /* Initialize register cache variables */
  //rcDriverOutputControl = 0;

  /* Initialization sequence, see UMSH-8252MD-T page 13 */

  /*  printf("R%x: 0x%x\n", DMD_SSD2119_VCOM_OTP_1, 0x0006); */
  WRITE_REGISTER(context, DMD_SSD2119_VCOM_OTP_1, 0x0006);

  /* Start oscillation */
  data = DMD_SSD2119_OSCILLATION_START_OSCEN;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_OSCILLATION_START, data); */
  WRITE_REGISTER(context, DMD_SSD2119_OSCILLATION_START, data);

  /* Exit sleep mode */
  data = 0;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_SLEEP_MODE_1, data); */
  WRITE_REGISTER(context, DMD_SSD2119_SLEEP_MODE_1, data);

  /* Display control */
  data  = DMD_SSD2119_DISPLAY_CONTROL_DTE;
  data |= DMD_SSD2119_DISPLAY_CONTROL_GON;
  data |= DMD_SSD2119_DISPLAY_CONTROL_D1;
  data |= DMD_SSD2119_DISPLAY_CONTROL_D0;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_DISPLAY_CONTROL, data); */
  WRITE_REGISTER(context, DMD_SSD2119_DISPLAY_CONTROL, data);

  /* Entry mode */
  data  = DMD_SSD2119_ENTRY_MODE_DFM_65K << DMD_SSD2119_ENTRY_MODE_DFM_SHIFT;
  data |= DMD_SSD2119_ENTRY_MODE_DENMODE;
  data |= DMD_SSD2119_ENTRY_MODE_WMODE;
  data |= DMD_SSD2119_ENTRY_MODE_NOSYNC;
  data |= DMD_SSD2119_ENTRY_MODE_TY_TYPE_B << DMD_SSD2119_ENTRY_MODE_TY_SHIFT;
  data |= DMD_SSD2119_ENTRY_MODE_ID1;
  data |= DMD_SSD2119_ENTRY_MODE_ID0;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_ENTRY_MODE, data); */
  WRITE_REGISTER(context, DMD_SSD2119_ENTRY_MODE, data);

  /* LCD AC control */
  data  = DMD_SSD2119_LCD_AC_CONTROL_BC;
  data |= DMD_SSD2119_LCD_AC_CONTROL_EOR;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_LCD_AC_CONTROL, data); */
  WRITE_REGISTER(context, DMD_SSD2119_LCD_AC_CONTROL, data);

  /* Power control */
  data  = 0x06 << DMD_SSD2119_POWER_CONTROL_1_DCT_SHIFT;
  data |= 0x05 << DMD_SSD2119_POWER_CONTROL_1_BT_SHIFT;
  data |= 0x03 << DMD_SSD2119_POWER_CONTROL_1_DC_SHIFT;
  data |= 0x04 << DMD_SSD2119_POWER_CONTROL_1_AP_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_POWER_CONTROL_1, data); */
  WRITE_REGISTER(context, DMD_SSD2119_POWER_CONTROL_1, data);

  /* Driver output control */
  data = 0;
  //data                 |= DMD_SSD2119_DRIVER_OUTPUT_CONTROL_RL;
  data                 |= DMD_SSD2119_DRIVER_OUTPUT_CONTROL_REV;
  data                 |= DMD_SSD2119_DRIVER_OUTPUT_CONTROL_GD;
  //data                 |= DMD_SSD2119_DRIVER_OUTPUT_CONTROL_TB;
  data                 |= (context->ySize - 1) << DMD_SSD2119_DRIVER_OUTPUT_CONTROL_MUX_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_DRIVER_OUTPUT_CONTROL, data); */
  WRITE_REGISTER(context, DMD_SSD2119_DRIVER_OUTPUT_CONTROL, data);

  /* Power Control */
  data = 0x05 << DMD_SSD2119_POWER_CONTROL_2_VRC_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_POWER_CONTROL_2, data); */
  WRITE_REGISTER(context, DMD_SSD2119_POWER_CONTROL_2, data);

  data = 0x0D << DMD_SSD2119_POWER_CONTROL_3_VRH_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_POWER_CONTROL_3, data); */
  WRITE_REGISTER(context, DMD_SSD2119_POWER_CONTROL_3, data);

  data  = DMD_SSD2119_POWER_CONTROL_4_VCOMG;
  data |= 0x0D << DMD_SSD2119_POWER_CONTROL_4_VDV_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_POWER_CONTROL_4, data); */
  WRITE_REGISTER(context, DMD_SSD2119_POWER_CONTROL_4, data);

  data  = DMD_SSD2119_POWER_CONTROL_5_NOTP;
  data |= 0x3E << DMD_SSD2119_POWER_CONTROL_5_VCM_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_POWER_CONTROL_5, data); */
  WRITE_REGISTER(context, DMD_SSD2119_POWER_CONTROL_5, data);

  data = 0x0058;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_GENERIC_INTERFACE_CONTROL, data); */
  WRITE_REGISTER(context, DMD_SSD2119_GENERIC_INTERFACE_CONTROL, data);

  /* Gamma settings */
  data  = 0x00 << DMD_SSD2119_GAMMA_1_PKP1_SHIFT;
  data |= 0x00 << DMD_SSD2119_GAMMA_1_PKP0_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_GAMMA_1, data); */
  WRITE_REGISTER(context, DMD_SSD2119_GAMMA_1, data);

  data  = 0x01 << DMD_SSD2119_GAMMA_2_PKP3_SHIFT;
  data |= 0x01 << DMD_SSD2119_GAMMA_2_PKP2_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_GAMMA_2, data); */
  WRITE_REGISTER(context, DMD_SSD2119_GAMMA_2, data);

  data  = 0x01 << DMD_SSD2119_GAMMA_3_PKP5_SHIFT;
  data |= 0x00 << DMD_SSD2119_GAMMA_3_PKP4_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_GAMMA_3, data); */
  WRITE_REGISTER(context, DMD_SSD2119_GAMMA_3, data);

  data  = 0x03 << DMD_SSD2119_GAMMA_4_PRP1_SHIFT;
  data |= 0x05 << DMD_SSD2119_GAMMA_4_PRP0_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_GAMMA_4, data); */
  WRITE_REGISTER(context, DMD_SSD2119_GAMMA_4, data);

  data  = 0x07 << DMD_SSD2119_GAMMA_5_PKN1_SHIFT;
  data |= 0x07 << DMD_SSD2119_GAMMA_5_PKN0_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_GAMMA_5, data); */
  WRITE_REGISTER(context, DMD_SSD2119_GAMMA_5, data);

  data  = 0x03 << DMD_SSD2119_GAMMA_6_PKN3_SHIFT;
  data |= 0x05 << DMD_SSD2119_GAMMA_6_PKN2_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_GAMMA_6, data); */
  WRITE_REGISTER(context, DMD_SSD2119_GAMMA_6, data);

  data  = 0x07 << DMD_SSD2119_GAMMA_7_PKN5_SHIFT;
  data |= 0x07 << DMD_SSD2119_GAMMA_7_PKN4_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_GAMMA_7, data); */
  WRITE_REGISTER(context, DMD_SSD2119_GAMMA_7, data);

  data  = 0x02 << DMD_SSD2119_GAMMA_8_PRN1_SHIFT;
  data |= 0x01 << DMD_SSD2119_GAMMA_8_PRN0_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_GAMMA_8, data); */
  WRITE_REGISTER(context, DMD_SSD2119_GAMMA_8, data);

  data  = 0x12 << DMD_SSD2119_GAMMA_9_VRP1_SHIFT;
  data |= 0x00 << DMD_SSD2119_GAMMA_9_VRP0_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_GAMMA_9, data); */
  WRITE_REGISTER(context, DMD_SSD2119_GAMMA_9, data);

  data  = 0x09 << DMD_SSD2119_GAMMA_10_VRN1_SHIFT;
  data |= 0x00 << DMD_SSD2119_GAMMA_10_VRN0_SHIFT;
  /*  printf("R%x: 0x%x\n", DMD_SSD2119_GAMMA_10, data); */
  WRITE_REGISTER(context, DMD_SSD2119_GAMMA_10, data);
}

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
/**************************************************************************//**
 * @brief
 *   Set pixel colour index
 * @details
 *   This function sets pixel at specified X,Y position to defined colour index.
 *   After leaving this function TFT controller is in data entry mode and X,Y
 *   indexes are incremented, so next data write to controller will result with
 *   setting next pixel colour. This feature could be used to consecutive pixels
 *   write in result speed-up transmision. Argument pDevice is present for
 *   compability with other display drivers.
 *****************************************************************************/
static void SSD2119_SetPixelIndex(GUI_DEVICE * pDevice, int x, int y, int PixelIndex)
{
  SSD2119_SetCurrentPosition((DRIVER_CONTEXT *)pDevice->u.pContext, x, y);
  SSD2119_PrepareDataAccess((DRIVER_CONTEXT *)pDevice->u.pContext);
  WRITE_DATA( (DRIVER_CONTEXT *)pDevice->u.pContext, PixelIndex);
}

/**************************************************************************//**
 * @brief
 *    Gets pixel colour index
 * @details
 *   This function gets pixel coulour at specified X,Y position.
 *   After leaving this function TFT controller is in data entry mode and X,Y
 *   indexes are equal to function arguments. So if there is need next pixel
 *   to be read, those indexes must be manually modified.
 *   Argument pDevice is present for compability with other display drivers.
 *****************************************************************************/
static unsigned int SSD2119_GetPixelIndex(GUI_DEVICE * pDevice, int x, int y)
{
  LCD_PIXELINDEX PixelIndex;

  SSD2119_SetCurrentPosition((DRIVER_CONTEXT *)pDevice->u.pContext, x, y);
  SSD2119_PrepareDataAccess((DRIVER_CONTEXT *)pDevice->u.pContext);
  /* first read after changing index is invalid */
  PixelIndex = READ_DATA((DRIVER_CONTEXT *)pDevice->u.pContext);
  PixelIndex = READ_DATA((DRIVER_CONTEXT *)pDevice->u.pContext);

  return PixelIndex;
}

/**************************************************************************//**
 * @brief
 *   Inverts colour of specified pixel
 * @details
 *   This function inverts pixel coulour at specified X,Y position.
 *   After leaving this function TFT controller is in data entry mode and X,Y
 *   indexes are incremented, so next data write to controller will result with
 *   setting next pixel colour. This feature could be used to consecutive pixels
 *   write in result speed-up transmision. Argument pDevice is present for
 *   compability with other display drivers.
 *****************************************************************************/
static void SSD2119_XorPixel(GUI_DEVICE * pDevice, int x, int y) {
  LCD_PIXELINDEX PixelIndex;
  LCD_PIXELINDEX IndexMask;

  PixelIndex = SSD2119_GetPixelIndex(pDevice, x, y);
  IndexMask  = pDevice->pColorConvAPI->pfGetIndexMask();
  SSD2119_SetPixelIndex(pDevice, x, y, PixelIndex ^ IndexMask);
}

/**************************************************************************//**
 * @brief
 *   Fill specified rectangle with current colour index
 * @details
 *   This function fills rectangle with its corner positions specified by
 *   arguments (x0, y0) and (x1, y1). To fill the rectangle current colour is
 *   used. As a side effect, function changes clipping area to entire display.
 *   Argument pDevice is present for compability with other display drivers.
 *****************************************************************************/
static void SSD2119_FillRect(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) {
  uint16_t PixelIndex;
  DRIVER_CONTEXT *context = (DRIVER_CONTEXT *)pDevice->u.pContext;

  int size;
  int xl, xh, yl, yh;
  if(x0<x1)
  { xl=x0;
    xh=x1;
  }else
  { xl=x1;
    xh=x0;
  }
  if(y0<y1)
  { yl=y0;
    yh=y1;
  }else
  { yl=y1;
    yh=y0;
  }
  switch (GUI_GetDrawMode() & LCD_DRAWMODE_XOR)
  { case 0:
      size = (xh-xl+1)*(yh-yl+1);
      SSD2119_SetClippingArea( context, x0, y0, x1, y1);
      SSD2119_SetCurrentPosition(context, xl, yl);
      PixelIndex = LCD__GetColorIndex();
      SSD2119_PrepareDataAccess(context);
      while(size--)
         WRITE_DATA(context, PixelIndex);
      SSD2119_SetClippingArea( context, 0, 0, context->xSize, context->ySize);
      break;
    case LCD_DRAWMODE_XOR:
      for(x0=xl;x0<=xh;x0++)
        for(y0=yl;y0<=yh;y0++)
          SSD2119_XorPixel(pDevice, x0, y0);
      break;
  }
}

/**************************************************************************//**
 * @brief
 *   draw horizontal line using current colour
 * @details
 *   This function draws horizontal line with using current colour.
 *   To achieve this functionality SSD2119_FillRect function is executed, so as a side
 *   effect, function changes clipping area to entire display.
 *   Argument pDevice is present for compability with other display drivers.
 *****************************************************************************/
static void SSD2119_DrawHLine(GUI_DEVICE * pDevice, int x0, int y,  int x1)
{
  SSD2119_FillRect(pDevice, x0, y, x1, y);
}

/**************************************************************************//**
 * @brief
 *   draw vertical line using current colour
 * @details
 *   This function draws vertical line with using current colour.
 *   To achieve this functionality SSD2119_FillRect function is executed, so as a side
 *   effect, function changes clipping area to entire display.
 *   Argument pDevice is present for compability with other display drivers.
 *****************************************************************************/
static void SSD2119_DrawVLine(GUI_DEVICE * pDevice, int x, int y0,  int y1)
{
  SSD2119_FillRect(pDevice, x, y0, x, y1);
}
/**************************************************************************//**
 * @brief
 *   draw one line of bitmap encoded in 1 bit per pixel
 * @details
 *   This function draws one line of 1BPP encoded bitmap. It is internally used
 *   by drawing entire bitmap routine. Depending on currend drawing mode the
 *   line could be simple setting pixels and background, setting only pixels
 *   with adequate bits set or inverting pixels when adequate bits set.
 *   Argument pDevice is present for compability with other display drivers.
 *****************************************************************************/
static void SSD2119_HorizontalLine1bpp(GUI_DEVICE * pDevice, unsigned x, unsigned y, uint8_t const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  unsigned Pixels, PixelCnt, invalidPosition = 1;
  LCD_PIXELINDEX Index, Index0, Index1, IndexMask;
  unsigned i, NumLoops;

  Index0   = *(pTrans + 0);
  Index1   = *(pTrans + 1);
  x       += Diff;
  PixelCnt = 8 - Diff;
  Pixels   = LCD_aMirror[*p] >> Diff;


  switch (GUI_GetDrawMode() & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    SSD2119_SetCurrentPosition((DRIVER_CONTEXT *)pDevice->u.pContext, x, y);
    SSD2119_PrepareDataAccess((DRIVER_CONTEXT *)pDevice->u.pContext);
    do {
      NumLoops = GUI_MIN(PixelCnt, (unsigned)xsize);
      i = NumLoops;
      do {
        Index = (Pixels & 1) ? Index1 : Index0;
        WRITE_DATA((DRIVER_CONTEXT *)pDevice->u.pContext, Index);
        Pixels >>= 1;
      } while (--i);
      PixelCnt -= NumLoops;
      xsize -= NumLoops;
      if (PixelCnt == 0) {
        Pixels   = LCD_aMirror[*(++p)];
        PixelCnt = 8;
      }
    } while (xsize);
    break;
  case LCD_DRAWMODE_TRANS:
    do {
      if (Pixels & 1) {
        if(invalidPosition)
        { SSD2119_SetCurrentPosition((DRIVER_CONTEXT *)pDevice->u.pContext, x, y);
          SSD2119_PrepareDataAccess((DRIVER_CONTEXT *)pDevice->u.pContext);
          invalidPosition=0;
        }
        WRITE_DATA((DRIVER_CONTEXT *)pDevice->u.pContext, Index1);
      } else
      { invalidPosition=1;
      }
      x++;
      PixelCnt--;
      Pixels >>= 1;
      if (PixelCnt == 0) {
        Pixels   = LCD_aMirror[*(++p)];
        PixelCnt = 8;
      }
    } while (--xsize);
    break;
  case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
  case LCD_DRAWMODE_XOR:
    IndexMask = pDevice->pColorConvAPI->pfGetIndexMask();
    do {
      SSD2119_SetCurrentPosition((DRIVER_CONTEXT *)pDevice->u.pContext, x, y);
      SSD2119_PrepareDataAccess((DRIVER_CONTEXT *)pDevice->u.pContext);
      /* dummy read required by LCD controller */
      Index = READ_DATA((DRIVER_CONTEXT *)pDevice->u.pContext);
      Index = READ_DATA((DRIVER_CONTEXT *)pDevice->u.pContext);
      if (Pixels & 1) {
        Index ^= IndexMask;
      }
      WRITE_DATA((DRIVER_CONTEXT *)pDevice->u.pContext, Index);
      x++;
      PixelCnt--;
      Pixels >>= 1;
      if (PixelCnt == 0) {
        Pixels   = LCD_aMirror[*(++p)];
        PixelCnt = 8;
      }
    } while (--xsize);
    break;
  }
}

/**************************************************************************//**
 * @brief
 *   draw one line of bitmap encoded in 2 bits per pixel
 * @details
 *   This function draws one line of 2BPP encoded bitmap. It is internally used
 *   by drawing entire bitmap routine. Depending on currend drawing mode the
 *   line could be simple setting pixels and background or setting only pixels
 *   with adequate bits set.
 *   Argument pDevice is present for compability with other display drivers.
 *****************************************************************************/
static void  SSD2119_HorizontalLine2bpp(GUI_DEVICE * pDevice, int x, int y, uint8_t const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels, PixelIndex;
  int CurrentPixel, Shift, Index;
  Pixels       = *p;
  CurrentPixel = Diff;
  x           += Diff;
  switch (GUI_GetDrawMode() & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    if (pTrans) {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        PixelIndex = *(pTrans + Index);
        SSD2119_SetPixelIndex(pDevice, x++, y, PixelIndex);
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
		  } while (--xsize);
    } else {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        SSD2119_SetPixelIndex(pDevice, x++, y, Index);
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
		  } while (--xsize);
    }
    break;
  case LCD_DRAWMODE_TRANS:
    if (pTrans) {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        if (Index) {
          PixelIndex = *(pTrans + Index);
          SSD2119_SetPixelIndex(pDevice, x, y, PixelIndex);
        }
        x++;
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
		  } while (--xsize);
    } else {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        if (Index) {
          SSD2119_SetPixelIndex(pDevice, x, y, Index);
        }
        x++;
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
		  } while (--xsize);
    }
    break;
  }
}

/**************************************************************************//**
 * @brief
 *   draw one line of bitmap encoded in 4 bits per pixel
 * @details
 *   This function draws one line of 4BPP encoded bitmap. It is internally used
 *   by drawing entire bitmap routine. Depending on currend drawing mode the
 *   line could be simple setting pixels and background or setting only pixels
 *   with adequate bits set.
 *   Argument pDevice is present for compability with other display drivers.
 *****************************************************************************/
static void  SSD2119_HorizontalLine4bpp(GUI_DEVICE * pDevice, int x, int y, uint8_t const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels, PixelIndex;
  int CurrentPixel, Shift, Index;
  Pixels       = *p;
  CurrentPixel = Diff;
  x           += Diff;
  switch (GUI_GetDrawMode() & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    if (pTrans) {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        PixelIndex = *(pTrans + Index);
        SSD2119_SetPixelIndex(pDevice, x++, y, PixelIndex);
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
		  } while (--xsize);
    } else {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        SSD2119_SetPixelIndex(pDevice, x++, y, Index);
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
		  } while (--xsize);
    }
    break;
  case LCD_DRAWMODE_TRANS:
    if (pTrans) {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        if (Index) {
          PixelIndex = *(pTrans + Index);
          SSD2119_SetPixelIndex(pDevice, x, y, PixelIndex);
        }
        x++;
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
		  } while (--xsize);
    } else {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        if (Index) {
          SSD2119_SetPixelIndex(pDevice, x, y, Index);
        }
        x++;
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
		  } while (--xsize);
    }
    break;
  }
}

/**************************************************************************//**
 * @brief
 *   draw one line of bitmap encoded in 8 bits per pixel
 * @details
 *   This function draws one line of 8BPP encoded bitmap. It is internally used
 *   by drawing entire bitmap routine. Depending on currend drawing mode the
 *   line could be simple setting pixels and background or setting only pixels
 *   with adequate bits set.
 *   Argument pDevice is present for compability with other display drivers.
 *****************************************************************************/
static void  SSD2119_HorizontalLine8bpp(GUI_DEVICE * pDevice, int x, int y, uint8_t const GUI_UNI_PTR * p, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixel;
  uint16_t * pDest;

  if (!pTrans) {
    return; /* No translation from 8bpp BMP to 16bpp device makes no sense */
  }
  SSD2119_SetCurrentPosition((DRIVER_CONTEXT *)pDevice->u.pContext, x, y);
  SSD2119_PrepareDataAccess((DRIVER_CONTEXT *)pDevice->u.pContext);
  switch (GUI_GetDrawMode() & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    while(xsize--)
    {
      WRITE_DATA((DRIVER_CONTEXT *)pDevice->u.pContext, *(pTrans + *p));
    }
    break;
  case LCD_DRAWMODE_TRANS:
    for (; xsize > 0; xsize--, p++, pDest++) {

      Pixel = *p;
      if (Pixel) {
        WRITE_DATA((DRIVER_CONTEXT *)pDevice->u.pContext, *(pTrans + *p));
      } else
      { uint16_t Index;
      /* we do this just to increment internal pointer */
        Index = READ_DATA((DRIVER_CONTEXT *)pDevice->u.pContext);
        Index = READ_DATA((DRIVER_CONTEXT *)pDevice->u.pContext);
        WRITE_DATA((DRIVER_CONTEXT *)pDevice->u.pContext, Index);
      }
    }
    break;
  }
}

/**************************************************************************//**
 * @brief
 *   draw one line of bitmap encoded in 16 bits per pixel
 * @details
 *   This function draws one line of 16BPP encoded bitmap. It is internally used
 *   by drawing entire bitmap routine.
 *   Argument pDevice is present for compability with other display drivers.
 *****************************************************************************/
static void SSD2119_HorizontalLine16bpp(GUI_DEVICE * pDevice, int x, int y, uint16_t const GUI_UNI_PTR *p, int xsize) {

  SSD2119_SetCurrentPosition((DRIVER_CONTEXT *)pDevice->u.pContext, x, y);
  SSD2119_PrepareDataAccess((DRIVER_CONTEXT *)pDevice->u.pContext);
  while(xsize--)
  {
    WRITE_DATA((DRIVER_CONTEXT *)pDevice->u.pContext, *p++);
  }
}

/**************************************************************************//**
 * @brief
 *   draw bitmap
 * @details
 *   This function draws bitmap encoded in one of supported formats.
 *   Argument pDevice is present for compability with other display drivers.
 *****************************************************************************/
static void SSD2119_DrawBitmap(GUI_DEVICE * pDevice, int x0, int y0,
                       int xSize, int ySize,
                       int BitsPerPixel,
                       int BytesPerLine,
                       const uint8_t GUI_UNI_PTR * pData, int Diff,
                       const LCD_PIXELINDEX* pTrans) {
  int i;

  switch (BitsPerPixel) {
  case 1:
    for (i = 0; i < ySize; i++) {
      SSD2119_HorizontalLine1bpp(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 2:
    for (i = 0; i < ySize; i++) {
      SSD2119_HorizontalLine2bpp(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 4:
    for (i = 0; i < ySize; i++) {
      SSD2119_HorizontalLine4bpp(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 8:
    for (i = 0; i < ySize; i++) {
      SSD2119_HorizontalLine8bpp(pDevice, x0, i + y0, pData, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 16:
    for (i = 0; i < ySize; i++) {
      SSD2119_HorizontalLine16bpp(pDevice, x0, i + y0, (const uint16_t *)pData, xSize);
      pData += BytesPerLine;
    }
    break;
  }
}

/**************************************************************************//**
 * @brief
 *   Return device property
 * @details
 *   This function returns requested driver or device property
 *****************************************************************************/
static I32 SSD2119_GetDevProp(GUI_DEVICE * pDevice, int Index) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  switch (Index) {
  case LCD_DEVCAP_XSIZE:
    return pContext->xSize;
  case LCD_DEVCAP_YSIZE:
    return pContext->ySize;
  case LCD_DEVCAP_VXSIZE:
    return pContext->vxSize;
  case LCD_DEVCAP_VYSIZE:
    return pContext->vySize;
  case LCD_DEVCAP_BITSPERPIXEL:
    return 16;
  case LCD_DEVCAP_NUMCOLORS:
    return 0;
  case LCD_DEVCAP_XMAG:
    return 1;
  case LCD_DEVCAP_YMAG:
    return 1;
  case LCD_DEVCAP_MIRROR_X:
    return 0;
  case LCD_DEVCAP_MIRROR_Y:
    return 0;
  case LCD_DEVCAP_SWAP_XY:
    return 0;
  }
  return -1;
}

/**************************************************************************//**
 * @brief
 *    Return MEMDEV handler
*****************************************************************************/
static void * SSD2119_GetDevData(GUI_DEVICE * pDevice, int Index) {
  GUI_USE_PARA(pDevice);
  #if GUI_SUPPORT_MEMDEV
    switch (Index) {
    case LCD_DEVDATA_MEMDEV:
      return (void *)&GUI_MEMDEV_DEVICE_16;
    }
  #else
    GUI_USE_PARA(Index);
  #endif
  return NULL;
}

/***************************************************************************//**
 * @brief
 *   Set LCD controller register and data address
 *
 * @param[in] pDevice- Device driver pointer
 * @param[in] commandAddr - controller registers address
 * @param[in] dataAddr - controller data address
 *
 ******************************************************************************/
static void _SetControllerAddres(GUI_DEVICE * pDevice, uint32_t commandAddr, uint32_t dataAddr)
{
  DRIVER_CONTEXT *context = (DRIVER_CONTEXT *)pDevice->u.pContext;

  context->commandAddr = (uint16_t *)commandAddr;
  context->dataAddr = (uint16_t *)dataAddr;
}

/***************************************************************************//**
 * @brief
 *   Initializes driver context structure
 *
 * @param[in] pDevice
 *   Device driver pointer
 *
 * @return
 *   @li 0 if success
 *   @li 1 when error
 ******************************************************************************/
static int _InitOnce(GUI_DEVICE * pDevice) {
DRIVER_CONTEXT *context;
  if (pDevice->u.pContext == NULL) {
    pDevice->u.pContext = GUI_ALLOC_GetFixedBlock(sizeof(DRIVER_CONTEXT));
    context = (DRIVER_CONTEXT *)pDevice->u.pContext;
    if(context)
    {
      GUI__memset((uint8_t *)pDevice->u.pContext, 0, sizeof(DRIVER_CONTEXT));
#ifdef SSD2119_REGISTER_ACCESS_HOOKS
      context->pfWriteRegister = SSD2119_WriteRegister;
      context->pfReadRegister = SSD2119_ReadRegister;
      context->pfWriteData = SSD2119_WriteData;
      context->pfReadData = SSD2119_ReadData;
#endif
      context->pfInitialize = SSD2119_Initialize;
      current_context = context;
      /* set default controller address, could be overided */
      _SetControllerAddres(pDevice, 0x84000000, 0x84000002);
    }
  }
  return pDevice->u.pContext ? 0 : 1;
}

/**************************************************************************//**
 * @brief
 *   read pixels in defined rectangle
 * @details
 *   This function reads display framebuffer and returns colour of pixels in
 *   defined rectangle.
 *****************************************************************************/
static void SSD2119_ReadRectangle(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1, LCD_PIXELINDEX * pBuffer) {
  int x, y;
  uint16_t * p;

  p = (uint16_t*) pBuffer;
  for(y=y0;y<=y1;y++)
     for(x=x0;x<=x1;x++)
       *p++ = SSD2119_GetPixelIndex(pDevice, x, y);
}

/**************************************************************************//**
 * @brief
 *   sets video buffer address (kept for compability reason)
 *
 * @param[in] pDevice - Device driver pointer
 * @param[in] pVRAM - pointer to video RAM
 *****************************************************************************/
static void _SetVRAMAddr(GUI_DEVICE * pDevice, void * pVRAM) {
  DRIVER_CONTEXT * pContext;
  LCD_X_SETVRAMADDR_INFO Data = {0};

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->VRAMAddr = pContext->BaseAddr = (uint32_t)pVRAM;
    Data.pVRAM = pVRAM;
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETVRAMADDR, (void *)&Data);
  }
  #ifdef WIN32
    SIM_Lin_SetVRAMAddr(pDevice->LayerIndex, pVRAM);
  #endif
}

/*********************************************************************
*
*       _SetOrg
*
* Purpose:
*   Calls the driver callback function with the display origin to be set
*/
static void _SetOrg(GUI_DEVICE * pDevice, int x, int y) {
  #ifndef WIN32
    DRIVER_CONTEXT * pContext;
    int Orientation;
  #endif
  LCD_X_SETORG_INFO Data = {0, 0};

  #ifdef WIN32
    LCDSIM_SetOrg(x, y, pDevice->LayerIndex);
  #else
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    Orientation  = LCD_GetMirrorXEx(pDevice->LayerIndex) * GUI_MIRROR_X;
    Orientation |= LCD_GetMirrorYEx(pDevice->LayerIndex) * GUI_MIRROR_Y;
    Orientation |= LCD_GetSwapXYEx (pDevice->LayerIndex) * GUI_SWAP_XY;
    switch (Orientation) {
    case 0:
      Data.xPos = x;
      Data.yPos = y;
      break;
    case GUI_MIRROR_X:
      Data.xPos = pContext->vxSize - pContext->xSize - x;
      Data.yPos = y;
      break;
    case GUI_MIRROR_Y:
      Data.xPos = x;
      Data.yPos = pContext->vySize - pContext->ySize - y;
      break;
    case GUI_MIRROR_X | GUI_MIRROR_Y:
      Data.xPos = pContext->vxSize - pContext->xSize - x;
      Data.yPos = pContext->vySize - pContext->ySize - y;
      break;
    case GUI_SWAP_XY:
      Data.xPos = y;
      Data.yPos = x;
      break;
    case GUI_SWAP_XY | GUI_MIRROR_X:
      Data.xPos = pContext->vySize - pContext->ySize  - y;
      Data.yPos = x;
      break;
    case GUI_SWAP_XY | GUI_MIRROR_Y:
      Data.xPos = y;
      Data.yPos = pContext->vxSize - pContext->xSize  - x;
      break;
    case GUI_SWAP_XY | GUI_MIRROR_X | GUI_MIRROR_Y:
      Data.xPos = pContext->vySize - pContext->ySize  - y;
      Data.yPos = pContext->vxSize - pContext->xSize  - x;
      break;
    }
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETORG, (void *)&Data);
  #endif
}

/*********************************************************************
*
*       _GetRect
*
* Purpose:
*   Returns the display size.
*/
static void _GetRect(GUI_DEVICE * pDevice, LCD_RECT * pRect) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pRect->x0 = 0;
  pRect->y0 = 0;
  pRect->x1 = pContext->vxSize - 1;
  pRect->y1 = pContext->vySize - 1;
}

/*********************************************************************
*
*       _SetVSize
*/
static void _SetVSize(GUI_DEVICE * pDevice, int xSize, int ySize) {
  DRIVER_CONTEXT * pContext;
  #ifdef WIN32
    int NumBuffers;
  #endif

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    #ifdef WIN32
      NumBuffers = GUI_MULTIBUF_GetNumBuffers();
    #endif
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    if (LCD_GetSwapXYEx(pDevice->LayerIndex)) {
      #ifdef WIN32
        pContext->vxSize = xSize * NumBuffers;
      #else
        pContext->vxSize = xSize;
      #endif
      pContext->vySize = ySize;
      pContext->vxSizePhys = ySize;
    } else {
      pContext->vxSize = xSize;
      #ifdef WIN32
        pContext->vySize = ySize * NumBuffers;
      #else
        pContext->vySize = ySize;
      #endif
      pContext->vxSizePhys = xSize;
    }
  }
  #ifdef WIN32
    SIM_Lin_SetVRAMSize(pDevice->LayerIndex, pContext->vxSize, pContext->vySize, pContext->xSize, pContext->ySize);
  #endif
}

/*********************************************************************
*
*       _SetSize
*/
static void _SetSize(GUI_DEVICE * pDevice, int xSize, int ySize) {
  DRIVER_CONTEXT * pContext;
  LCD_X_SETSIZE_INFO Data = {0, 0};

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    if (pContext->vxSizePhys == 0) {
      if (LCD_GetSwapXYEx(pDevice->LayerIndex)) {
        pContext->vxSizePhys = ySize;
      } else {
        pContext->vxSizePhys = xSize;
      }
    }
    pContext->xSize = xSize;
    pContext->ySize = ySize;
    Data.xSize = xSize;
    Data.ySize = ySize;
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETSIZE, (void *)&Data);
  }
}

/*********************************************************************
*
*       _SetPos
*
* Purpose:
*   Sets the position of the given layer by sending a LCD_X_SETPOS command to LCD_X_DisplayDriver()
*   (Requires special hardware support.)
*/
static void _SetPos(GUI_DEVICE * pDevice, int xPos, int yPos) {
  DRIVER_CONTEXT * pContext;
  LCD_X_SETPOS_INFO Data = {0, 0};

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->xPos = xPos;
    pContext->yPos = yPos;
    Data.xPos = xPos;
    Data.yPos = yPos;
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETPOS, (void *)&Data);
  }
}

/*********************************************************************
*
*       _GetPos
*
* Purpose:
*   Returns the position of the given layer.
*/
static void _GetPos(GUI_DEVICE * pDevice, int * pxPos, int * pyPos) {
  DRIVER_CONTEXT * pContext;

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    *pxPos = pContext->xPos;
    *pyPos = pContext->yPos;
  }
}

/*********************************************************************
*
*       _SetAlpha
*
* Purpose:
*   Sets the alpha value of the given layer by sending a LCD_X_SETALPHA command to LCD_X_DisplayDriver()
*   (Requires special hardware support.)
*/
static void _SetAlpha(GUI_DEVICE * pDevice, int Alpha) {
  DRIVER_CONTEXT * pContext;
  LCD_X_SETALPHA_INFO Data = {0};

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->Alpha = Alpha;
    Data.Alpha = Alpha;
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETALPHA, (void *)&Data);
  }
}

/*********************************************************************
*
*       _SetVis
*
* Purpose:
*   Sets the visibility of the given layer by sending a LCD_X_SETVIS command to LCD_X_DisplayDriver()
*   (Requires special hardware support.)
*/
static void _SetVis(GUI_DEVICE * pDevice, int OnOff) {
  DRIVER_CONTEXT * pContext;
  LCD_X_SETVIS_INFO Data = {0};

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->IsVisible = OnOff;
    Data.OnOff = OnOff;
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETVIS, (void *)&Data);
  }
}

/*********************************************************************
*
*       _Init
*
* Purpose:
*   Called during the initialization process of emWin.
*/
static int  _Init(GUI_DEVICE * pDevice) {
  int r;
  DRIVER_CONTEXT *pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;

  r = _InitOnce(pDevice);
  r |= LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_INITCONTROLLER, NULL);
  if(pContext->pfInitialize)
    pContext->pfInitialize(pDevice);
  return r;
}

/*********************************************************************
*
*       _On
*
* Purpose:
*   Sends a LCD_X_ON command to LCD_X_DisplayDriver().
*/
static void _On (GUI_DEVICE * pDevice) {
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_ON, NULL);
}

/*********************************************************************
*
*       _Off
*
* Purpose:
*   Sends a LCD_X_OFF command to LCD_X_DisplayDriver().
*/
static void _Off (GUI_DEVICE * pDevice) {
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_OFF, NULL);
}

/*********************************************************************
*
*       _SetAlphaMode
*
* Purpose:
*   Sets the alpha mode of the given layer by sending a LCD_X_SETALPHAMODE command to LCD_X_DisplayDriver()
*   (Requires special hardware support.)
*/
static void _SetAlphaMode(GUI_DEVICE * pDevice, int AlphaMode) {
  LCD_X_SETALPHAMODE_INFO Data = {0};

  Data.AlphaMode = AlphaMode;
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETALPHAMODE, (void *)&Data);
}

/*********************************************************************
*
*       _SetChromaMode
*
* Purpose:
*   Sets the chroma mode of the given layer by sending a LCD_X_SETCHROMAMODE command to LCD_X_DisplayDriver()
*   (Requires special hardware support.)
*/
static void _SetChromaMode(GUI_DEVICE * pDevice, int ChromaMode) {
  LCD_X_SETCHROMAMODE_INFO Data = {0};

  Data.ChromaMode = ChromaMode;
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETCHROMAMODE, (void *)&Data);
}

/*********************************************************************
*
*       _SetChroma
*
* Purpose:
*   Sets the chroma values of the given layer by sending a LCD_X_SETCHROMA command to LCD_X_DisplayDriver()
*   (Requires special hardware support.)
*/
static void _SetChroma(GUI_DEVICE * pDevice, LCD_COLOR ChromaMin, LCD_COLOR ChromaMax) {
  LCD_X_SETCHROMA_INFO Data = {0, 0};

  Data.ChromaMin = ChromaMin;
  Data.ChromaMax = ChromaMax;
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETCHROMA, (void *)&Data);
}

/*********************************************************************
*
*       _CopyBuffer
*
* Purpose:
*   Copies the source buffer to the destination buffer and routes
*   further drawing operations to the destination buffer.
*
*   (Required for using multiple buffers)
*/
static void _CopyBuffer(GUI_DEVICE * pDevice, int IndexSrc, int IndexDst) {
  DRIVER_CONTEXT * pContext;
  #ifndef WIN32
    uint32_t AddrSrc, AddrDst;
    I32 BufferSize;
    int BitsPerPixel;
  #endif

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    if (IndexSrc != IndexDst) {
      #ifdef WIN32
        SIM_Lin_CopyBuffer(IndexSrc, IndexDst);
      #else
        BitsPerPixel = pDevice->pDeviceAPI->pfGetDevProp(pDevice, LCD_DEVCAP_BITSPERPIXEL);
        BufferSize = (((uint32_t)pContext->xSize * pContext->ySize * BitsPerPixel) >> 3);
        AddrSrc = pContext->BaseAddr + BufferSize * IndexSrc;
        AddrDst = pContext->BaseAddr + BufferSize * IndexDst;
        if (pContext->pfCopyBuffer) {
          //
          // Use custom callback function for copy operation
          //
          pContext->pfCopyBuffer(pDevice->LayerIndex, IndexSrc, IndexDst);
        } else {
          //
          // Calculate pointers for copy operation
          //
          GUI_MEMCPY((void *)AddrDst, (void *)AddrSrc, BufferSize);
        }
        //
        // Set destination buffer as target for further drawing operations
        //
        pContext->VRAMAddr = AddrDst;
      #endif
    }
  }
}

/*********************************************************************
*
*       _ShowBuffer
*
* Purpose:
*   Sends a LCD_X_SHOWBUFFER command to LCD_X_DisplayDriver() to make the given buffer visible.
*
*   (Required for using multiple buffers)
*/
static void _ShowBuffer(GUI_DEVICE * pDevice, int Index) {
  LCD_X_SHOWBUFFER_INFO Data = {0};

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    #ifdef WIN32
      SIM_Lin_ShowBuffer(Index);
    #else
      Data.Index = Index;
      LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SHOWBUFFER, (void *)&Data);
    #endif
  }
}

/*********************************************************************
*
*       _SetFunc
*
* Purpose:
*   Setting of function pointers.
*/
static void _SetFunc(GUI_DEVICE * pDevice, int Index, void (* pFunc)(void)) {
  DRIVER_CONTEXT * pContext;

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    switch (Index) {
    case LCD_DEVFUNC_COPYBUFFER:
      pContext->pfCopyBuffer = (void (*)(int LayerIndex, int IndexSrc, int IndexDst))pFunc;
      break;
    case LCD_DEVFUNC_FILLRECT:
      pContext->pfFillRect   = (void (*)(int LayerIndex, int x0, int y0, int x1, int y1, uint32_t PixelIndex))pFunc;
      break;
    case LCD_DEVFUNC_DRAWBMP_1BPP:
      pContext->pfDrawBMP1   = (void (*)(int LayerIndex, int x, int y, uint8_t const * p, int Diff, int xSize, int ySize, int BytesPerLine, const LCD_PIXELINDEX * pTrans))pFunc;
      break;
    case LCD_DEVFUNC_COPYRECT:
      pContext->pfCopyRect   = (void (*)(int LayerIndex, int x0, int y0, int x1, int y1, int xSize, int ySize))pFunc;
      break;
#ifdef SSD2119_REGISTER_ACCESS_HOOKS
    case LCD_DEVFUNC_WRITEREGISTER:
      pContext->pfWriteRegister   =  (void (*) (uint16_t reg))pFunc;
      break;
    case LCD_DEVFUNC_READREGISTER:
      pContext->pfReadRegister   =  (uint16_t (*) (uint16_t reg))pFunc;
      break;
    case LCD_DEVFUNC_WRITEDATA:
      pContext->pfWriteData   =  (void (*) (uint16_t value))pFunc;
      break;
    case LCD_DEVFUNC_READDATA:
      pContext->pfReadData   =  (uint16_t (*) (void))pFunc;
#endif
    case LCD_DEVFUNC_INITIALIZE:
      pContext->pfInitialize = (void (*) (GUI_DEVICE *))pFunc;
      break;
    }
  }
}

/**************************************************************************//**
 * @brief
 *   return defined function handler
 * @details
 *   Specified function handler is returned
 *****************************************************************************/
static void (* SSD2119_GetDevFunc(GUI_DEVICE ** ppDevice, int Index))(void) {
  GUI_USE_PARA(ppDevice);
  switch (Index)
  {
  case LCD_DEVFUNC_SET_VRAM_ADDR:
    return (void (*)(void))_SetVRAMAddr;
  case LCD_DEVFUNC_SET_VSIZE:
    return (void (*)(void))_SetVSize;
  case LCD_DEVFUNC_SET_SIZE:
    return (void (*)(void))_SetSize;
  case LCD_DEVFUNC_SETPOS:
    return (void (*)(void))_SetPos;
  case LCD_DEVFUNC_GETPOS:
    return (void (*)(void))_GetPos;
  case LCD_DEVFUNC_SETALPHA:
    return (void (*)(void))_SetAlpha;
  case LCD_DEVFUNC_SETVIS:
    return (void (*)(void))_SetVis;
  case LCD_DEVFUNC_INIT:
    return (void (*)(void))_Init;
  case LCD_DEVFUNC_ON:
    return (void (*)(void))_On;
  case LCD_DEVFUNC_OFF:
    return (void (*)(void))_Off;
  case LCD_DEVFUNC_ALPHAMODE:
    return (void (*)(void))_SetAlphaMode;
  case LCD_DEVFUNC_CHROMAMODE:
    return (void (*)(void))_SetChromaMode;
  case LCD_DEVFUNC_CHROMA:
    return (void (*)(void))_SetChroma;
  case LCD_DEVFUNC_COPYBUFFER:
    return (void (*)(void))_CopyBuffer;
  case LCD_DEVFUNC_SHOWBUFFER:
    return (void (*)(void))_ShowBuffer;
  case LCD_DEVFUNC_SETFUNC:
    return (void (*)(void))_SetFunc;
  case LCD_DEVFUNC_COPYRECT:
    return (void (*)(void))((DRIVER_CONTEXT *)(*ppDevice)->u.pContext)->pfCopyRect;
  case LCD_DEVFUNC_READRECT:
    return (void (*)(void))SSD2119_ReadRectangle;
  case LCD_DEVFUNC_CONTRADDR:
    return (void (*)(void))_SetControllerAddres;
  }
  return (void (*)(void))NULL;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_DEVICE_API structure
*/
const GUI_DEVICE_API GUIDRV_SSD2119 =
{
  DEVICE_CLASS_DRIVER,
  SSD2119_DrawBitmap,
  SSD2119_DrawHLine,
  SSD2119_DrawVLine,
  SSD2119_FillRect,
  SSD2119_GetPixelIndex,
  SSD2119_SetPixelIndex,
  SSD2119_XorPixel,
  _SetOrg,
  SSD2119_GetDevFunc,
  SSD2119_GetDevProp,
  SSD2119_GetDevData,
  _GetRect,
};

#else

void GUIDRV_SSD2119(void);   // Avoid empty object files
void GUIDRV_SSD2119(void) {}

#endif

/** @endcond */

/*************************** End of file ****************************/
