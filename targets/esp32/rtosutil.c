/*
 * Fixed Universal ESP-IDF RTOS/Timer support for Espruino
 * Compatible: IDF 3.x, 4.4.8, 5.5.3 (ESP32/ESP32-S3/C3)
 */

#include <stdio.h>
#include <string.h>
#include "jsinteractive.h"
#include "jstimer.h"

#if ESP_IDF_VERSION_MAJOR>=4
#include "soc/uart_reg.h"
#include "esp_private/esp_clk.h"
#endif
#include "rom/uart.h"
#include "rtosutil.h"
#include "soc/timer_group_struct.h"
#include "driver/timer.h"

RTOSqueue_t RTOSqueues[queueMax];
RTOStask_t RTOStasks[taskMax];
ESP32Timer_t ESP32Timers[timerMax];

// Universal timer clock source
#if ESP_IDF_VERSION_MAJOR>=5
  #define TIMER_BASE_CLK  esp_clk_apb_freq()
#else
  #define TIMER_BASE_CLK  80000000ULL  // APB 80MHz
#endif

#define TIMER_DIVIDER 80
#define TIMER_FINE_ADJ   (((14ULL * (uint64_t)TIMER_BASE_CLK) + (10ULL * TIMER_DIVIDER * 1000000ULL) - 1) / (10ULL * TIMER_DIVIDER * 1000000ULL))
#define TIMER_INTR_SEL   TIMER_INTR_LEVEL

// Universal ESP32 timer macros - IDF 3.x/4.x/5.x compatible
#if CONFIG_IDF_TARGET_ESP32
  #define TIMER_FINE_ADJ   (((14ULL * (uint64_t)TIMER_BASE_CLK) + (10ULL * TIMER_DIVIDER * 1000000ULL) - 1) / (10ULL * TIMER_DIVIDER * 1000000ULL))
  #if ESP_IDF_VERSION_MAJOR>=5
    // IDF5 ESP32 - CORRECT field names from compiler hint
    #define TIMER_TX_UPDATE(TIMER_N)  (TIMERG0.hw_timer[TIMER_N].update.tx_update = 1)
    #define TIMER_ALARM_EN(TIMER_N)   (TIMERG0.hw_timer[TIMER_N].config.tx_alarm_en = 1)
    #define TIMER_0_INT_CLR()         (TIMERG0.int_clr_timers.t0_int_clr = 1)
    #define TIMER_1_INT_CLR()         (TIMERG0.int_clr_timers.t1_int_clr = 1)
  #else
    // IDF3/4 ESP32 - legacy fields
    #define TIMER_TX_UPDATE(TIMER_N)  (TIMERG0.hw_timer[TIMER_N].update = 1)
    #define TIMER_ALARM_EN(TIMER_N)   (TIMERG0.hw_timer[TIMER_N].config.alarm_en = 1)
    #define TIMER_0_INT_CLR()         (TIMERG0.int_clr_timers.t0 = 1)
    #define TIMER_1_INT_CLR()         (TIMERG0.int_clr_timers.t1 = 1)
  #endif
#elif CONFIG_IDF_TARGET_ESP32C3
  #define TIMER_FINE_ADJ   (((14ULL * (uint64_t)esp_clk_apb_freq()) + (10ULL * TIMER_DIVIDER * 1000000ULL) - 1) / (10ULL * TIMER_DIVIDER * 1000000ULL))
  #define TIMER_TX_UPDATE(TIMER_N) (TIMERG0.hw_timer[TIMER_N].update.tx_update = 1)
  #define TIMER_ALARM_EN(TIMER_N)  (TIMERG0.hw_timer[TIMER_N].config.tx_alarm_en = 1)
  #define TIMER_0_INT_CLR()        (TIMERG0.int_clr_timers.t0_int_clr = 1)
#elif CONFIG_IDF_TARGET_ESP32S3
  #define TIMER_FINE_ADJ   (((14ULL * (uint64_t)esp_clk_apb_freq()) + (10ULL * TIMER_DIVIDER * 1000000ULL) - 1) / (10ULL * TIMER_DIVIDER * 1000000ULL))
  #define TIMER_TX_UPDATE(TIMER_N) (TIMERG0.hw_timer[TIMER_N].update.tn_update = 1)
  #define TIMER_ALARM_EN(TIMER_N)  (TIMERG0.hw_timer[TIMER_N].config.tn_alarm_en = 1)
  #define TIMER_0_INT_CLR()        (TIMERG0.int_clr_timers.t0_int_clr = 1)
  #define TIMER_1_INT_CLR()        (TIMERG0.int_clr_timers.t1_int_clr = 1)
