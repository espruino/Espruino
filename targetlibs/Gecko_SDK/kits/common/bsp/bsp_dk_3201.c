/***************************************************************************//**
 * @file
 * @brief Board support package API implementation for BRD3201.
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



#include "em_device.h"
#include "em_cmu.h"
#include "em_ebi.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "bsp_dk_bcreg_3201.h"
#include "bsp.h"

#if defined( BSP_DK_BRD3201 )
/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/* USART used for SPI access */
#define BSP_SPI_USART_USED  USART2          /**< USART used for BC register interface */
#define BSP_SPI_USART_CLK   cmuClock_USART2 /**< Clock for BC register USART */

/* GPIO pins used fotr SPI pins, please refer to DK user guide. */
#define BSP_PORT_SPI_TX     gpioPortC       /**< SPI transmit GPIO port */
#define BSP_PIN_SPI_TX      2               /**< SPI transmit GPIO pin */
#define BSP_PORT_SPI_RX     gpioPortC       /**< SPI receive GPIO port */
#define BSP_PIN_SPI_RX      3               /**< SPI receive GPIO pin */
#define BSP_PORT_SPI_CLK    gpioPortC       /**< SPI clock port */
#define BSP_PIN_SPI_CLK     4               /**< SPI clock pin */
#define BSP_PORT_SPI_CS     gpioPortC       /**< SPI Chip Select port */
#define BSP_PIN_SPI_CS      5               /**< SPI Chip Select pin */

/** SPI control */
typedef enum
{
  BSP_SPI_Audio,    /**< Configure switchable SPI interface to Audio I2S */
  BSP_SPI_Ethernet, /**< Configure switchable SPI interface to Ethernet */
  BSP_SPI_Display,  /**< Configure switchable SPI interface to SSD2119 */
  BSP_SPI_OFF,      /**< Disable SPI interface */
} BSP_SpiControl_TypeDef;

/** BSP board control access method */
typedef enum
{
  BSP_Init_EBI,    /**< Use EBI to interface board control functionality */
  BSP_Init_SPI,    /**< Use SPI to interface board control functionality */
  BSP_Init_DIRECT, /**< No board control, only GPIO */
  BSP_Init_OFF,    /**< Disabled */
} BSP_Init_TypeDef;

static bool EbiInit(void);
static void EbiDisable(void);
static uint16_t SpiBcAccess(uint8_t addr, uint8_t rw, uint16_t data);
static void SpiBcInit(void);
static void SpiControl(BSP_SpiControl_TypeDef device);
static void SpiBcDisable(void);
static bool SpiInit(void);
static uint16_t SpiRegisterRead(volatile uint16_t *addr);
static void SpiRegisterWrite(volatile uint16_t *addr, uint16_t data);

/* Keep intialization mode */
static uint32_t bspOperationMode;
static BSP_BusControl_TypeDef busMode = BSP_BusControl_Undefined;
static volatile const uint16_t *lastAddr = 0; /**< Last register accessed */
static uint16_t bcFwVersion;

/** @endcond */

/***************************************************************************//**
 * @addtogroup BSP
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup BSPCOMMON API common for all kits
 * @{
 ******************************************************************************/

/**************************************************************************//**
 * @brief Deinitialize board support package functionality.
 *        Reverse actions performed by BSP_Init().
 *        This function is a dummy on STK's.
 *
 * @return
 *   @ref BSP_STATUS_OK on DK's, @ref BSP_STATUS_NOT_IMPLEMENTED on STK's.
 *****************************************************************************/
int BSP_Disable(void)
{
  if (bspOperationMode == BSP_INIT_DK_EBI)
  {
    EbiDisable();
  }
  if (bspOperationMode == BSP_INIT_DK_SPI)
  {
    SpiBcDisable();
  }
  BSP_BusControlModeSet(BSP_BusControl_OFF);

  return BSP_STATUS_OK;
}

/**************************************************************************//**
 * @brief Initialize board support package functionality.
 *
 * @param[in] flags
 *   DK3x50:  Initialize in EBI or SPI mode using @ref BSP_INIT_DK_EBI or
 *            @ref BSP_INIT_DK_SPI.
 *   Gxxx_DK: Use 0.
 *   STK:     Use @ref BSP_INIT_BCC to initialize board controller UART, 0
 *            otherwise.
 *
 *   The value BSP_INIT_DEFAULT is defined and is: @ref BSP_INIT_DK_EBI on DK3x50,
 *             0 on all other kits.
 *
 * @return @ref BSP_STATUS_OK
 *****************************************************************************/
int BSP_Init(uint32_t flags)
{
  bool ret = false;

  if (flags & BSP_INIT_DK_EBI)
  {
    bspOperationMode = BSP_INIT_DK_EBI;
    BSP_BusControlModeSet(BSP_BusControl_EBI);
    ret = EbiInit();
  }
  if (flags & BSP_INIT_DK_SPI)
  {
    bspOperationMode = BSP_INIT_DK_SPI;
    BSP_BusControlModeSet(BSP_BusControl_SPI);
    ret = SpiInit();
  }

  if (ret == false)
  {
    /* Unable to access board control, this is an abornomal situation. */
    /* Try to restart kit and reprogram EFM32 with a standard example */
    /* as this is most likely caused by a peripheral misconfiguration. */
    while (1) ;
  }

  /* Inform AEM application that we are in Energy Mode 0 by default */
  BSP_RegisterWrite(&BC_REGISTER->EM, 0);

  /* Read out BC firmware version */
  bcFwVersion = BSP_RegisterRead(&BC_REGISTER->FW_VERSION);

  return BSP_STATUS_OK;
}
/** @} (end group BSPCOMMON) */

/***************************************************************************//**
 * @addtogroup BSP_DK API for DK's
 * @{
 ******************************************************************************/

/**************************************************************************//**
 * @brief Get current board controller bus decode logic configuration.
 *
 * @return Bus decode logic configuration as enumerated in
 *         @ref BSP_BusControl_TypeDef.
 *****************************************************************************/
BSP_BusControl_TypeDef BSP_BusControlModeGet( void )
{
  return busMode;
}

