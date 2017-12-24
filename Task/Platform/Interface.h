#ifndef TASK_H
#define TASK_H

#define CHN_UART_CFG         0x00


void DAQ_EthSend(u8 socket,u8 *sendbufer,u8 lenth);
void DAQ_UartSend(u8 *sendbuf, u8 lenth,u8 specchn);
void SocketConnectHook(u8 socket);
void DAQ_LIB_Init(void);
void DAQ_Timer(void);
void DAQ_Mainfunction(void);
#endif
