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

** emWin V5.14 - Graphical user interface for embedded applications **
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
File        : GUI_X.C
Purpose     : Config / System dependent externals for GUI
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "WM.h"
#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_msc.h"
#include "em_gpio.h"
#include "rtcdrv.h"
#include "em_rtc.h"
#include "bsp.h"
#include "LCDConf.h"
#include "touch.h"
#include "rtcdrv.h"

#define JOYSTICK_DOWN  (1uL << 0)
#define JOYSTICK_RIGHT (1uL << 1)
#define JOYSTICK_UP    (1uL << 2)
#define JOYSTICK_LEFT  (1uL << 3)
#define JOYSTICK_ENTER (1uL << 4)
#define JOYSTICK_MOVE_PRECISION 10

#define RTC_FREQ    32768               /**< RTC Frequency 32.768 kHz */
#define RTC_MS_SHIFT 5                  /**< system timer unit */
#define RTC_HIGHEST_BIT (_RTC_COMP0_COMP0_MASK ^ (_RTC_COMP0_COMP0_MASK>>1))
/*********************************************************************
*
*       Global data
*/

int                       aem_mode = 0;               /**< Flag showing previous AEM state */
volatile bool             rtcFlag;                    /**< Flag used by the RTC timing routines */
int                       prevJoy = 0;                /**< Previous joystick state */
int                       lastJoyTime = 0;            /**< Time when previously joystick state was checked */
static int                time = 0;

/***************************************************************************//**
*   @brief
*      RTC Interrupt Handler, invoke callback if defined.
*     The interrupt table is in assembly startup file startup_efm32.s
*******************************************************************************/
void RTC_IRQHandler(void)
{

  /* Clear interrupt source */
  RTC_IntClear(RTC_IF_COMP0);

  /* Disable interrupt */
  RTC_IntDisable(RTC_IF_COMP0);

  /* Trigger callback if defined */
  rtcFlag = false;
}

/***************************************************************************//**
*     @brief
*       returns system time in milisecond unit.
*     @details
*       This function returns system time. The unit is 1/1024 of second due to
*       fact that RTC is used for counting time.
 ******************************************************************************/
int GUI_X_GetTime(void)
{ int timeNow = RTC_CounterGet()>>RTC_MS_SHIFT;

  if( (timeNow  ^ time) & (RTC_HIGHEST_BIT>>RTC_MS_SHIFT) )
  { /* RTC counter overload, increase virtual counter bits */
    time |= _RTC_COMP0_COMP0_MASK >> RTC_MS_SHIFT;
    time ++;
    time |= timeNow;
  } else
  { /* RTC counter did not overloaded since last call */
    time &= ~(_RTC_COMP0_COMP0_MASK >> RTC_MS_SHIFT);
    time |= timeNow;
  }
  return time;
}

/***************************************************************************//**
*     @brief
*       check joystick state
*     @details
*       If joystick is shifted cursor position is moved according to time spend
*       in delay function (ms argument provides this information). Also when
*       joystick press or release is detected adequate message is sent to emWin.
 ******************************************************************************/
static void _serveJoystick(void)
{
  int joy, Max;
  int ms = GUI_X_GetTime() - lastJoyTime;
  GUI_PID_STATE curPos;      /**< cursor position */

  if(ms>500)
    ms = 500;
  joy = BSP_JoystickGet();
  lastJoyTime = GUI_X_GetTime();

  if( joy || (joy != prevJoy) )
  { /* execute this action if joy is in use or just released */
    GUI_PID_GetState(&curPos);
    if (joy & JOYSTICK_LEFT)
      curPos.x -= 1 + ms/JOYSTICK_MOVE_PRECISION;
    if (joy & JOYSTICK_RIGHT)
      curPos.x += 1 + ms/JOYSTICK_MOVE_PRECISION;
    if (joy & JOYSTICK_UP)
      curPos.y -= 1 + ms/JOYSTICK_MOVE_PRECISION;
    if (joy & JOYSTICK_DOWN)
      curPos.y += 1 + ms/JOYSTICK_MOVE_PRECISION;
    //
    // Make sure coordinates are still in bounds
    //
    if (curPos.x < 1)
      curPos.x = 1;
    if (curPos.y < 1)
      curPos.y = 1;
    Max = LCD_GetXSize() - 1;
    if ( curPos.x >= Max )
      curPos.x = Max;
    Max = LCD_GetYSize() - 1;
    if (curPos.y > Max)
      curPos.y = Max;
    //
    // Inform emWin
    //
    curPos.Pressed = (joy & JOYSTICK_ENTER) ? 1 : 0;
    GUI_PID_StoreState(&curPos);

    prevJoy = joy;
  }
}

/***************************************************************************//**
*     @brief
*       Callback used when screen needs redrawing
*     @details
*       This function is used as a calback during redrawing all screen elements.
 ******************************************************************************/
static void _cbInvalidateWindow(WM_HWIN hWin, void *p)
{
  (void)p;                              /* Unused parameter */
  WM_InvalidateWindow(hWin);
}

