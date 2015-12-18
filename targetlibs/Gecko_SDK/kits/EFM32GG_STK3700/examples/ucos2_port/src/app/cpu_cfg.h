/*
*********************************************************************************************************
*                                                uC/CPU
*                                    CPU CONFIGURATION & PORT LAYER
*
*                          (c) Copyright 2004-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/CPU is provided in source form to registered licensees ONLY.  It is 
*               illegal to distribute this source code to any third party unless you receive 
*               written permission by an authorized Micrium representative.  Knowledge of 
*               the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest 
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                       CPU CONFIGURATION FILE
*
*                                              TEMPLATE
*
* Filename      : cpu_cfg.h
* Version       : V1.29.02
* Programmer(s) : SR
*                 ITJ
*                 JBL
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/

#ifndef  CPU_CFG_MODULE_PRESENT
#define  CPU_CFG_MODULE_PRESENT


/*
*********************************************************************************************************
*                                       CPU NAME CONFIGURATION
*
* Note(s) : (1) Configure CPU_CFG_NAME_EN to enable/disable CPU host name feature :
*
*               (a) CPU host name storage
*               (b) CPU host name API functions
*
*           (2) Configure CPU_CFG_NAME_SIZE with the desired ASCII string size of the CPU host name, 
*               including the terminating NULL character.
*
*               See also 'cpu_core.h  GLOBAL VARIABLES  Note #1'.
*********************************************************************************************************
*/

                                                                /* Configure CPU host name feature (see Note #1) :      */
#define  CPU_CFG_NAME_EN                        DEF_DISABLED
                                                                /*   DEF_DISABLED  CPU host name DISABLED               */
                                                                /*   DEF_ENABLED   CPU host name ENABLED                */

                                                                /* Configure CPU host name ASCII string size ...        */
#define  CPU_CFG_NAME_SIZE                                16    /* ... (see Note #2).                                   */


/*
*********************************************************************************************************
*                                     CPU TIMESTAMP CONFIGURATION
*
* Note(s) : (1) Configure CPU_CFG_TS_xx_EN to enable/disable CPU timestamp features :
*
*               (a) CPU_CFG_TS_32_EN   enable/disable 32-bit CPU timestamp feature
*               (b) CPU_CFG_TS_64_EN   enable/disable 64-bit CPU timestamp feature
*
*           (2) (a) Configure CPU_CFG_TS_TMR_SIZE with the CPU timestamp timer's word size :
*
*                       CPU_WORD_SIZE_08         8-bit word size
*                       CPU_WORD_SIZE_16        16-bit word size
*                       CPU_WORD_SIZE_32        32-bit word size
*                       CPU_WORD_SIZE_64        64-bit word size
*
*               (b) If the size of the CPU timestamp timer is not a binary multiple of 8-bit octets 
*                   (e.g. 20-bits or even 24-bits), then the next lower, binary-multiple octet word 
*                   size SHOULD be configured (e.g. to 16-bits).  However, the minimum supported word 
*                   size for CPU timestamp timers is 8-bits.
*
*                   See also 'cpu_core.h  FUNCTION PROTOTYPES  CPU_TS_TmrRd()  Note #2a'.
*********************************************************************************************************
*/

                                                                /* Configure CPU timestamp features (see Note #1) :     */
#define  CPU_CFG_TS_32_EN                       DEF_DISABLED
#define  CPU_CFG_TS_64_EN                       DEF_DISABLED
                                                                /*   DEF_DISABLED  CPU timestamps DISABLED              */
                                                                /*   DEF_ENABLED   CPU timestamps ENABLED               */

                                                                /* Configure CPU timestamp timer word size ...          */
                                                                /* ... (see Note #2) :                                  */
#define  CPU_CFG_TS_TMR_SIZE                    CPU_WORD_SIZE_32


