#ifndef	S7400_TCP

#define S7400_TCP

void S7400TCP_Init(unsigned char nodeID);
void TIM_1ms_S7400TCP();
//接收数据TCP的子task
void f_S7400TCP_Task(void);



#endif
