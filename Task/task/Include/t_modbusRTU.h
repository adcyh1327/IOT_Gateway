#ifndef MODBUSRTU_H
#define MODBUSRTU_H			  

extern u8 Write_FuncCode;

void ModbusRTU_Init(u8 nodeID);
void ModbusRTU_Read(u8 func_code,u8 lenth);
void ModbusRTU_Write(u8 func_code,u16 addr,u8 lenth);
void UART_ModbusRTU_Recv(unsigned char  l_u8ReceData);
void TIM_1ms_ModbusRTU(void);
void f_ModbusRTU_task(void);

#endif
