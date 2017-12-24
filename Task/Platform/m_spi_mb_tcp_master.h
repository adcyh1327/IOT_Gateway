#ifndef _M_SPI_MB_TCP_MASTER_H
#define	_M_SPI_MB_TCP_MASTER_H
#include "stm32f10x.h"
#include "stm32f10x_spi.h"

#define S_RX_SIZE	1024
#define S_TX_SIZE	1024
typedef  unsigned char SOCKET;

/* Reset W5500 复位*/
#define TCP_MASTER_RST_PORT				GPIOD
#define TCP_MASTER_RST_PIN 				GPIO_Pin_3
/* Reset W5500 中断*/
#define TCP_MASTER_INT_PORT				GPIOA
#define TCP_MASTER_INT_PIN				GPIO_Pin_1
/* W5500 SPI1 Chip select output 片选*/
#define TCP_MASTER_NSS_PORT				GPIOB
#define TCP_MASTER_NSS_PIN				GPIO_Pin_12
#define TCP_MASTER_NSS_LOW				GPIO_ResetBits(TCP_MASTER_NSS_PORT, TCP_MASTER_NSS_PIN)
#define TCP_MASTER_NSS_HIGH				GPIO_SetBits(TCP_MASTER_NSS_PORT, TCP_MASTER_NSS_PIN)												
//unsigned int S1_SendOK=1;
//unsigned int S1_TimeOut=0;

//unsigned short S0_Port=8000;
//unsigned short S1_Port=9000;

//unsigned char S_Data_Buffer[2048];
//unsigned char S_Data_Buffer1[2048];
/***************----- 端口收发数据的状态 -----***************/
extern unsigned char mb_tcp_master_Socket_Data[8];		//端口0接收和发送数据的状态,1:端口接收到数据,2:端口发送数据完成 
extern unsigned char mb_tcp_master_Socket_State[8];
#define S_INIT		0x01	//端口完成初始化 
#define S_CONN		0x02	//端口完成连接,可以正常传输数据 
#define S_RECEIVE		0x01		//端口接收到一个数据包 
#define S_TRANSMITOK	0x02		//端口发送一个数据包完成 
/***************----- 端口数据缓冲区 -----***************/
extern unsigned char mb_tcp_master_Rx_Buffer[S_RX_SIZE];	//端口接收数据缓冲区 
extern unsigned char mb_tcp_master_Tx_Buffer[S_TX_SIZE];	//端口发送数据缓冲区 
extern void System_Initialization(void);
extern void SPI2_Configuration(void);
extern void TCP_MasterW5500_Configuration(void);

/* Write W5500 Common Register a byte */
extern void MasterWrite_1_Byte(unsigned short reg, unsigned char dat);
/* Write W5500 Common Register 2 bytes */
extern void MasterWrite_2_Byte(unsigned short reg, unsigned short dat);
/* Write W5500 Common Register n bytes */
extern void MasterWrite_Bytes(unsigned short reg, unsigned char *dat_ptr, unsigned short size);

/* Write W5500 Socket Register 1 byte */
extern void MasterWrite_SOCK_1_Byte(SOCKET s, unsigned short reg, unsigned char dat);
/* Write W5500 Socket Register 2 byte */
extern void MasterWrite_SOCK_2_Byte(SOCKET s, unsigned short reg, unsigned short dat);
/* Write W5500 Socket Register 2 byte */
extern void MasterWrite_SOCK_4_Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr);

/* Read W5500 Common register 1 Byte */
extern unsigned char MasterRead_1_Byte(unsigned short reg);
/* Read W5500 Socket register 1 Byte */
extern unsigned char MasterRead_SOCK_1_Byte(SOCKET s, unsigned short reg);
/* Read W5500 Socket register 2 Bytes (short) */
extern unsigned short MasterRead_SOCK_2_Byte(SOCKET s, unsigned short reg);

/* Read data from W5500 Socket data RX Buffer */
extern unsigned short MasterRead_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr);
/* Write data to W5500 Socket data TX Buffer */
extern void MasterWrite_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size);

/* Set Socket n in TCP Client mode */
extern unsigned int MasterSocket_Connect(SOCKET s);

extern unsigned int MasterSocket_Listen(SOCKET s);

extern void MasterSocket_Config(u8 s,unsigned short usTCPPort);

extern void modbus_appcall(void);


extern void modbus_sendpoll(void);

extern void Process_IR(void);

extern void Process_LoopBack(void);

extern unsigned char check_new(void);

extern void mb_tcp_master_W5500_Initialization(void);

extern unsigned int mb_tcp_master_Socket_Connect(SOCKET s);
extern void mb_tcp_master_W5500_Socket_Set(u8 s);
extern void mb_tcp_master_W5500_Interrupt_Process(u8 s);


#endif

