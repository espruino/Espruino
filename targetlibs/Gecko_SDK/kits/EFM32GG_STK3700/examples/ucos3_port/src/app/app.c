/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2010; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                         uC/OS-III example code
*                                       Main application function
*
*                                   Silicon Labs EFM32 (EFM32GG990F1024)
*                                              with the
*                            Silicon Labs EFM32GG990F1024-STK Starter Kit
*
* @file   app.c
* @brief
* @version 4.2.1
******************************************************************************
* @section License
* <b>(C) Copyright 2013 Energy Micro AS, http://www.energymicro.com</b>
*******************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
* 4. The source and compiled code may only be used on Energy Micro "EFM32"
*    microcontrollers and "EFR4" radios.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
* obligation to support this Software. Energy Micro AS is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Energy Micro AS will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/
#include <includes.h>


/*
*********************************************************************************************************
*                                           MACRO DEFINITIONS
*********************************************************************************************************
*/
/* Message queue size */
#define MSG_Q_SIZE  10U


/*
*********************************************************************************************************
*                                         LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
/* Task specific stacks */
static CPU_STK App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static CPU_STK App_TaskOneStk[APP_CFG_TASK_ONE_STK_SIZE];
static CPU_STK App_TaskTwoStk[APP_CFG_TASK_TWO_STK_SIZE];
static CPU_STK App_TaskThreeStk[APP_CFG_TASK_THREE_STK_SIZE];

/* Task control blocks */
static  OS_TCB AppTaskStartTCB;
static  OS_TCB AppTaskOneTCB;
static  OS_TCB AppTaskTwoTCB;
static  OS_TCB AppTaskThreeTCB;

/* Message queue */
static  OS_Q   AppMsgQueue[MSG_Q_SIZE];


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
/* Initialization functions */
static void App_TaskStart(void *p_arg);
static void App_TaskCreate(void);
static void App_MailboxCreate(void);

/* Static function for energyAware Profiler */
static void setupSWO(void);


/*
*********************************************************************************************************
*                                      EXTERNAL GLOBAL VARIABLES
*********************************************************************************************************
*/
/* Definition of global mailbox object for inter-task communication
 * extern declaration in includes.h */
OS_Q *pSerialQueObj = AppMsgQueue;


/*
*********************************************************************************************************
*                                      EXTERNAL GLOBAL FUNCTIONS
*********************************************************************************************************
*/
extern void App_TaskIdleHook(void);


/*
*********************************************************************************************************
*                                         FUNCTION DEFINITIONS
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/
int main(void)
{
  OS_ERR err = OS_ERR_NONE;


  /* Disable all interrupts until we are ready to accept
   * them.                                                */
  CPU_IntDis();

  /* Chip errata */
  CHIP_Init();

  /* setup SW0 for energyAware Profiler */
  setupSWO();

  /* Initialize "uC/OS-III, The Real-Time Kernel".        */
  OSInit(&err);

  /* Create the start task                                */
  OSTaskCreate((void           *)&AppTaskStartTCB,   /* pointer to TCB */
               (CPU_CHAR       *) "Start", /* task name, text */
               (void (*)(void *)) App_TaskStart, /* function pointer to task */
               (void           *) 0U, /* pointer to optional data */
               (CPU_INT08U      ) APP_CFG_TASK_START_PRIO, /* priority */
               (CPU_STK        *) App_TaskStartStk, /* stack base ptr */
               (CPU_STK_SIZE    )(APP_CFG_TASK_START_STK_SIZE / 10U), /* stack limit */
               (CPU_STK_SIZE    ) APP_CFG_TASK_START_STK_SIZE, /* stack size */
               (OS_MSG_QTY      ) 0U, /* q size */
               (OS_TICK         ) 0U, /* round robin time tick */
               (void           *) 0U, /* p ext */
               (OS_OPT          )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), /* options */
               (OS_ERR         *)&err);

  /* Setup the idle task */
  OS_AppIdleTaskHookPtr = (OS_APP_HOOK_VOID)App_TaskIdleHook;

  /* Start multitasking (i.e. give control to uC/OS-III).  */
  OSStart(&err);

  /* OSStart() never returns, serious error had occured if
   * code execution reached this point                    */
  while(1U) ;
}


