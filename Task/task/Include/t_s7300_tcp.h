#ifndef	S7300_TCP

#define S7300_TCP

void S7300TCP_Init(u8 nodeID);
void TIM_1ms_S7300TCP();
//��������TCP����task
void f_S7300TCP_Task(void);



#endif
