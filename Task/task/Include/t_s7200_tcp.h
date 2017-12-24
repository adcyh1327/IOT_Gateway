#ifndef	S7200_TCP

#define S7200_TCP

void S7200TCP_Init(u8 nodeID);
void TIM_1ms_S7200TCP();
//接收数据TCP的子task
void f_S7200TCP_Task(void);



#endif
