//#ifndef FX2N485BD_H
//#define FX2N485BD_H

#include "stm32f10x.h" 



void Fx2n485bd_Init(u8 nodeID);
void TIM_1ms_Fx2n485bd(void);
void UART_Fx2n485bd_Recv(u8 data);
void f_Fx2n485bd_task(void);

//#endif
