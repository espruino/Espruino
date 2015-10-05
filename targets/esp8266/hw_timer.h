
#ifndef HW_TIMER_H
#define HW_TIMER_H

typedef enum {
    FRC1_SOURCE = 0,
    NMI_SOURCE = 1,
} FRC1_TIMER_SOURCE_TYPE;

void hw_timer_arm(u32 val);
void hw_timer_set_func(void (* user_hw_timer_cb_set)(void));
void hw_timer_init(FRC1_TIMER_SOURCE_TYPE source_type, u8 req);

#endif