/**************************************************************************//**
 * @brief Configure Board Controller bus decode logic.
 *
 * @param[in] mode Mode of operation, use enum @ref BSP_BusControl_TypeDef mode.
 *                 On Gxxx_DK's this functions is a dummy.
 *
 * @return @ref BSP_STATUS_OK or @ref BSP_STATUS_ILLEGAL_PARAM.
 *         @ref BSP_STATUS_NOT_IMPLEMENTED on Gxxx_DK's.
 *****************************************************************************/
int BSP_BusControlModeSet(BSP_BusControl_TypeDef mode)
{
  int retVal = BSP_STATUS_OK;

  /* Configure GPIO pins for Board Bus mode */
  /* Note: Inverter on GPIO lines to BC, so signals are active low */
  CMU_ClockEnable(cmuClock_GPIO, true);

  busMode = mode;

  switch (mode)
  {
    case BSP_BusControl_OFF:
      /* Configure board for OFF mode on PB15 MCU_EBI_CONNECT */
      GPIO_PinModeSet(gpioPortB, 15, gpioModePushPull, 1);
      /* Configure board for OFF mode on PD13 MCU_SPI_CONNECT */
      GPIO_PinModeSet(gpioPortD, 13, gpioModePushPull, 1);
      break;

    case BSP_BusControl_DIRECT:
      /* Configure board for DIRECT on PB15 MCU_EBI_CONNECT */
      GPIO_PinModeSet(gpioPortB, 15, gpioModePushPull, 0);
      /* Configure board for DIRECT on PD13 MCU_SPI_CONNECT */
      GPIO_PinModeSet(gpioPortD, 13, gpioModePushPull, 0);
      break;

    case BSP_BusControl_SPI:
      /* Configure board for SPI mode on PB15 MCU_EBI_CONNECT */
      GPIO_PinModeSet(gpioPortB, 15, gpioModePushPull, 1);
      /* Configure board for SPI mode on PD13 MCU_SPI_CONNECT */
      GPIO_PinModeSet(gpioPortD, 13, gpioModePushPull, 0);
      break;

    case BSP_BusControl_EBI:
      /* Configure board for EBI mode on PB15 MCU_EBI_CONNECT */
      GPIO_PinModeSet(gpioPortB, 15, gpioModePushPull, 0);
      /* Configure board for EBI mode on PD13 MCU_SPI_CONNECT */
      GPIO_PinModeSet(gpioPortD, 13, gpioModePushPull, 1);
      break;

    default:
      retVal = BSP_STATUS_ILLEGAL_PARAM;
      break;
  }
  return retVal;
}

/**************************************************************************//**
 * @brief Get status of the DIP switches on the DK.
 * @return Bitmask with a single bit for each DIP switch.
 *****************************************************************************/
uint32_t BSP_DipSwitchGet(void)
{
  return BSP_RegisterRead(&BC_REGISTER->UIF_DIP) & 0x000f;
}

/**************************************************************************//**
 * @brief Configure display control.
 *
 * @param[in] option Configure using enum @ref BSP_Display_TypeDef.
 *                 On Gxxx_DK's this functions is a dummy.
 *
 * @return @ref BSP_STATUS_OK or @ref BSP_STATUS_NOT_IMPLEMENTED on Gxxx_DK's.
 *****************************************************************************/
int BSP_DisplayControl(BSP_Display_TypeDef option)
{
  uint16_t tmp;

  switch (option)
  {
  case BSP_Display_EBI:
    BSP_RegisterWrite(&BC_REGISTER->ARB_CTRL, BC_ARB_CTRL_EBI);
    break;

  case BSP_Display_SPI:
    BSP_RegisterWrite(&BC_REGISTER->ARB_CTRL, BC_ARB_CTRL_SPI);
    break;

  case BSP_Display_BC:
    BSP_RegisterWrite(&BC_REGISTER->ARB_CTRL, BC_ARB_CTRL_BC);
    break;

  case BSP_Display_PowerEnable:
    tmp  = BSP_RegisterRead(&BC_REGISTER->DISPLAY_CTRL);
    tmp |= (BC_DISPLAY_CTRL_POWER_ENABLE);
    BSP_RegisterWrite(&BC_REGISTER->DISPLAY_CTRL, tmp);
    break;

  case BSP_Display_PowerDisable:
    tmp  = BSP_RegisterRead(&BC_REGISTER->DISPLAY_CTRL);
    tmp &= ~(BC_DISPLAY_CTRL_POWER_ENABLE);
    BSP_RegisterWrite(&BC_REGISTER->DISPLAY_CTRL, tmp);
    break;

  case BSP_Display_ResetAssert:
    tmp  = BSP_RegisterRead(&BC_REGISTER->DISPLAY_CTRL);
    tmp |= (BC_DISPLAY_CTRL_RESET);
    BSP_RegisterWrite(&BC_REGISTER->DISPLAY_CTRL, tmp);
    break;

  case BSP_Display_ResetRelease:
    tmp  = BSP_RegisterRead(&BC_REGISTER->DISPLAY_CTRL);
    tmp &= ~(BC_DISPLAY_CTRL_RESET);
    BSP_RegisterWrite(&BC_REGISTER->DISPLAY_CTRL, tmp);
    break;

  case BSP_Display_Mode8080:
    tmp  = BSP_RegisterRead(&BC_REGISTER->DISPLAY_CTRL);
    tmp &= ~(BC_DISPLAY_CTRL_MODE_GENERIC);
    BSP_RegisterWrite(&BC_REGISTER->DISPLAY_CTRL, tmp);
    break;

  case BSP_Display_ModeGeneric:
    tmp  = BSP_RegisterRead(&BC_REGISTER->DISPLAY_CTRL);
    tmp |= (BC_DISPLAY_CTRL_MODE_GENERIC);
    BSP_RegisterWrite(&BC_REGISTER->DISPLAY_CTRL, tmp);
    break;

  default:
    /* Unknown command */
    while (1);
  }

  return BSP_STATUS_OK;
}

