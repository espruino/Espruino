/***************************************************************************//**
 * @file
 * @brief Board support package API implementation for BRD3200.
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
#include "em_system.h"
#include "em_usart.h"
#include "bsp_dk_bcreg_3200.h"
#include "bsp.h"

#if defined( BSP_DK_BRD3200 )
/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/* USART used for SPI access */
#define BSP_SPI_USART_USED  USART2          /**< USART used for BC register interface */
#define BSP_SPI_USART_CLK   cmuClock_USART2 /**< Clock for BC register USART */

/* GPIO pins used fotr SPI pins, please refer to DK user guide. */
#define BSP_PORT_SPIBUS_CONNECT   gpioPortC   /**< SPI Bus Connection Port */
#define BSP_PIN_SPIBUS_CONNECT    13          /**< SPI Bus Connection Pin */
#define BSP_PORT_EBIBUS_CONNECT   gpioPortC   /**< EBI Bus Connection Port */
#define BSP_PIN_EBIBUS_CONNECT    12          /**< EBI Bus Connection Pin */
#define BSP_PORT_SPI_TX           gpioPortC   /**< SPI transmit GPIO port */
#define BSP_PIN_SPI_TX            2           /**< SPI transmit GPIO pin */
#define BSP_PORT_SPI_RX           gpioPortC   /**< SPI receive GPIO port */
#define BSP_PIN_SPI_RX            3           /**< SPI receive GPIO pin */
#define BSP_PORT_SPI_CLK          gpioPortC   /**< SPI clock port */
#define BSP_PIN_SPI_CLK           4           /**< SPI clock pin */
#define BSP_PORT_SPI_CS           gpioPortC   /**< SPI Chip Select port */
#define BSP_PIN_SPI_CS            5           /**< SPI Chip Select pin */

static uint16_t SpiBcAccess(uint8_t addr, uint8_t rw, uint16_t data);
static void SpiBcInit(void);
static void SpiBcDisable(void);
static bool SpiInit(void);
static uint16_t SpiRegisterRead(volatile uint16_t *addr);
static void SpiRegisterWrite(volatile uint16_t *addr, uint16_t data);
static volatile const uint16_t *lastAddr = 0; /**< Last register accessed */

#if defined( BSP_BC_CTRL_EBI )
static void EbiConfigure(void);
static bool EbiInit(void);
static void EbiDisable(void);
#endif

static BSP_BusControl_TypeDef busMode = BSP_BusControl_Undefined;

int BSP_BusControlModeSet(BSP_BusControl_TypeDef mode)
{
  (void)mode;                 /* Unused parameter. */
  return BSP_STATUS_NOT_IMPLEMENTED;
}

BSP_BusControl_TypeDef BSP_BusControlModeGet( void )
{
  return busMode;
}

uint32_t BSP_DipSwitchGet(void)
{
  return (~BSP_RegisterRead(BC_DIPSWITCH)) & 0x00ff;
}

int BSP_Disable(void)
{
#if defined( BSP_BC_CTRL_EBI )
  /* Handover bus control */
  BSP_RegisterWrite(BC_BUS_CFG, 0);
  /* Disable EBI interface */
  EbiDisable();
#else

  SpiBcDisable();
#endif

  busMode = BSP_BusControl_Undefined;

  return BSP_STATUS_OK;
}

int BSP_DisplayControl(BSP_Display_TypeDef option)
{
  (void)option;               /* Unused parameter. */
  return BSP_STATUS_NOT_IMPLEMENTED;
}

int BSP_EbiExtendedAddressRange(bool enable)
{
  (void)enable;               /* Unused parameter. */
  return BSP_STATUS_NOT_IMPLEMENTED;
}

int BSP_EnergyModeSet(uint16_t energyMode)
{
  BSP_RegisterWrite(BC_EM, energyMode);
  return BSP_STATUS_OK;
}

