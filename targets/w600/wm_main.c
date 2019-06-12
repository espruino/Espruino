/*****************************************************************************
*
* File Name : wm_main.c
*
* Description: wm main
*
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd.
* All rights reserved.
*
* Author :
*
* Date : 2014-6-14
*****************************************************************************/
#include <string.h>
#include "wm_irq.h"
#include "tls_sys.h"

#include "wm_regs.h"
#include "wm_type_def.h"
#include "wm_timer.h"
#include "wm_irq.h"
#include "wm_params.h"
#include "wm_hostspi.h"
#include "wm_flash.h"
#include "wm_fls_gd25qxx.h"
#include "wm_internal_flash.h"
#include "wm_efuse.h"
#include "wm_debug.h"
#include "wm_netif.h"
#include "wm_at_ri_init.h"
#include "wm_config.h"
#include "wm_osal.h"
#include "wm_http_client.h"
#include "wm_cpu.h"
#include "misc.h"
#include "wm_webserver.h"
#include "wm_io.h"
#include "wm_mem.h"
#include "wm_wl_task.h"
#include "wm_wl_timers.h"
#include "wm_watchdog.h"
#ifdef TLS_CONFIG_HARD_CRYPTO
#include "wm_crypto_hard.h"
#endif
#include "wm_gpio_afsel.h"
#include "wm_pmu.h"

/* c librayr mutex */
tls_os_sem_t    *libc_sem;
/*----------------------------------------------------------------------------
 *      Standard Library multithreading interface
 *---------------------------------------------------------------------------*/

#ifndef __MICROLIB
/*--------------------------- _mutex_initialize -----------------------------*/

int _mutex_initialize (u32 *mutex) {
    /* Allocate and initialize a system mutex. */
    //tls_os_sem_create(&libc_sem, 1);
    //mutex = (u32 *)libc_sem;
    return(1);
}


/*--------------------------- _mutex_acquire --------------------------------*/

void _mutex_acquire (u32 *mutex) {
    //u8 err;
    /* Acquire a system mutex, lock stdlib resources. */
	tls_os_sem_acquire(libc_sem, 0);
}


/*--------------------------- _mutex_release --------------------------------*/

void _mutex_release (u32 *mutex) {
    /* Release a system mutex, unlock stdlib resources. */
	tls_os_sem_release(libc_sem);
}

#endif

#define     TASK_START_STK_SIZE         1200     /* Size of each task's stacks (# of WORDs)  */

u32 TaskStartStk[TASK_START_STK_SIZE];


#define FW_MAJOR_VER           0x03
#define FW_MINOR_VER           0x02
#define FW_PATCH_VER           0x00

const char FirmWareVer[4] = {
	'G',
	FW_MAJOR_VER,  /* Main version */
	FW_MINOR_VER, /* Subversion */
	FW_PATCH_VER  /* Internal version */
	};
const char HwVer[6] = {
	'H',
	0x1,
	0x0,
	0x0,
	0x0,
	0x0
};
extern const char WiFiVer[];
extern u8 tx_gain_group[];
extern void *tls_wl_init(u8 *tx_gain, u8* mac_addr, u8 *hwver);
extern int wpa_supplicant_init(u8 *mac_addr);
extern void tls_sys_auto_mode_run(void);
extern void tls_spi_slave_sel(u16 slave);
extern void UserMain(void);
extern void tls_fls_layout_init(void);


void task_start (void *data);

void tls_os_timer_init(void)
{
	tls_sys_clk sysclk;
	
	tls_sys_clk_get(&sysclk);
	SysTick_Config(sysclk.cpuclk*UNIT_MHZ/HZ);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}
/****************/
/* main program */
/****************/

void vApplicationIdleHook( void )
{
    /* clear watch dog interrupt */
    tls_watchdog_clr();

#if !defined(__CC_ARM)
        __asm volatile ("wfi");
#else
        __WFI();
#endif

    return;
}

void SystemInit()
{
	//SCB->VTOR = (unsigned int)&$$Image$$LR$$LR_IROM1$$Base;
}

void wm_gpio_config()
{
	/* must call first */
	wm_gpio_af_disable();

	/* UART0_TX-PA04 UART0_RX-PA05 */
	wm_uart0_tx_config(WM_IO_PA_04);
	wm_uart0_rx_config(WM_IO_PA_05);

	/* UART1_RX-PB11  UART1_TX-PB12 */	
	wm_uart1_rx_config(WM_IO_PB_11);
	wm_uart1_tx_config(WM_IO_PB_12);	

	/*MASTER SPI configuratioin*/
	wm_spi_cs_config(WM_IO_PA_02);
	wm_spi_ck_config(WM_IO_PA_11);
	wm_spi_di_config(WM_IO_PA_03);
	wm_spi_do_config(WM_IO_PA_09);
}