/*
*********************************************************************************************************
*                                          AppTaskStart()
*
* Description : The startup task. The uC/OS-III ticker should only be initialize once multitasking starts.
*
* Argument(s) : p_arg       Argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Note(s)     : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*
*               (2) Interrupts are enabled once the task starts because the I-bit of the CCR register was
*                   set to 0 by 'OSTaskCreate()'.
*********************************************************************************************************
*/
static void App_TaskStart(void *p_arg)
{
  uint16_t osVersion1, osVersion2, osVersion3;

  OS_ERR err = OS_ERR_NONE;

  (void)p_arg; /* Note(1) */


  /* Initialize BSP functions                             */
  BSPOS_Init();

  /* Initialize the uC/OS-III ticker                       */
  OS_CPU_SysTickInit(OS_CPU_SysTickClkFreq() / OS_CFG_TICK_RATE_HZ);

#if (OS_TASK_STAT_EN > 0U)
  /* Determine CPU capacity                               */
  OSStatInit();
#endif

  /* Create application tasks                             */
  App_TaskCreate();

  /* Create application mailboxes                         */
  App_MailboxCreate();

  /* Initialize LCD                                       */
  SegmentLCD_Init(true);

  /* Turn gecko symbol ON                                 */
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, 1);

  /* Turn EFM32 symbol ON                                 */
  SegmentLCD_Symbol(LCD_SYMBOL_EFM32, 1);

  /* Write welcome message on LCD                         */
  SegmentLCD_Write("uC/OS-3");

/* As USART connectors are not available on the STK by default,
 * therefore printf() functions are turned off.
 * Uncomment the macro definition in includes.h if serial
 * is connected to your STK board (USART1 or LEUART0)!    */
#ifdef USART_CONNECTED

  /* Initialize serial port                               */
  RETARGET_SerialInit();
  RETARGET_SerialCrLf(1);

  osVersion3 = OSVersion( &err );
  osVersion1 = osVersion3 / 10000;
  osVersion3 -= osVersion1 * 10000;
  osVersion2 = osVersion3 / 100;
  osVersion3 -= osVersion2 * 100;
  osVersion3 %= 100;

  /* Write welcome message on serial                      */
  printf("\n*****************************************************************************");
  printf("\n                    uC/OS-III v%d.%02d.%02d on Silicon Labs EFM32 STK             ",
         osVersion1, osVersion2, osVersion3 );
  printf("\n                               Demo Application                              \n");
  printf("\n                                   uC/OS-III                                  ");
  printf("\n                           \"The real time kernel\"                            ");
  printf("\n                               www.micrium.com                               ");
  printf("\n\n                                is running on                              ");
  printf("\n\n                             Silicon Labs EFM32                              ");
  printf("\n            \"The world's most energy friendly microcontrollers\"              ");
  printf("\n                              www.silabs.com                                 \n");
  printf("\nDescription:");
  printf("\nTask1: LED blink task");
  printf("\nTask2: Receives characters from serial and posts message to Task3");
  printf("\nTask3: Receives message from Task2 and writes it on LCD and serial.");
  printf("\n*****************************************************************************\n");
  printf("\nStart typing...\n");

#endif /* end of #ifndef USART_CONNECTED */

  /* Suspend this task as it is only used once in one Reset cycle */
  OSTaskSuspend(&AppTaskStartTCB, &err);

  /* Error had occured if code execution reached this point as suspend calls the scheduler
   * that performs a context switch */
  while (1U) ;
}


/*
*********************************************************************************************************
*                                      App_MailboxCreate()
*
* Description : Create the application Mailboxes
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static void App_MailboxCreate (void)
{
  OS_ERR err = OS_ERR_NONE;


  /* Create message queue for messaging received serial data between tasks */
  OSQCreate(pSerialQueObj, "MsgBox", MSG_Q_SIZE, &err); /* OS-III */

  /* Error check */
  if(OS_ERR_NONE != err)
  {
    /* An error has been ocurred during message queue initialization */
    while(1U);
  }
}


