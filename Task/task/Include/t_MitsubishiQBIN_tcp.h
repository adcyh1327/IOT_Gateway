#ifndef _T_MITSUBISHIQBIN_TCP_MASTER_H
#define _T_MITSUBISHIQBIN_TCP_MASTER_H	 

void MitsubishiQBINtcp_Init(unsigned char nodeID);
void TIM_1ms_MitsubishiQBIN_tcp(void);
void f_MitsubishiQBINtcp_task(void);
void MitsubishiQBIN_Write(unsigned char LeiXing, unsigned int  Adderadd, unsigned char l_u8Len);

#endif