/**************************************************************************//**
 * @brief Extended the EBI addressing range by enabling address lines A16-A22.
 *    On Gxxx_DK's this functions is a dummy.
 *
 * @param[in] enable
 *    If true, enable address lines A16-A22, if false disable.
 *
 * @return
 *   @ref BSP_STATUS_OK on DK3x50, @ref BSP_STATUS_NOT_IMPLEMENTED on Gxxx_DK's.
 *****************************************************************************/
int BSP_EbiExtendedAddressRange(bool enable)
{
  if (enable)
  {
    BSP_RegisterWrite(&BC_REGISTER->EBI_CTRL, BC_EBI_CTRL_EXTADDR_MASK);
  }
  else
  {
    BSP_RegisterWrite(&BC_REGISTER->EBI_CTRL, 0);
  }
  return BSP_STATUS_OK;
}


/**************************************************************************//**
 * @brief Inform board controller about current energy mode.
 *
 * @param[in] energyMode A number describing the energy mode.
 *
 * @return @ref BSP_STATUS_OK
 *****************************************************************************/
int BSP_EnergyModeSet(uint16_t energyMode)
{
  BSP_RegisterWrite(&BC_REGISTER->EM, energyMode);
  return BSP_STATUS_OK;
}

/**************************************************************************//**
 * @brief Disable interrupts from board controller.
 *
 * @param[in] flags A bitmask defining which interrupt sources to disable.
 *
 * @return @ref BSP_STATUS_OK
 *****************************************************************************/
int BSP_InterruptDisable(uint16_t flags)
{
  uint16_t tmp;

  /* Clear flags from interrupt enable register */
  tmp   = BSP_RegisterRead(&BC_REGISTER->INTEN);
  flags = ~(flags);
  tmp  &= flags;
  BSP_RegisterWrite(&BC_REGISTER->INTEN, tmp);
  return BSP_STATUS_OK;
}

/**************************************************************************//**
 * @brief Enable interrupts from board controller.
 *
 * @param[in] flags A bitmask defining which interrupt sources to enable.
 *
 * @return @ref BSP_STATUS_OK
 *****************************************************************************/
int BSP_InterruptEnable(uint16_t flags)
{
  uint16_t tmp;

  /* Add flags to interrupt enable register */
  tmp  = BSP_RegisterRead(&BC_REGISTER->INTEN);
  tmp |= flags;
  BSP_RegisterWrite(&BC_REGISTER->INTEN, tmp);
  return BSP_STATUS_OK;
}

/**************************************************************************//**
 * @brief Clear board controller interrupt flags.
 *
 * @param[in] flags A bitmask defining which interrupt sources to clear.
 *
 * @return @ref BSP_STATUS_OK
 *****************************************************************************/
int BSP_InterruptFlagsClear(uint16_t flags)
{
  uint16_t intFlags;

  /* Board control firmware version 257 and higher has a new interrupt architecture */
  if (bcFwVersion < 257)
  {
    intFlags  = BSP_RegisterRead(&BC_REGISTER->INTFLAG);
    intFlags &= ~(flags);
    BSP_RegisterWrite(&BC_REGISTER->INTFLAG, intFlags);
  }
  else
  {
    BSP_RegisterWrite(&BC_REGISTER->INTCLEAR, flags);
  }
  return BSP_STATUS_OK;
}

/**************************************************************************//**
 * @brief Set board controller interrupt flags.
 *
 * @param[in] flags A bitmask defining which interrupt sources to set.
 *
 * @return @ref BSP_STATUS_OK
 *****************************************************************************/
int BSP_InterruptFlagsSet(uint16_t flags)
{
  BSP_RegisterWrite(&BC_REGISTER->INTSET, flags);
  return BSP_STATUS_OK;
}

/**************************************************************************//**
 * @brief Get board controller interrupt flags.
 *
 * @return A bitmask defining which interrupt sources have their flaf set.
 *****************************************************************************/
uint16_t BSP_InterruptFlagsGet(void)
{
  return BSP_RegisterRead(&BC_REGISTER->INTFLAG);
}

/**************************************************************************//**
 * @brief Get status of joystick on the DK.
 * @return The status of the 5 joystick switches. See @ref BC_JOYSTICK_MASK
 *         or @ref BC_UIF_JOYSTICK_MASK according to your kit.
 *****************************************************************************/
uint16_t BSP_JoystickGet(void)
{
  return ~(BSP_RegisterRead(&BC_REGISTER->UIF_JOYSTICK)) & 0x001f;
}

/**************************************************************************//**
 * @brief DK Peripheral Access Control
 *    Enable or disable access to on-board peripherals through switches
 *    and SPI switch where applicable. Turn off conflicting peripherals when
 *    enabling another.
 *
 * @param[in] perf
 *    Which peripheral to configure. Use enum @ref BSP_Peripheral_TypeDef or
 *    @ref BSP_Peripheral_Typedef according to DK type.
 *
 * @param[in] enable
 *    If true, set up access to peripheral, if false disable access.
 *
 * @return
 *   @ref BSP_STATUS_OK.
 *****************************************************************************/
