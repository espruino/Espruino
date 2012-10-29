#include "platform_config.h"
#ifdef USB
 #include "usb_utils.h"
 #include "usb_lib.h"
 #include "usb_desc.h"
 #include "usb_pwr.h"
#endif
#include "jsinteractive.h"
#include "jshardware.h"



int main(void){	
  jshInit();
#ifdef USB
  Set_System();
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
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

