#ifndef _T_FX3UENETADP2_TCP_MASTER_H
#define _T_FX3UENETADP2_TCP_MASTER_H	 

#define FX3UENETADP2_SEND_LENTH             300          //发送缓冲区大小
#define FX3UENETADP2_RECE_LENGTH             300         //接受缓冲区大小

void Fx3uEnet_adp2_Init(u8 nodeID);
void TIM_1ms_Fx3uEnet_adp2_tcp(void);
void f_Fx3uEnet_adp2_task(void);
void TIM_1ms_Fx3uEnet_adp2(void);
#endif