int BSP_PeripheralAccess(BSP_Peripheral_TypeDef perf, bool enable)
{
  uint16_t perfControl;

  perfControl = BSP_RegisterRead(&BC_REGISTER->PERICON);

  /* Enable or disable the specified peripheral by setting board control switch */
  if (enable)
  {
    switch (perf)
    {
    case BSP_RS232_SHUTDOWN:
      perfControl |= (1 << BC_PERICON_RS232_SHUTDOWN_SHIFT);
      break;

    case BSP_RS232_UART:
      perfControl &= ~(1 << BC_PERICON_RS232_SHUTDOWN_SHIFT);
      perfControl &= ~(1 << BC_PERICON_RS232_LEUART_SHIFT);
      perfControl |= (1 << BC_PERICON_RS232_UART_SHIFT);
      break;

    case BSP_RS232_LEUART:
      perfControl &= ~(1 << BC_PERICON_RS232_SHUTDOWN_SHIFT);
      perfControl &= ~(1 << BC_PERICON_RS232_UART_SHIFT);
      perfControl |= (1 << BC_PERICON_RS232_LEUART_SHIFT);
      break;

    case BSP_I2C:
      perfControl |= (1 << BC_PERICON_I2C_SHIFT);
      break;

    case BSP_ETH:
      /* Enable SPI interface */
      SpiControl(BSP_SPI_Ethernet);

      /* Enable Ethernet analog switches */
      perfControl |= (1 << BC_PERICON_I2S_ETH_SHIFT);
      perfControl |= (1 << BC_PERICON_I2S_ETH_SEL_SHIFT);

      /* Disable Analog Diff Input - pins PD0 and PD1 is shared */
      perfControl &= ~(1 << BC_PERICON_ANALOG_DIFF_SHIFT);
      /* Disable Touch Inputs - pin PD3 is shared */
      perfControl &= ~(1 << BC_PERICON_TOUCH_SHIFT);
      /* Disable Analog SE Input - pin PD2 is shared */
      perfControl &= ~(1 << BC_PERICON_ANALOG_SE_SHIFT);
      break;

    case BSP_I2S:
      /* Direct SPI interface to I2S DAC */
      SpiControl(BSP_SPI_Audio);

      /* Also make surea Audio out is connected for I2S operation */
      perfControl |= (1 << BC_PERICON_AUDIO_OUT_SHIFT);
      perfControl |= (1 << BC_PERICON_AUDIO_OUT_SEL_SHIFT);
      perfControl |= (1 << BC_PERICON_I2S_ETH_SHIFT);
      perfControl &= ~(1 << BC_PERICON_I2S_ETH_SEL_SHIFT);

      /* Disable Analog Diff Input - pins PD0 and PD1 is shared */
      perfControl &= ~(1 << BC_PERICON_ANALOG_DIFF_SHIFT);
      /* Disable Touch Inputs - pin PD3 is shared */
      perfControl &= ~(1 << BC_PERICON_TOUCH_SHIFT);
      /* Disable Analog SE Input - pin PD2 is shared */
      perfControl &= ~(1 << BC_PERICON_ANALOG_SE_SHIFT);
      break;

    case BSP_TRACE:
      #if defined(ETM_PRESENT)
        perfControl |= (1 << BC_PERICON_TRACE_SHIFT);
        break;
      #else
        /* TRACE is not available on EFM32G890F128, application error */
        while (1) ;
      #endif

    case BSP_TOUCH:
      perfControl |= (1 << BC_PERICON_TOUCH_SHIFT);
      /* Disconnect SPI switch, pin PD3 is shared */
      perfControl &= ~(1 << BC_PERICON_I2S_ETH_SHIFT);
      perfControl &= ~(1 << BC_PERICON_I2S_ETH_SEL_SHIFT);
      SpiControl(BSP_SPI_OFF);
      break;

    case BSP_AUDIO_IN:
      perfControl |= (1 << BC_PERICON_AUDIO_IN_SHIFT);
      break;

    case BSP_AUDIO_OUT:
      perfControl &= ~(1 << BC_PERICON_AUDIO_OUT_SEL_SHIFT);
      perfControl |= (1 << BC_PERICON_AUDIO_OUT_SHIFT);
      break;

    case BSP_ANALOG_DIFF:
      perfControl |= (1 << BC_PERICON_ANALOG_DIFF_SHIFT);
      /* Disconnect SPI switch, pin PD0 and PD1 is shared */
      perfControl &= ~(1 << BC_PERICON_I2S_ETH_SHIFT);
      perfControl &= ~(1 << BC_PERICON_I2S_ETH_SEL_SHIFT);
      SpiControl(BSP_SPI_OFF);
      break;

    case BSP_ANALOG_SE:
      perfControl |= (1 << BC_PERICON_ANALOG_SE_SHIFT);
      /* Disconnect SPI switch, pin PD2 is shared */
      perfControl &= ~(1 << BC_PERICON_I2S_ETH_SHIFT);
      perfControl &= ~(1 << BC_PERICON_I2S_ETH_SEL_SHIFT);
      SpiControl(BSP_SPI_OFF);
      break;

    case BSP_MICROSD:
      perfControl |= (1 << BC_PERICON_SPI_SHIFT);
      break;

    case BSP_TFT:
      /* Enable SPI to SSD2119 */
      SpiControl(BSP_SPI_Display);
      /* Enable SPI analog switch */
      perfControl |= (1 << BC_PERICON_I2S_ETH_SHIFT);
      /* Disable Analog Diff Input - pins D0 and D1 is shared */
      perfControl &= ~(1 << BC_PERICON_ANALOG_DIFF_SHIFT);
      /* Disable Touch Inputs - pin D3 is shared */
      perfControl &= ~(1 << BC_PERICON_TOUCH_SHIFT);
      /* Disable Analog SE Input - pin D2 is shared */
      perfControl &= ~(1 << BC_PERICON_ANALOG_SE_SHIFT);
      break;
    }
  }
  else
  {
    switch (perf)
    {
    case BSP_RS232_SHUTDOWN:
      perfControl &= ~(1 << BC_PERICON_RS232_SHUTDOWN_SHIFT);
      break;

    case BSP_RS232_UART:
      perfControl |= (1 << BC_PERICON_RS232_SHUTDOWN_SHIFT);
      perfControl &= ~(1 << BC_PERICON_RS232_UART_SHIFT);
      break;

    case BSP_RS232_LEUART:
      perfControl |= (1 << BC_PERICON_RS232_SHUTDOWN_SHIFT);
      perfControl &= ~(1 << BC_PERICON_RS232_LEUART_SHIFT);
      break;

    case BSP_I2C:
      perfControl &= ~(1 << BC_PERICON_I2C_SHIFT);
      break;

    case BSP_ETH:
      /* Disable SPI interface */
      perfControl &= ~(1 << BC_PERICON_I2S_ETH_SHIFT);
      perfControl &= ~(1 << BC_PERICON_I2S_ETH_SEL_SHIFT);
      SpiControl(BSP_SPI_OFF);
      break;

    case BSP_I2S:
      /* Disable SPI interface and audio out */
      perfControl &= ~(1 << BC_PERICON_AUDIO_OUT_SHIFT);
      perfControl &= ~(1 << BC_PERICON_AUDIO_OUT_SEL_SHIFT);
      perfControl &= ~(1 << BC_PERICON_I2S_ETH_SHIFT);
      perfControl &= ~(1 << BC_PERICON_I2S_ETH_SEL_SHIFT);
      SpiControl(BSP_SPI_OFF);
      break;

    case BSP_TRACE:
      #if defined(ETM_PRESENT)
        perfControl &= ~(1 << BC_PERICON_TRACE_SHIFT);
        break;
      #else
        /* TRACE is not available on EFM32G890F128, application error */
        while (1) ;
      #endif

    case BSP_TOUCH:
      perfControl &= ~(1 << BC_PERICON_TOUCH_SHIFT);
      break;

    case BSP_AUDIO_IN:
      perfControl &= ~(1 << BC_PERICON_AUDIO_IN_SHIFT);
      break;

    case BSP_AUDIO_OUT:
      perfControl &= ~(1 << BC_PERICON_AUDIO_OUT_SEL_SHIFT);
      perfControl &= ~(1 << BC_PERICON_AUDIO_OUT_SHIFT);
      break;

    case BSP_ANALOG_DIFF:
      perfControl &= ~(1 << BC_PERICON_ANALOG_DIFF_SHIFT);
      break;

    case BSP_ANALOG_SE:
      perfControl &= ~(1 << BC_PERICON_ANALOG_SE_SHIFT);
      break;

    case BSP_MICROSD:
      perfControl &= ~(1 << BC_PERICON_SPI_SHIFT);
      break;

    case BSP_TFT:
      /* Disable SPI interface */
      perfControl &= ~(1 << BC_PERICON_I2S_ETH_SHIFT);
      perfControl &= ~(1 << BC_PERICON_I2S_ETH_SEL_SHIFT);
      SpiControl(BSP_SPI_OFF);
      break;
    }
  }
  /* Write back register */
  BSP_RegisterWrite(&BC_REGISTER->PERICON, perfControl);

  return BSP_STATUS_OK;
}

