#ifndef	S1200_TCP

#define S1200_TCP

void S1200TCP_Init(u8 nodeID);
void TIM_1ms_S1200TCP();
//��������TCP����task
void f_S1200TCP_Task(void);



#endif