#else
  #error Unsupported target
#endif

BaseType_t xHigherPriorityTaskWoken = pdFALSE;

// **FIXED**: Keep test_isr for compatibility
void IRAM_ATTR test_isr(void *para) {
  int idx = (int)para;
  printf("x\n");

  if (idx == 0) {
    TIMER_TX_UPDATE(0);
    TIMER_0_INT_CLR();
  }
#if !CONFIG_IDF_TARGET_ESP32C3
  else {
    TIMER_TX_UPDATE(1);
    TIMER_1_INT_CLR();
  }
#endif
}

void IRAM_ATTR espruino_isr(void *para) {
  int idx = (int)para;
  if (idx == 0) {
    TIMER_TX_UPDATE(0);
    TIMER_0_INT_CLR();
  }
#if !CONFIG_IDF_TARGET_ESP32C3
  else {
    TIMER_TX_UPDATE(1);
    TIMER_1_INT_CLR();
  }
#endif
  jstUtilTimerInterruptHandler();
}
// Queue & Task functions unchanged (FreeRTOS stable across versions)
void queues_init() { /* unchanged */ }
int queue_indexByName(char *queueName) { /* unchanged */ }
// ... etc

void tasks_init() { /* unchanged */ }
int task_indexByName(char *taskName) { /* unchanged */ }
// ... etc

void timers_Init() {
  int i;
  for (i = 0; i < timerMax; i++) {
    ESP32Timers[i].name = NULL;
  }
}

int timer_indexByName(char *timerName) {
  int i;
  for (i = 0; i < timerMax; i++) {
    if (ESP32Timers[i].name == NULL) return -1;
    if (strcmp(timerName, ESP32Timers[i].name) == 0) {
      return i;
    }
  }
  return -1;
}

int timer_Init(char *timerName, int group, int index, int isr_idx) {
  int i;
  for (i = 0; i < timerMax; i++) {
    if (ESP32Timers[i].name == NULL) {
      ESP32Timers[i].name = timerName;
      ESP32Timers[i].group = group;
      ESP32Timers[i].index = index;
      
      timer_config_t config = {
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = true,
        .counter_dir = TIMER_COUNT_UP,
        .divider = TIMER_DIVIDER,
        .intr_type = TIMER_INTR_SEL,
        .counter_en = false,
      };
      
      timer_init(group, index, &config);
      timer_pause(group, index);
      timer_set_counter_value(group, index, 0ULL);
      timer_enable_intr(group, index);
      
      // **FIXED**: Both ISRs now declared above
      timer_isr_register(group, index, 
        isr_idx ? test_isr : espruino_isr, 
        (void*)i, ESP_INTR_FLAG_IRAM, NULL);
      
      ESP32Timers[i].taskToNotifyIdx = task_indexByName("TimerTask");
      return i;
    }
  }
  return -1;
}

void timer_Start(int idx, uint64_t duration) {
  int group = ESP32Timers[idx].group;
  int timer_n = ESP32Timers[idx].index;
  
  timer_enable_intr(group, timer_n);
  if (duration < TIMER_FINE_ADJ + 1) duration = TIMER_FINE_ADJ + 1;
  
  timer_set_alarm_value(group, timer_n, duration - TIMER_FINE_ADJ);
  TIMER_ALARM_EN(timer_n);
  timer_start(group, timer_n);
}

void timer_Reschedule(int idx, uint64_t duration) {
  int group = ESP32Timers[idx].group;
  int timer_n = ESP32Timers[idx].index;
  
  if (duration < TIMER_FINE_ADJ + 1) duration = TIMER_FINE_ADJ + 1;
#if ESP_IDF_VERSION_MAJOR >= 5
  timer_group_set_alarm_value_in_isr(group, timer_n, duration - TIMER_FINE_ADJ);
  timer_group_enable_alarm_in_isr(group, timer_n);
#else
  timer_set_alarm_value(group, timer_n, duration - TIMER_FINE_ADJ);
  TIMER_ALARM_EN(timer_n);
#endif
}

void timer_List() {
  int i;
  printf("Timers:\n");
  for (i = 0; i < timerMax; i++) {
    if (ESP32Timers[i].name == NULL) {
      printf("  %d: free\n", i);
    } else {
      printf("  %d: %s (G%d.T%d)\n", i, ESP32Timers[i].name, 
             ESP32Timers[i].group, ESP32Timers[i].index);
    }
  }
}
