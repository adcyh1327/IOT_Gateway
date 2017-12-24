#ifndef _T_MITSUBISHIQ_SERIAL_MASTER_H
#define _T_MITSUBISHIQ_SERIAL_MASTER_H	 

//#include "stm32f10x.h" 

void MitsubisihQserial_Init(unsigned char nodeID);
void TIM_1ms_MitsubishiQserial(void);
void UART_MitsubishiQserial_Recv(unsigned char data);
void f_MitsubishiQserial_task(void);

#endif