/**************************************************************************//**
 * @brief Get status of the pushbutton switches on the DK.
 * @return The status of the switches. See @ref BC_PUSHBUTTON_MASK
 *         or @ref BC_UIF_PB_MASK according to your kit.
 *****************************************************************************/
uint16_t BSP_PushButtonsGet(void)
{
  return (~BSP_RegisterRead(&BC_REGISTER->UIF_PB)) & 0x000F;
}

/**************************************************************************//**
 * @brief Read from a board controller register.
 * @param[in] addr The board controller register address.
 * @return The value read.
 *****************************************************************************/
uint16_t BSP_RegisterRead(volatile uint16_t *addr)
{
  if (bspOperationMode == BSP_INIT_DK_EBI)
  {
    return *addr;
  }
  else
  {
    return SpiRegisterRead(addr);
  }
}

/**************************************************************************//**
 * @brief Write to a board controller register.
 * @param[in] addr The Board controller register address.
 * @param[in] data The value to write.
 *****************************************************************************/
int BSP_RegisterWrite(volatile uint16_t *addr, uint16_t data)
{
  if (bspOperationMode == BSP_INIT_DK_EBI)
  {
    *addr = data;
  }
  else
  {
    SpiRegisterWrite(addr, data);
  }
  return BSP_STATUS_OK;
}
/** @} (end group BSP_DK) */
/** @} (end group BSP) */

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

static void EbiDisable(void)
{
#if defined(_EFM32_GECKO_FAMILY)

  /* Configure GPIO pins as disabled */
  GPIO_PinModeSet( gpioPortA,  0, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortA,  1, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortA,  2, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortA,  3, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortA,  4, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortA,  5, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortA,  6, gpioModeDisabled, 0 );

  GPIO_PinModeSet( gpioPortA, 15, gpioModeDisabled, 0 );

  GPIO_PinModeSet( gpioPortD,  9, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortD, 10, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortD, 11, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortD, 12, gpioModeDisabled, 0 );

  GPIO_PinModeSet( gpioPortE,  8, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE,  9, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE, 10, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE, 11, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE, 12, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE, 13, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE, 14, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE, 15, gpioModeDisabled, 0 );

  GPIO_PinModeSet( gpioPortF, 2, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortF, 3, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortF, 4, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortF, 5, gpioModeDisabled, 0 );

  /* EBI Byte Lane 0 support BL0/BL1 */
  GPIO_PinModeSet( gpioPortF, 6, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortF, 7, gpioModeDisabled, 0 );

#else

  /* Configure GPIO pins as disabled */
  /* EBI AD9..15 */
  GPIO_PinModeSet( gpioPortA,  0, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortA,  1, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortA,  2, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortA,  3, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortA,  4, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortA,  5, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortA,  6, gpioModeDisabled, 0 );

  /* EBI AD8 */
  GPIO_PinModeSet( gpioPortA, 15, gpioModeDisabled, 0 );

  /* EBI A16-A22 */
  GPIO_PinModeSet( gpioPortB,  0, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortB,  1, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortB,  2, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortB,  3, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortB,  4, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortB,  5, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortB,  6, gpioModeDisabled, 0 );

  /* EBI CS0-CS3 */
  GPIO_PinModeSet( gpioPortD,  9, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortD, 10, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortD, 11, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortD, 12, gpioModeDisabled, 0 );

  /* EBI AD0..7 */
  GPIO_PinModeSet( gpioPortE,  8, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE,  9, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE, 10, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE, 11, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE, 12, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE, 13, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE, 14, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortE, 15, gpioModeDisabled, 0 );

  /* EBI ARDY/WEN/REN/ALE */
  GPIO_PinModeSet( gpioPortF,  2, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortF,  8, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortF,  9, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortC, 11, gpioModeDisabled, 0 );

  /* EBI Byte Lane 0 support BL0/BL1 */
  GPIO_PinModeSet( gpioPortF,  6, gpioModeDisabled, 0 );
  GPIO_PinModeSet( gpioPortF,  7, gpioModeDisabled, 0 );

#endif

  /* Reset EBI configuration */
  EBI_Disable();

  /* Turn off EBI clock */
  CMU_ClockEnable(cmuClock_EBI, false);
}