int main(void)
{
	SystemInit();

	tls_sys_clk_set(CPU_CLK_80M);
	
	tls_pmu_clk_select(0);

	tls_os_init(NULL);

    /* before use malloc() function, must create mutex used by c_lib */
    tls_os_sem_create(&libc_sem, 1);


	{
		tls_os_task_create(NULL, NULL,
	                    task_start,
	                    (void *)0,
	                    (void *)&TaskStartStk[0],          /* 任务栈的起始地址 */
	                    TASK_START_STK_SIZE * sizeof(u32), /* 任务栈的大小     */
	                    1,
	                    0);
	}


    /* initial os ticker */
    tls_os_timer_init();


	tls_os_start_scheduler();

    return 0;
}


void disp_version_info(void)
{
    TLS_DBGPRT_INFO("\n\n");
    TLS_DBGPRT_INFO("****************************************************************\n");
    TLS_DBGPRT_INFO("*                                                              *\n");
    TLS_DBGPRT_INFO("* Copyright (C) 2014 WinnerMicro Co. Ltd.                      *\n");
    TLS_DBGPRT_INFO("* All rights reserved.                                         *\n");
    TLS_DBGPRT_INFO("* WinnerMicro Firmware Version: %x.%x.%X                         *\n",
           FirmWareVer[1], FirmWareVer[2], FirmWareVer[3]);
    TLS_DBGPRT_INFO("* WinnerMicro Hardware Version: %x.%x.%x.%x.%x                      *\n",
           HwVer[1], HwVer[2], HwVer[3],HwVer[4],HwVer[5]);
    TLS_DBGPRT_INFO("*                                                              *\n");
    TLS_DBGPRT_INFO("* WinnerMicro Wi-Fi Lib Version: %x.%x.%x                         *\n",
           WiFiVer[0], WiFiVer[1], WiFiVer[2]);
    TLS_DBGPRT_INFO("****************************************************************\n");
}


/*****************************************************************************
 * Function Name        // task_start
 * Descriptor             // before create multi_task, we create a task_start task
 *                      	   // in this example, this task display the cpu usage
 * Input
 * Output
 * Return
 ****************************************************************************/
void task_start (void *data)
{
    u8 mac_addr[6] = {0x00,0x25,0x08,0x09,0x01,0x0F};
	bool enable = FALSE;
	/* must call first to configure gpio Alternate functions according the hardware design */
	wm_gpio_config();

#if TLS_CONFIG_HARD_CRYPTO
	tls_crypto_init();
#endif

#if (TLS_CONFIG_LS_SPI)	
	tls_spi_init();
	tls_spifls_init();
#endif

	tls_fls_init();

	/*initialize flash layout parameter according to image type*/
	tls_fls_layout_init();

	/*PARAM GAIN,MAC default*/
	tls_ft_param_init();
	tls_param_load_factory_default();
	tls_param_init(); /*add param to init sysparam_lock sem*/

	tls_get_tx_gain(&tx_gain_group[0]);
	TLS_DBGPRT_INFO("tx gain ");
	TLS_DBGPRT_DUMP((char *)(&tx_gain_group[0]), 12);

	if (tls_wifi_mem_cfg(WIFI_MEM_START_ADDR, 7, 7)) /*wifi tx&rx mem customized interface*/
	{
		TLS_DBGPRT_INFO("wl mem initial failured\n");
	}

	tls_get_mac_addr(&mac_addr[0]);
	TLS_DBGPRT_INFO("mac addr ");
	TLS_DBGPRT_DUMP((char *)(&mac_addr[0]), 6);

	if(tls_wl_init(NULL, &mac_addr[0], NULL) == NULL){
		TLS_DBGPRT_INFO("wl driver initial failured\n");
	}
	if (wpa_supplicant_init(mac_addr)) {
		TLS_DBGPRT_INFO("supplicant initial failured\n");
	}

	tls_ethernet_init();

	tls_sys_init();

    /* open low power mode */
	tls_param_get(TLS_PARAM_ID_PSM, &enable, TRUE);	
	if (enable != TRUE)
	{
	    enable = TRUE;
	    tls_param_set(TLS_PARAM_ID_PSM, &enable, TRUE);	  
	}

	UserMain();
	tls_sys_auto_mode_run();

    NVIC_SystemLPConfig(NVIC_LP_SLEEPDEEP, ENABLE);
	
    for (;;)
    {
#if 1
		tls_os_time_delay(0x10000000);
#else
	//printf("start up\n");
	extern void tls_os_disp_task_stat_info(void);
	tls_os_disp_task_stat_info();
	tls_os_time_delay(1000);
#endif
    }
}

