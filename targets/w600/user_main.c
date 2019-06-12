#include "wm_include.h"
#include "wm_gpio.h"

#include "jshardware.h"
#include "jsvar.h"
#include "jsinteractive.h"

#define ES_STK_SIZE	1024
static OS_STK   es_sys_task_stk[ES_STK_SIZE];


static void es_sys_task(void *sdata){

  // jshInitDefaultConsole();

  jshInit();
  jsvInit(0);
  jsiInit(true);
  
  //jswrap_wifi_restore();

  while (1) {
    jsiLoop();
  }
}

void UserMain(void){

  tls_os_task_create(NULL, 
      NULL,
      es_sys_task,
      NULL,
      (void *)es_sys_task_stk,
      ES_STK_SIZE * sizeof(u32),
      29,
      0);

}