#ifndef __TASK_MODBUSTCP_H
#define __TASK_MODBUSTCP_H

#include "main.h"
#include "ucos_ii.h"

void MBTCP_Timer1ms(void);
void Task_ModbusTCP(void *p_arg);
void MBTCP_DataHandler(u8 chnidx,struct USARTCAN_Recv_t *recv,void *arg);
#endif