int BSP_Init(uint32_t flags)
{
  bool ret = false;
  (void)flags;                /* Unused parameter. */

#if defined( BSP_BC_CTRL_EBI )
  ret = EbiInit();
  busMode = BSP_BusControl_EBI;
#else
  ret = SpiInit();
  busMode = BSP_BusControl_SPI;
#endif

  if (ret == false)
  {
    /* Board is configured in wrong mode, please restart KIT! */
    while (1) ;
  }

  /* Set Energy Mode 0. */
  BSP_RegisterWrite(BC_EM, 0);

  return BSP_STATUS_OK;
}

int BSP_InterruptDisable(uint16_t flags)
{
  uint16_t tmp;

  /* Clear flags from interrupt enable register */
  tmp   = BSP_RegisterRead(BC_INTEN);
  flags = ~(flags);
  tmp  &= flags;
  BSP_RegisterWrite(BC_INTEN, tmp);
  return BSP_STATUS_OK;
}

int BSP_InterruptEnable(uint16_t flags)
{
  uint16_t tmp;

  /* Add flags to interrupt enable register */
  tmp  = BSP_RegisterRead(BC_INTEN);
  tmp |= flags;
  BSP_RegisterWrite(BC_INTEN, tmp);
  return BSP_STATUS_OK;
}

int BSP_InterruptFlagsClear(uint16_t flags)
{
  BSP_RegisterWrite(BC_INTFLAG, flags);
  return BSP_STATUS_OK;
}

uint16_t BSP_InterruptFlagsGet(void)
{
  return BSP_RegisterRead(BC_INTFLAG);
}

uint16_t BSP_JoystickGet(void)
{
  uint16_t joyStick = 0;
  uint16_t aemState;

  /* Check state */
  aemState = BSP_RegisterRead(BC_AEMSTATE);
  /* Read pushbutton status */
  if (aemState == BC_AEMSTATE_EFM)
  {
    joyStick = (~(BSP_RegisterRead(BC_JOYSTICK))) & 0x001f;
  }
  return joyStick;
}

int BSP_PeripheralAccess(BSP_Peripheral_TypeDef perf, bool enable)
{
  uint16_t bit;
  uint16_t tmp;

  /* Calculate which bit to set */
  bit = (uint16_t)perf;

  /* Read peripheral control register */
  tmp = BSP_RegisterRead(BC_PERCTRL);

  /* Enable or disable the specificed peripheral by setting board control switch */
  if (enable)
  {
    /* Enable peripheral */
    tmp |= bit;

    /* Special case for RS232, if enabled disable shutdown */
    if ((perf == BSP_RS232A) || (perf == BSP_RS232B))
    {
      /* clear shutdown bit */
      tmp &= ~(BC_PERCTRL_RS232_SHUTDOWN);
    }

    /* Special case for IRDA if enabled disable shutdown */
    if (perf == BSP_IRDA)
    {
      /* clear shutdown bit */
      tmp &= ~(BC_PERCTRL_IRDA_SHUTDOWN);
    }
  }
  else
  {
    /* Disable peripheral */
    tmp &= ~(bit);

    /* Special case for RS232, if enabled disable shutdown */
    if ((perf == BSP_RS232A) || (perf == BSP_RS232B))
    {
      /* Set shutdown bit */
      tmp |= (BC_PERCTRL_RS232_SHUTDOWN);
    }

    /* Special case for IRDA */
    if (perf == BSP_IRDA)
    {
      /* Set shutdown bit */
      tmp |= (BC_PERCTRL_IRDA_SHUTDOWN);
    }
  }

  BSP_RegisterWrite(BC_PERCTRL, tmp);

  return BSP_STATUS_OK;
}

uint16_t BSP_PushButtonsGet(void)
{
  uint16_t pb = 0;
  uint16_t aemState;

  /* Check state */
  aemState = BSP_RegisterRead(BC_AEMSTATE);
  /* Read pushbutton status */
  if (aemState == BC_AEMSTATE_EFM)
  {
    pb = (~(BSP_RegisterRead(BC_PUSHBUTTON))) & 0x000f;
  }
  return pb;
}

