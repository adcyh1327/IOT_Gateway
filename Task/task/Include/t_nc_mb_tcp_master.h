#ifndef _T_M70V_NC_H
#define _T_M70V_NC_H	 


extern unsigned char  g_u8NcConMech; 
extern unsigned char  g_u8NcParaRead[20];  
extern unsigned char  g_u8NcParaReadValid;

void M70V_NC_TCP_Init(u8 nodeID);
void TIM_1ms_M70V_NC(void);
void f_M70V_NCTCP_task(void);
void M70V_NC_SocketConnect(u8 socket);
//void SocketConnectHook(void);
#endif
