#ifndef	SMART200_TCP_H

#define SMART200_TCP_H

void Smart200TCP_Init(u8 nodeID);
void TIM_1ms_Smart200TCP();
//��������TCP����task
void f_Smart200TCP_Task(void);



#endif