/*
*********************************************************************************************************
*                        CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION
*
* Note(s) : (1) (a) Configure CPU_CFG_INT_DIS_MEAS_EN to enable/disable measuring CPU's interrupts 
*                   disabled time :
*
*                   (a)  Enabled,       if CPU_CFG_INT_DIS_MEAS_EN      #define'd in 'cpu_cfg.h'
*
*                   (b) Disabled,       if CPU_CFG_INT_DIS_MEAS_EN  NOT #define'd in 'cpu_cfg.h'
*
*                   See also 'cpu_core.h  FUNCTION PROTOTYPES  Note #1'.
*
*               (b) Configure CPU_CFG_INT_DIS_MEAS_OVRHD_NBR with the number of times to measure & 
*                   average the interrupts disabled time measurements overhead.
*
*                   Recommend a single (1) overhead time measurement, even for instruction-cache-enabled 
*                   CPUs, since critical sections are NOT typically called within instruction-cached loops.
*                   Thus a single non-cached/non-averaged time measurement is a more realistic overhead 
*                   for the majority of non-cached interrupts disabled time measurements.
*
*                   See also 'cpu_core.c  CPU_IntDisMeasInit()  Note #3a'.
*********************************************************************************************************
*/

#if 0                                                           /* Configure CPU interrupts disabled time ...           */
#define  CPU_CFG_INT_DIS_MEAS_EN                                /* ... measurements feature (see Note #1a).             */
#endif

                                                                /* Configure number of interrupts disabled overhead ... */
#define  CPU_CFG_INT_DIS_MEAS_OVRHD_NBR                    1u   /* ... time measurements (see Note #1b).                */


/*
*********************************************************************************************************
*                                    CPU COUNT ZEROS CONFIGURATION
*
* Note(s) : (1) (a) Configure CPU_CFG_LEAD_ZEROS_ASM_PRESENT  to define count leading  zeros bits 
*                   function(s) in :
*
*                   (1) 'cpu_a.asm',  if CPU_CFG_LEAD_ZEROS_ASM_PRESENT       #define'd in 'cpu.h'/
*                                         'cpu_cfg.h' to enable assembly-optimized function(s)
*
*                   (2) 'cpu_core.c', if CPU_CFG_LEAD_ZEROS_ASM_PRESENT   NOT #define'd in 'cpu.h'/
*                                         'cpu_cfg.h' to enable C-source-optimized function(s) otherwise
*
*               (b) Configure CPU_CFG_TRAIL_ZEROS_ASM_PRESENT to define count trailing zeros bits 
*                   function(s) in :
*
*                   (1) 'cpu_a.asm',  if CPU_CFG_TRAIL_ZEROS_ASM_PRESENT      #define'd in 'cpu.h'/
*                                         'cpu_cfg.h' to enable assembly-optimized function(s)
*
*                   (2) 'cpu_core.c', if CPU_CFG_TRAIL_ZEROS_ASM_PRESENT  NOT #define'd in 'cpu.h'/
*                                         'cpu_cfg.h' to enable C-source-optimized function(s) otherwise
*********************************************************************************************************
*/

#if 0                                                           /* Configure CPU count leading  zeros bits ...          */
#define  CPU_CFG_LEAD_ZEROS_ASM_PRESENT                         /* ... assembly-version (see Note #1a).                 */
#endif

#if 0                                                           /* Configure CPU count trailing zeros bits ...          */
#define  CPU_CFG_TRAIL_ZEROS_ASM_PRESENT                        /* ... assembly-version (see Note #1b).                 */
#endif


/*
*********************************************************************************************************
*                                      CPU ENDIAN TYPE OVERRIDE
*
* Note(s) : (1) Configure CPU_CFG_ENDIAN_TYPE to override the default CPU endian type defined in cpu.h.
*
*               (a) CPU_ENDIAN_TYPE_BIG         Big-   endian word order (CPU words' most  significant
*                                                                         octet @ lowest memory address)
*               (b) CPU_ENDIAN_TYPE_LITTLE      Little-endian word order (CPU words' least significant
*                                                                         octet @ lowest memory address)
*
*           (2) Defining CPU_CFG_ENDIAN_TYPE here is only valid for supported bi-endian architectures.
*               See  'cpu.h  CPU WORD CONFIGURATION  Note #3' for details
*********************************************************************************************************
*/

#if 0
#define  CPU_CFG_ENDIAN_TYPE            CPU_ENDIAN_TYPE_BIG     /* Defines CPU data    word-memory order (see Note #2). */
#endif


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of CPU cfg module include.                       */

