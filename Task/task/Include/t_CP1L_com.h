#include "stm32f10x.h" 



void CP1LCom_Init(u8 nodeID);
void UART_CPCom_Recv(u8 data);
void TIM_1ms_CP1LCom(void);
void f_CPcom_task(void);