uint16_t BSP_RegisterRead(volatile uint16_t *addr)
{
#if defined( BSP_BC_CTRL_EBI )
  return *addr;
#else
  return SpiRegisterRead(addr);
#endif
}

int BSP_RegisterWrite(volatile uint16_t *addr, uint16_t data)
{
#if defined( BSP_BC_CTRL_EBI )
  *addr = data;
#else
  SpiRegisterWrite(addr, data);
#endif

  return BSP_STATUS_OK;
}

#if defined( BSP_BC_CTRL_EBI )
static void EbiConfigure(void)
{
  EBI_Init_TypeDef ebiConfig = EBI_INIT_DEFAULT;

  /* Run time check if we have EBI on-chip capability on this device */
  switch ( SYSTEM_GetPartNumber() )
  {
    /* Only device types EFM32G 280/290/880 and 890 have EBI capability */
    case 280:
    case 290:
    case 880:
    case 890:
      break;
    default:
      /* This device do not have EBI capability - use SPI to interface DK */
      /* With high probability your project has been configured for an */
      /* incorrect part number. */
      while (1) ;
  }

  /* Enable clocks */
  CMU_ClockEnable(cmuClock_EBI, true);
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure mode - disable SPI, enable EBI */
  GPIO_PinModeSet(gpioPortC, 13, gpioModePushPull, 1);
  GPIO_PinModeSet(gpioPortC, 12, gpioModePushPull, 0);

  /* Configure GPIO pins as push pull */
  /* EBI AD9..15 */
  GPIO_PinModeSet(gpioPortA, 0, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortA, 1, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortA, 2, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortA, 3, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortA, 4, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortA, 5, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortA, 6, gpioModePushPull, 0);

  /* EBI AD8 */
  GPIO_PinModeSet(gpioPortA, 15, gpioModePushPull, 0);

  /* EBI CS0-CS3 */
  GPIO_PinModeSet(gpioPortD, 9, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortD, 10, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortD, 11, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortD, 12, gpioModePushPull, 0);

  /* EBI AD0..7 */
  GPIO_PinModeSet(gpioPortE, 8, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortE, 9, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortE, 10, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortE, 11, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortE, 12, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortE, 14, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortE, 15, gpioModePushPull, 0);

  /* EBI ARDY/ALEN/Wen/Ren */
  GPIO_PinModeSet(gpioPortF, 2, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortF, 3, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortF, 4, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortF, 5, gpioModePushPull, 0);

  /* Configure EBI controller, changing default values */
  ebiConfig.mode = ebiModeD16A16ALE;
  /* Enable bank 0 address map 0x80000000, FPGA Flash */
  /* Enable bank 1 address map 0x84000000, FPGA SRAM */
  /* Enable bank 2 address map 0x88000000, FPGA TFT Display (SSD2119) */
  /* Enable bank 3 address map 0x8c000000, FPGA Board Control Registers */
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

  /* Polarity values are default */

  /* Configure EBI */
  EBI_Init(&ebiConfig);
}

static void EbiDisable(void)
{
  /* Disable EBI and SPI _BC_BUS_CONNECT */
  GPIO_PinModeSet(gpioPortC, 12, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortC, 13, gpioModeDisabled, 0);

  /* Configure GPIO pins as disabled */
  GPIO_PinModeSet(gpioPortA, 0, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortA, 1, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortA, 2, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortA, 3, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortA, 4, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortA, 5, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortA, 6, gpioModeDisabled, 0);

  GPIO_PinModeSet(gpioPortA, 15, gpioModeDisabled, 0);

  GPIO_PinModeSet(gpioPortD, 9, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortD, 10, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortD, 11, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortD, 12, gpioModeDisabled, 0);

  GPIO_PinModeSet(gpioPortE, 8, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortE, 9, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortE, 10, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortE, 11, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortE, 12, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortE, 13, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortE, 14, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortE, 15, gpioModeDisabled, 0);

  GPIO_PinModeSet(gpioPortF, 2, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortF, 3, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortF, 4, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortF, 5, gpioModeDisabled, 0);

  /* Disable EBI clock in CMU */
  CMU_ClockEnable(cmuClock_EBI, false);
}

