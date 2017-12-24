#ifndef _T_MB_TCP_MASTER_H
#define _T_MB_TCP_MASTER_H	 



void ModbusTCp_Init(u8 nodeID);
void TIM_1ms_MBTCPMaster(void);
void f_mb_tcp_master_task(void);

#endif