/**************************************************************************//**
 * @brief Configure DK3x50 EBI (external bus interface) access for
 *    - 0x80000000: Board Control registers (Xilinx Spartan FPGA)
 *    - 0x84000000: TFT memory mapped drive (URT/SSD2119 controller)
 *    - 0x88000000: PSRAM external memory (Micron MT45W2MW16PGA-70 IT)
 *    - 0x8c000000: NOR flash (Spansion flash S29GLxxx_FBGA)
 * @return true if successful, false if board controller access failed
 *****************************************************************************/
static bool EbiInit(void)
{
  EBI_Init_TypeDef ebiConfig = EBI_INIT_DEFAULT;

  /* Enable clocks */
  CMU_ClockEnable(cmuClock_EBI, true);
  CMU_ClockEnable(cmuClock_GPIO, true);

#if defined(_EFM32_GECKO_FAMILY)

  /* Configure LCD_SELECT (EBI and LCD cannot be shared) */
  GPIO_PinModeSet( gpioPortC, 12, gpioModePushPull, 1 );

  /* Configure GPIO pins as push pull */
  /* EBI AD9..15 */
  GPIO_PinModeSet( gpioPortA,  0, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortA,  1, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortA,  2, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortA,  3, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortA,  4, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortA,  5, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortA,  6, gpioModePushPull, 0 );

  /* EBI AD8 */
  GPIO_PinModeSet( gpioPortA, 15, gpioModePushPull, 0 );

  /* EBI CS0-CS3 */
  GPIO_PinModeSet( gpioPortD,  9, gpioModePushPull, 1 );
  GPIO_PinModeSet( gpioPortD, 10, gpioModePushPull, 1 );
  GPIO_PinModeSet( gpioPortD, 11, gpioModePushPull, 1 );
  GPIO_PinModeSet( gpioPortD, 12, gpioModePushPull, 1 );

  /* EBI AD0..7 */
  GPIO_PinModeSet( gpioPortE,  8, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE,  9, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE, 10, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE, 11, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE, 12, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE, 13, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE, 14, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE, 15, gpioModePushPull, 0 );

  /* EBI ARDY/ALEN/Wen/Ren */
  GPIO_PinModeSet( gpioPortF,  2, gpioModeInput,    0 );
  GPIO_PinModeSet( gpioPortF,  3, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortF,  4, gpioModePushPull, 1 );
  GPIO_PinModeSet( gpioPortF,  5, gpioModePushPull, 1 );

  /* Byte Lanes */
  GPIO_PinModeSet( gpioPortF,  6, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortF,  7, gpioModePushPull, 0 );

  /* Configure EBI controller, changing default values */
  ebiConfig.mode = ebiModeD16A16ALE;

  /* --------------------------------------------------------- */
  /* Board Control Registers, Bank 0, Base Address 0x80000000  */
  /* FPGA Xilinx Spartan XC6SLX9 CSG324                        */
  /* --------------------------------------------------------- */

  /* ----------------------------------------------------- */
  /* TFT-LCD Registers, Bank1, Base Address 0x84000000     */
  /* URT USMH_8252MD_320X240_RGB                           */
  /* Solomon Systech SSD 2119                              */
  /* ----------------------------------------------------- */

  /* ---------------------------------------------------- */
  /* External 4MB PSRAM, Bank 2, Base Address 0x88000000  */
  /* Micron MT45W2MW16PGA-70 IT, 32Mb Cellular RAM        */
  /* ---------------------------------------------------- */

  /* ----------------------------------------- */
  /* NOR Flash, Bank3, Base Address 0x8c000000 */
  /* Spansion flash S29GLxxx_FBGA              */
  /* ----------------------------------------- */

  ebiConfig.banks   = EBI_BANK0 | EBI_BANK1 | EBI_BANK2 | EBI_BANK3;
  ebiConfig.csLines = EBI_CS0 | EBI_CS1 | EBI_CS2 | EBI_CS3;

  /* Address Setup and hold time */
  ebiConfig.addrHoldCycles  = 3;
  ebiConfig.addrSetupCycles = 3;

  /* Read cycle times */
  ebiConfig.readStrobeCycles = 7;
  ebiConfig.readHoldCycles   = 3;
  ebiConfig.readSetupCycles  = 3;

  /* Write cycle times */
  ebiConfig.writeStrobeCycles = 7;
  ebiConfig.writeHoldCycles   = 3;
  ebiConfig.writeSetupCycles  = 3;

  /* Address Latch Enable polarity is active high */
  ebiConfig.alePolarity = ebiActiveHigh;

  /* Configure EBI */
  EBI_Init(&ebiConfig);

#else

  /* Giant or Leopard family. */

  /* Configure GPIO pins as push pull */
  /* EBI AD9..15 */
  GPIO_PinModeSet( gpioPortA,  0, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortA,  1, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortA,  2, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortA,  3, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortA,  4, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortA,  5, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortA,  6, gpioModePushPull, 0 );

  /* EBI AD8 */
  GPIO_PinModeSet( gpioPortA, 15, gpioModePushPull, 0 );

  /* EBI A16-A22 */
  GPIO_PinModeSet( gpioPortB,  0, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortB,  1, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortB,  2, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortB,  3, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortB,  4, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortB,  5, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortB,  6, gpioModePushPull, 0 );

  /* EBI CS0-CS3 */
  GPIO_PinModeSet( gpioPortD,  9, gpioModePushPull, 1 );
  GPIO_PinModeSet( gpioPortD, 10, gpioModePushPull, 1 );
  GPIO_PinModeSet( gpioPortD, 11, gpioModePushPull, 1 );
  GPIO_PinModeSet( gpioPortD, 12, gpioModePushPull, 1 );

  /* EBI AD0..7 */
  GPIO_PinModeSet( gpioPortE,  8, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE,  9, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE, 10, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE, 11, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE, 12, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE, 13, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE, 14, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortE, 15, gpioModePushPull, 0 );

  /* EBI ARDY/WEN/REN/ALE */
  GPIO_PinModeSet( gpioPortF,  2, gpioModeInput,    0 );
  GPIO_PinModeSet( gpioPortF,  8, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortF,  9, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortC, 11, gpioModePushPull, 0 );

  /* EBI Byte Lane 0 support BL0/BL1 */
  GPIO_PinModeSet( gpioPortF, 6, gpioModePushPull, 0 );
  GPIO_PinModeSet( gpioPortF, 7, gpioModePushPull, 0 );

  /* ---------------------------------------------------- */
  /* External 4MB PSRAM, Bank 2, Base Address 0x88000000  */
  /* Micron MT45W2MW16PGA-70 IT, 32Mb Cellular RAM        */
  /* ---------------------------------------------------- */
  ebiConfig.banks        = EBI_BANK2;
  ebiConfig.csLines      = EBI_CS2;
  ebiConfig.mode         = ebiModeD16A16ALE;
  ebiConfig.alePolarity  = ebiActiveHigh;
  ebiConfig.blEnable     = true;
  ebiConfig.noIdle       = true;
  ebiConfig.ardyEnable   = false;
  ebiConfig.addrHalfALE  = true;
  ebiConfig.readPrefetch = true;
  ebiConfig.aLow         = ebiALowA16;
  ebiConfig.aHigh        = ebiAHighA23;
  ebiConfig.location     = ebiLocation1;

  /* Address Setup and hold time */
  ebiConfig.addrHoldCycles  = 0;
  ebiConfig.addrSetupCycles = 0;

  /* Read cycle times */
  ebiConfig.readStrobeCycles = 4;
  ebiConfig.readHoldCycles   = 0;
  ebiConfig.readSetupCycles  = 0;

  /* Write cycle times */
  ebiConfig.writeStrobeCycles = 2;
  ebiConfig.writeHoldCycles   = 0;
  ebiConfig.writeSetupCycles  = 0;

  /* Configure EBI bank 2 - external PSRAM */
  EBI_Init(&ebiConfig);

  /* --------------------------------------------------------- */
  /* Board Control Registers, Bank 0, Base Address 0x80000000  */
  /* FPGA Xilinx Spartan XC6SLX9 CSG324                        */
  /* --------------------------------------------------------- */
  ebiConfig.banks       = EBI_BANK0;
  ebiConfig.csLines     = EBI_CS0;
  ebiConfig.mode        = ebiModeD16A16ALE;;
  ebiConfig.alePolarity = ebiActiveHigh;
  /* keep blEnable */
  ebiConfig.blEnable     = false;
  ebiConfig.addrHalfALE  = true;
  ebiConfig.readPrefetch = false;
  ebiConfig.noIdle       = true;

  /* keep alow/ahigh configuration */
  /* ebiConfig.aLow = ebiALowA0; - needs to be set for PSRAM */
  /* ebiConfig.aHigh = ebiAHighA0; - needs to be set for PSRAM */

  /* Address Setup and hold time */
  ebiConfig.addrHoldCycles  = 3;
  ebiConfig.addrSetupCycles = 3;

  /* Read cycle times */
  ebiConfig.readStrobeCycles = 7;
  ebiConfig.readHoldCycles   = 3;
  ebiConfig.readSetupCycles  = 3;

  /* Write cycle times */
  ebiConfig.writeStrobeCycles = 7;
  ebiConfig.writeHoldCycles   = 3;
  ebiConfig.writeSetupCycles  = 3;

  /* Configure EBI bank 0 */
  EBI_Init(&ebiConfig);

  /* ----------------------------------------------------- */
  /* TFT-LCD Registers, Bank1, Base Address 0x84000000     */
  /* URT USMH_8252MD_320X240_RGB                           */
  /* Solomon Systech SSD 2119                              */
  /* ----------------------------------------------------- */
  ebiConfig.banks   = EBI_BANK1;
  ebiConfig.csLines = EBI_CS1;

  /* Address Setup and hold time */
  ebiConfig.addrHoldCycles  = 1;
  ebiConfig.addrSetupCycles = 1;

  /* Read cycle times */
  ebiConfig.readStrobeCycles = 7;
  ebiConfig.readHoldCycles   = 3;
  ebiConfig.readSetupCycles  = 3;

  /* Write cycle times */
  ebiConfig.writeStrobeCycles = 2;
  ebiConfig.writeHoldCycles   = 1;
  ebiConfig.writeSetupCycles  = 1;

  /* Configure EBI bank 1 */
  EBI_Init(&ebiConfig);

  /* ----------------------------------------- */
  /* NOR Flash, Bank3, Base Address 0x8c000000 */
  /* Spansion flash S29GLxxx_FBGA              */
  /* ----------------------------------------- */
  ebiConfig.banks       = EBI_BANK3;
  ebiConfig.csLines     = EBI_CS3;
  ebiConfig.mode        = ebiModeD16A16ALE;
  ebiConfig.alePolarity = ebiActiveHigh;

  /* keep blEnable */
  ebiConfig.blEnable     = true;
  ebiConfig.addrHalfALE  = true;
  ebiConfig.readPrefetch = false;
  ebiConfig.noIdle       = true;

  /* Address Setup and hold time */
  ebiConfig.addrHoldCycles  = 0;
  ebiConfig.addrSetupCycles = 0;

  /* Read cycle times */
  ebiConfig.readStrobeCycles = 6;
  ebiConfig.readHoldCycles   = 0;
  ebiConfig.readSetupCycles  = 0;

  /* Write cycle times */
  ebiConfig.writeStrobeCycles = 5;
  ebiConfig.writeHoldCycles   = 0;
  ebiConfig.writeSetupCycles  = 0;

  /* Configure EBI bank 3 */
  EBI_Init(&ebiConfig);

  /* Enable extended address range */
  BSP_EbiExtendedAddressRange(true);
#endif

  /* Verify connectivity to Board Control registers */
  if (BC_REGISTER->MAGIC != 0xef32)
  {
    return false;
  }
  else
  {
    return true;
  }
}