/**************************************************************************//**
 * @brief Configure Gxxx_DK EBI (external bus interface) access for
 *    - 0x80000000: Board Control registers (Xilinx Spartan FPGA)
 *    - 0x84000000: TFT memory mapped drive (URT/SSD2119 controller)
 *    - 0x88000000: PSRAM external memory (Micron MT45W2MW16PGA-70 IT)
 *    - 0x8c000000: NOR flash (Spansion flash S29GLxxx_FBGA)
 * @return true if successful, false if board controller access failed
 *****************************************************************************/
static bool EbiInit(void)
{
  uint16_t ebiMagic;
  int      retry = 10;

  /* Disable all GPIO pins and register  */
  EbiDisable();
  /* Configure EBI */
  EbiConfigure();
  /* Verify that EBI access is working, if not kit is in SPI mode and needs to
   * be configured for EBI access */
  ebiMagic = BSP_RegisterRead(BC_MAGIC);
  while ((ebiMagic != BC_MAGIC_VALUE) && retry)
  {
    EbiDisable();
    /* Enable SPI interface */
    SpiInit();
    /* Set EBI mode - after this SPI access will no longer be available */
    ebiMagic = SpiRegisterRead(BC_MAGIC);
    SpiRegisterWrite(BC_CFG, BC_CFG_EBI);
    /* Disable SPI */
    SpiBcDisable();

    /* Now setup EBI again */
    EbiConfigure();
    /* Wait until ready */
    ebiMagic = BSP_RegisterRead(BC_MAGIC);
    if (ebiMagic == BC_MAGIC_VALUE) break;

    retry--;
  }
  if (!retry) return false;

  BSP_RegisterWrite(BC_LED, retry);
  return true;
}
#endif

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
  USART_Reset(BSP_SPI_USART_USED);

  /* Disable LCD_SELECT */
  GPIO_PinModeSet(gpioPortD, 13, gpioModeDisabled, 0);

  /* Disable SPI pins */
  GPIO_PinModeSet(BSP_PORT_SPIBUS_CONNECT, 13, gpioModeDisabled, 0);
  GPIO_PinModeSet(BSP_PORT_SPIBUS_CONNECT, 12, gpioModeDisabled, 0);
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

  /* Configure SPI bus connect pins, DOUT set to 0, disable EBI */
  GPIO_PinModeSet(BSP_PORT_SPIBUS_CONNECT, BSP_PIN_SPIBUS_CONNECT, gpioModePushPull, 0);
  GPIO_PinModeSet(BSP_PORT_EBIBUS_CONNECT, BSP_PIN_EBIBUS_CONNECT, gpioModePushPull, 1);

  /* Configure SPI pins */
  GPIO_PinModeSet(BSP_PORT_SPI_TX, BSP_PIN_SPI_TX, gpioModePushPull, 0);
  GPIO_PinModeSet(BSP_PORT_SPI_RX, BSP_PIN_SPI_RX, gpioModeInput, 0);
  GPIO_PinModeSet(BSP_PORT_SPI_CLK, BSP_PIN_SPI_CLK, gpioModePushPull, 0);

  /* Keep CS high to not activate slave */
  GPIO_PinModeSet(BSP_PORT_SPI_CS, BSP_PIN_SPI_CS, gpioModePushPull, 1);

  /* Configure to use SPI master with manual CS */
  /* For now, configure SPI for worst case 32MHz clock in order to work for all */
  /* configurations. */

  bcinit.refFreq  = 32000000;
  bcinit.baudrate = 7000000;

  /* Initialize USART */
  USART_InitSync(BSP_SPI_USART_USED, &bcinit);

  /* Enable pins at default location */
  BSP_SPI_USART_USED->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN;
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
  bcMagic = SpiRegisterRead(BC_MAGIC);
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
#endif  /* BSP_DK_BRD3200 */
