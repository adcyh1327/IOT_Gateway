#ifndef _T_MITSUBISHIQ_TCP_MASTER_H
#define _T_MITSUBISHIQ_TCP_MASTER_H	 



void MitsubishiQtcp_Init(u8 nodeID);
void TIM_1ms_MitsubishiQ_tcp(void);
void f_MitsubishiQtcp_task(void);
void MitsubishiQ_Write(unsigned char LeiXing, unsigned int  Adderadd, unsigned char l_u8Len);
void TIM_1ms_MitsubishiQtcp(void);
#endif