/*
*********************************************************************************************************
*                                      App_TaskCreate()
*
* Description : Create the application tasks.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static void App_TaskCreate (void)
{
  OS_ERR err = OS_ERR_NONE;


  /* Create the One task                         */
  OSTaskCreate((void           *)&AppTaskOneTCB,   /* pointer to TCB */
               (CPU_CHAR       *)"AppTaskOne", /* task name, text */
               (void (*)(void *)) APP_TaskOne, /* function pointer to task */
               (void           *) 0U, /* pointer to optional data */
               (CPU_INT08U      ) APP_CFG_TASK_ONE_PRIO, /* priority */
               (CPU_STK        *) App_TaskOneStk, /* stack base ptr */
               (CPU_STK_SIZE    )(APP_CFG_TASK_ONE_STK_SIZE / 10U), /* stack limit */
               (CPU_STK_SIZE    ) APP_CFG_TASK_ONE_STK_SIZE, /* stack size */
               (OS_MSG_QTY      ) 2U, /* q size */
               (OS_TICK         ) 0U, /* round robin time tick */
               (void           *) 0U, /* p ext */
               (OS_OPT          )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), /* options */
               (OS_ERR         *)&err);

  /* Create the Two task                         */
  OSTaskCreate((void           *)&AppTaskTwoTCB,   /* pointer to TCB */
               (CPU_CHAR       *)"AppTaskTwo", /* task name, text */
               (void (*)(void *)) APP_TaskTwo, /* function pointer to task */
               (void           *) 0U, /* pointer to optional data */
               (CPU_INT08U      ) APP_CFG_TASK_TWO_PRIO, /* priority */
               (CPU_STK        *) App_TaskTwoStk, /* stack base ptr */
               (CPU_STK_SIZE    )(APP_CFG_TASK_TWO_STK_SIZE / 10U), /* stack limit */
               (CPU_STK_SIZE    ) APP_CFG_TASK_TWO_STK_SIZE, /* stack size */
               (OS_MSG_QTY      ) 2U, /* q size */
               (OS_TICK         ) 0U, /* round robin time tick */
               (void           *) 0U, /* p ext */
               (OS_OPT          )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), /* options */
               (OS_ERR         *)&err);


  /* Create the Three task                        */
  OSTaskCreate((void           *)&AppTaskThreeTCB,   /* pointer to TCB */
               (CPU_CHAR       *)"AppTaskThree", /* task name, text */
               (void (*)(void *)) APP_TaskThree, /* function pointer to task */
               (void           *) 0U, /* pointer to optional data */
               (CPU_INT08U      ) APP_CFG_TASK_THREE_PRIO, /* priority */
               (CPU_STK        *) App_TaskThreeStk, /* stack base ptr */
               (CPU_STK_SIZE    )(APP_CFG_TASK_THREE_STK_SIZE / 10U), /* stack limit */
               (CPU_STK_SIZE    ) APP_CFG_TASK_THREE_STK_SIZE, /* stack size */
               (OS_MSG_QTY      ) 2U, /* q size */
               (OS_TICK         ) 0U, /* round robin time tick */
               (void           *) 0U, /* p ext */
               (OS_OPT          )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), /* options */
               (OS_ERR         *)&err);

  /* Error check */
  if(OS_ERR_NONE != err)
  {
    /* An error has been ocurred */
    while(1U) ;
  }
}


/*
*********************************************************************************************************
*                                              setupSW0()
*
* Description : Setup SW0 for energyAware Profiler.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : none.
********************************************************************************************************
*/
static void setupSWO(void)
{
  uint32_t *dwt_ctrl = (uint32_t *) 0xE0001000;
  uint32_t *tpiu_prescaler = (uint32_t *) 0xE0040010;
  uint32_t *tpiu_protocol = (uint32_t *) 0xE00400F0;

  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;
  /* Enable Serial wire output pin */
  GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;
  /* Set location 1 */
  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC1;
  /* Enable output on pin */
  GPIO->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE15_MASK);
  GPIO->P[2].MODEH |= GPIO_P_MODEH_MODE15_PUSHPULL;
  /* Enable debug clock AUXHFRCO */
  CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;

  while(!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY));

  /* Enable trace in core debug */
  CoreDebug->DHCSR |= 1;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  /* Enable PC and IRQ sampling output */
  *dwt_ctrl = 0x400113FF;
  /* Set TPIU prescaler to 16. */
  *tpiu_prescaler = 0xf;
  /* Set protocol to NRZ */
  *tpiu_protocol = 2;
  /* Unlock ITM and output data */
  ITM->LAR = 0xC5ACCE55;
  ITM->TCR = 0x10009;
}
