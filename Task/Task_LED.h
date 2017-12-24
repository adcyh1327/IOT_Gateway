#ifndef __TASK_LED_H
#define __TASK_LED_H
#include "ucos_ii.h"

extern OS_EVENT *mBOX_LED_R;
extern OS_EVENT *mBOX_LED_G;
extern unsigned char led_blink;

#define S_FAULT                mBOX_LED_R
#define S_NORMAL               mBOX_LED_G 

#define LED_BLINK_ONCE(sem_led)  OSMboxPost(sem_led,(void *)&led_blink);

void Task_LedR(void *p_arg);
void Task_LedG(void *p_arg);



#endif