static uint16_t SpiBcAccess(uint8_t addr, uint8_t rw, uint16_t data)
{
  uint16_t tmp;

  /* Enable CS */
  GPIO_PinOutClear(BSP_PORT_SPI_CS, BSP_PIN_SPI_CS);

  /* Write SPI address MSB */
  USART_Tx(BSP_SPI_USART_USED, (addr & 0x3) | rw << 3);
  /* Just ignore data read back */
  USART_Rx(BSP_SPI_USART_USED);

  /* Write SPI address  LSB */
  USART_Tx(BSP_SPI_USART_USED, data & 0xFF);

  tmp = (uint16_t) USART_Rx(BSP_SPI_USART_USED);

  /* SPI data MSB */
  USART_Tx(BSP_SPI_USART_USED, data >> 8);
  tmp |= (uint16_t) USART_Rx(BSP_SPI_USART_USED) << 8;

  /* Disable CS */
  GPIO_PinOutSet(BSP_PORT_SPI_CS, BSP_PIN_SPI_CS);

  return tmp;
}

static void SpiBcDisable(void)
{
  /* Restore and disable USART */
  USART_Reset(BSP_SPI_USART_USED);

  GPIO_PinModeSet(BSP_PORT_SPI_TX, BSP_PIN_SPI_TX, gpioModeDisabled, 0);
  GPIO_PinModeSet(BSP_PORT_SPI_RX, BSP_PIN_SPI_RX, gpioModeDisabled, 0);
  GPIO_PinModeSet(BSP_PORT_SPI_CLK, BSP_PIN_SPI_CLK, gpioModeDisabled, 0);
  GPIO_PinModeSet(BSP_PORT_SPI_CS, BSP_PIN_SPI_CS, gpioModeDisabled, 0);

  /* Disable USART clock - we can't disable GPIO or HFPER as we don't know who else
   * might be using it */
  CMU_ClockEnable(BSP_SPI_USART_CLK, false);
}

