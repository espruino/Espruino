#include "platform_config.h"
#ifdef USB
#ifdef STM32F1
 #include "usb_utils.h"
 #include "usb_lib.h"
 #include "usb_desc.h"
 #include "usb_pwr.h"
#endif
#ifdef STM32F4
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usb_conf.h"
#include "usbd_desc.h"
#endif
#endif
#include "jsinteractive.h"
#include "jshardware.h"



int main(void){	
  jshInit();
#ifdef USB
#ifdef STM32F1
  Set_System();
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
#endif
#ifdef STM32F4
  USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_HS
            USB_OTG_HS_CORE_ID,
#else
            USB_OTG_FS_CORE_ID,
#endif
            &USR_desc,
            &USBD_CDC_cb,
            &USR_cb);
#endif
#endif

  volatile int w,h;
#ifdef USB
  for (w=0;w<1000000;w++)
    for (h=0;h<2;h++); // wait for things to settle (for USB)
#else
  for (w=0;w<100000;w++)
    for (h=0;h<2;h++); // wait for things to settle (for Serial comms)
#endif

  bool buttonState = false;
#ifdef BTN_PORT
  buttonState = GPIO_ReadInputDataBit(BTN_PORT, BTN_PIN);
#endif  
  jsiInit(!buttonState); // pressing USER button skips autoload


  int counter = 0;
  while (1) {
    jsiLoop();

/*#ifdef LED1_PORT
    counter++;
    GPIO_WriteBit(LED1_PORT,LED1_PIN, (counter>>13) & 1);
#endif*/
  }
  //jsiKill();
  //jshKill();
}


/*******************************************************************************
* Function Name  : main.
* Description    : Main routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
/*int main(void)
{
  Set_System();
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
  
  while (1)
  {
  }
}*/

#ifdef USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

