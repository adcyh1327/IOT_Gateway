#ifndef __TASK_TCPIP_H
#define __TASK_TCPIP_H

#include "variable.h"
#include "ucos_ii.h"

void TCPIP_Timer1ms(void);
void Task_TCPIP(void *p_arg);
void TCPIP_DataHandler(u8 chnidx,struct USARTCAN_Recv_t *recv,void *arg);
#endif

