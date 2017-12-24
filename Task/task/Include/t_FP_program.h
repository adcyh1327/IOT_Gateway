#include "stm32f10x.h" 


void FPprogram_Init(u8 nodeID);
void UART_FPProgram_Recv(u8 data);
void TIM_1ms_FPProgram(void);
void f_FPprogram_task(void);