static void SpiBcInit(void)
{
  USART_InitSync_TypeDef bcinit = USART_INITSYNC_DEFAULT;

  /* Enable module clocks */
  CMU_ClockEnable(BSP_SPI_USART_CLK, true);

  /* Configure SPI pins */
  GPIO_PinModeSet(BSP_PORT_SPI_TX, BSP_PIN_SPI_TX, gpioModePushPull, 0);
  GPIO_PinModeSet(BSP_PORT_SPI_RX, BSP_PIN_SPI_RX, gpioModeInput, 0);
  GPIO_PinModeSet(BSP_PORT_SPI_CLK, BSP_PIN_SPI_CLK, gpioModePushPull, 0);

  /* Keep CS high to not activate slave */
  GPIO_PinModeSet(BSP_PORT_SPI_CS, BSP_PIN_SPI_CS, gpioModePushPull, 1);

  /* Configure to use SPI master with manual CS */
  /* For now, configure SPI for worst case 48/32MHz clock in order to work */
  /* for all configurations. */

  #if defined(_EFM32_GECKO_FAMILY)
    bcinit.refFreq  = 32000000;
  #else
    bcinit.refFreq  = 48000000;
  #endif
  bcinit.baudrate = 7000000;

  /* Initialize USART */
  USART_InitSync(BSP_SPI_USART_USED, &bcinit);

  /* Enable pins at default location */
  BSP_SPI_USART_USED->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN;
}

static void SpiControl(BSP_SpiControl_TypeDef device)
{
  switch (device)
  {
  case BSP_SPI_Audio:
    BSP_RegisterWrite(&BC_REGISTER->SPI_DEMUX, BC_SPI_DEMUX_SLAVE_AUDIO);
    break;

  case BSP_SPI_Ethernet:
    BSP_RegisterWrite(&BC_REGISTER->SPI_DEMUX, BC_SPI_DEMUX_SLAVE_ETHERNET);
    break;

  case BSP_SPI_Display:
    BSP_RegisterWrite(&BC_REGISTER->SPI_DEMUX, BC_SPI_DEMUX_SLAVE_DISPLAY);
    break;

  case BSP_SPI_OFF:
    USART_Reset(USART1);
    CMU_ClockEnable(cmuClock_USART1, false);
    break;
  }
}

static bool SpiInit(void)
{
  uint16_t bcMagic;

  /* Enable HF and GPIO clocks */
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);

  SpiBcInit();
  /* Read "board control Magic" register to verify SPI is up and running */
  /*  if not FPGA is configured to be in EBI mode  */
  bcMagic = SpiRegisterRead(&BC_REGISTER->MAGIC);
  if (bcMagic != BC_MAGIC_VALUE)
  {
    return false;
  }
  else
  {
    return true;
  }
}

static uint16_t SpiRegisterRead(volatile uint16_t *addr)
{
  uint16_t data;

  if (addr != lastAddr)
  {
    SpiBcAccess(0x00, 0, 0xFFFF & ((uint32_t) addr));           /* LSBs of address */
    SpiBcAccess(0x01, 0, 0xFF & ((uint32_t) addr >> 16));       /* MSBs of address */
    SpiBcAccess(0x02, 0, (0x0C000000 & (uint32_t) addr) >> 26); /* Chip select */
  }
  /* Read twice; when register address has changed we need two SPI transfer
   * to clock out valid data through board controller FIFOs */
  data     = SpiBcAccess(0x03, 1, 0);
  data     = SpiBcAccess(0x03, 1, 0);

  lastAddr = addr;
  return data;
}

static void SpiRegisterWrite(volatile uint16_t *addr, uint16_t data)
{
  if (addr != lastAddr)
  {
    SpiBcAccess(0x00, 0, 0xFFFF & ((uint32_t) addr));           /* LSBs of address */
    SpiBcAccess(0x01, 0, 0xFF & ((uint32_t) addr >> 16));       /* MSBs of address */
    SpiBcAccess(0x02, 0, (0x0C000000 & (uint32_t) addr) >> 26); /* Chip select */
  }
  SpiBcAccess(0x03, 0, data);                                   /* Data */
  lastAddr = addr;
}

/** @endcond */
#endif  /* BSP_DK_BRD3201 */