/***************************************************************************//**
*     @brief
*       reinitializes LCD if Advanced Energy Monitor was enabled previously
*     @details
*       If user enables AEM on DK, LCD must be reinitialized and redrawn. This
*       function checks AEM state and if detects that it is just disabled,
*       reinitializes LCD controller and redraw LCD content.
 ******************************************************************************/
static void _checkForAEM(void)
{
  if(BSP_RegisterRead(&BC_REGISTER->UIF_AEM) != BC_UIF_AEM_EFM)
    aem_mode = 1; /* switched to Advanced Energy Monitor, LCD will need to be reinitialized */
  else if(aem_mode)
  {
    aem_mode = 0;
    LCD_InitializeDriver(); /* switched back from Advanced Energy Monitor, reinitialize LCD, touch panel */
    WM_InvalidateWindow(WM_HBKWIN);
    WM_ForEachDesc(WM_HBKWIN, _cbInvalidateWindow, (void *)0);
    GUI_Exec();
  }
}

static bool _checkIfBusy(void)
{ bool busy = false;
#if defined(EBI_PRESENT)
  if( (EBI->TFTCTRL & _EBI_TFTCTRL_DD_MASK) != EBI_TFTCTRL_DD_DISABLED)
    busy = true;
#endif
  if(busy)
    return busy;
  return (TOUCH_IsBusy()!=TOUCH_IDLE);
}

/***************************************************************************//**
*     @brief
*        is used to stop code execution for specified time
*     @param[in] ms
*        contains number of miliseconds to suspend program. Maximum allowed
*        value is 10000 (10 seconds).
*     @details
*        This routine could enter into EM1 or EM2 mode to reduce power
*        consumption. If touch panel is not pressed EM2 is executed, otherwise
*        due to fact that ADC requires HF clock, only EM1 is enabled. This
*        function is also used to handle joystick state and move cursor
*        according to it. In addition it could also reinitialize LCD if
*        previously Advanced Energy Monitor screen was active.
 ******************************************************************************/
void GUI_X_Delay(int ms)
{
  uint32_t rtc_counter;

  rtc_counter = RTC_CounterGet();

  _checkForAEM();
  _serveJoystick();

    /* Clear interrupt source */
  RTC_IntClear(RTC_IF_COMP0);

  rtcFlag = true;

  /* Calculate trigger value in ticks based on 32768Hz clock */
  RTC_CompareSet(0, (rtc_counter + ((RTC_FREQ * ms ) / 1000))& _RTC_COMP0_COMP0_MASK);

  /* Enable RTC */
  RTC_Enable(true);

  /* Enable interrupt on COMP0 */
  RTC_IntEnable(RTC_IF_COMP0);

  /* check if time already elapsed */
  if( (RTC_CompareGet(0)<= RTC_CounterGet()) ||
      (RTC_CompareGet(0) > (RTC_CounterGet()+RTC_FREQ*10)) )
    rtcFlag = false;

  /* The rtcFlag variable is set in the RTC interrupt routine using the callback
   * RTC_TimeOutHandler. This makes sure that the elapsed time is correct. */
  while (rtcFlag)
  {
    if(_checkIfBusy())
       EMU_EnterEM1();
    else
       EMU_EnterEM2(true);
  }
}


/***************************************************************************//**
 * @brief
 *    Enables LFACLK and selects LFRCO as clock source for RTC
 ******************************************************************************/
static void _InitRTC(void)
{
  RTC_Init_TypeDef init;

  /* Ensure LE modules are accessible */
  CMU_ClockEnable(cmuClock_CORELE, true);

  /* Enable LFRCO as LFACLK in CMU (will also enable oscillator if not enabled) */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);

  /* Enable clock to RTC module */
  CMU_ClockEnable(cmuClock_RTC, true);

  init.enable   = false;
  init.debugRun = false;
  init.comp0Top = false; /* Count to max before wrapping */
  RTC_Init(&init);

  /* Disable interrupt generation from RTC0 */
  RTC_IntDisable(_RTC_IF_MASK);

  /* Enable interrupts */
  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_EnableIRQ(RTC_IRQn);
}

/*********************************************************************
*
*       GUI_X_Init()
*
* Note:
*     @brief is called from GUI_Init is a possibility to init
*     some hardware which needs to be up and running before the GUI.
*     If not required, leave this routine blank.
*/

void GUI_X_Init(void)
{
  _InitRTC();
}


/*********************************************************************
*
*       GUI_X_ExecIdle
*
* Note:
*  @brief Called if WM is in idle state
*/

void GUI_X_ExecIdle(void) { GUI_X_Delay(100); }

/*********************************************************************
*
*      Logging: OS dependent

Note:
  Logging is used in higher debug levels only. The typical target
  build does not use logging and does therefor not require any of
  the logging routines below. For a release build without logging
  the routines below may be eliminated to save some space.
  (If the linker is not function aware and eliminates unreferenced
  functions automatically)

*/

void GUI_X_Log     (const char *s) { GUI_USE_PARA(s); }
void GUI_X_Warn    (const char *s) { GUI_USE_PARA(s); }
void GUI_X_ErrorOut(const char *s) { GUI_USE_PARA(s); }

/*************************** End of file ****************************/
