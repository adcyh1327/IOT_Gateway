#ifndef _M_SPI_MB_TCP_SLAVE_H
#define	_M_SPI_MB_TCP_SLAVE_H
#include "stm32f10x.h"
#include "stm32f10x_spi.h"

#define TCP_SLAVE_RST_PORT				GPIOA
#define TCP_SLAVE_RST_PIN 				GPIO_Pin_3
/* Reset W5500 ÖÐ¶Ï*/
#define TCP_SLAVE_INT_PORT				GPIOA
#define TCP_SLAVE_INT_PIN				GPIO_Pin_0
/* W5500 SPI1 Chip select output Æ¬Ñ¡*/
#define TCP_SLAVE_NSS_PORT				GPIOA
#define TCP_SLAVE_NSS_PIN				GPIO_Pin_4
#define TCP_SLAVE_NSS_LOW				GPIO_ResetBits(TCP_SLAVE_NSS_PORT, TCP_SLAVE_NSS_PIN)
#define TCP_SLAVE_NSS_HIGH				GPIO_SetBits(TCP_SLAVE_NSS_PORT, TCP_SLAVE_NSS_PIN)												

typedef  unsigned char SOCKET;


extern void SPI1_Configuration(void);
extern void TCP_SlaveW5500_Configuration(void);
extern void mb_tcp_slave_W5500_Initialization(void);
/* Write W5500 Common Register a byte */
extern void SlaveWrite_1_Byte(unsigned short reg, unsigned char dat);
/* Write W5500 Common Register 2 bytes */
extern void SlaveWrite_2_Byte(unsigned short reg, unsigned short dat);
/* Write W5500 Common Register n bytes */
extern void SlaveWrite_Bytes(unsigned short reg, unsigned char *dat_ptr, unsigned short size);

/* Write W5500 Socket Register 1 byte */
extern void SlaveWrite_SOCK_1_Byte(SOCKET s, unsigned short reg, unsigned char dat);
/* Write W5500 Socket Register 2 byte */
extern void SlaveWrite_SOCK_2_Byte(SOCKET s, unsigned short reg, unsigned short dat);
/* Write W5500 Socket Register 2 byte */
extern void SlaveWrite_SOCK_4_Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr);

/* Read W5500 Common register 1 Byte */
extern unsigned char SlaveRead_1_Byte(unsigned short reg);
/* Read W5500 Socket register 1 Byte */
extern unsigned char SlaveRead_SOCK_1_Byte(SOCKET s, unsigned short reg);
/* Read W5500 Socket register 2 Bytes (short) */
extern unsigned short SlaveRead_SOCK_2_Byte(SOCKET s, unsigned short reg);

/* Read data from W5500 Socket data RX Buffer */
extern unsigned short SlaveRead_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr);
/* Write data to W5500 Socket data TX Buffer */
extern void SlaveWrite_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size);

void MB_TCP_Slave_Socket_Init(u8 socket,u16 port,u8 mode);
/* Set Socket n in TCP Client mode */
extern unsigned int SlaveSocket_Connect(SOCKET s);

extern u16 SlaveSocket_Listen(SOCKET s);

extern void SlaveSocket0_Config(unsigned short usTCPPort);

extern void modbus_appcall(void);

extern void modbus_sendpoll(void);

extern void Process_IR(void);

extern void Process_LoopBack(void);

extern unsigned char check_new(void);

#endif

