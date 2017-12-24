#include "stm32f10x.h"   
#include "m_spi_mb_tcp_slave.h"
//#include "t_mb_tcp_slave.h"
#include "m_w5500.h"  
#include "mes.h"

typedef  unsigned char SOCKET;

#define S_RX_SIZE  1024
#define S_TX_SIZE  1024
/*   SPI Initialization  */
void SPI1_Configuration(void)
{
	GPIO_InitTypeDef 		GPIO_InitStructure;
	SPI_InitTypeDef   		SPI_InitStructure;
//	EXTI_InitTypeDef        EXTI_InitStructure;	

	/*开启GPIOx的外设时钟*/
	/* Enable SPI1 and GPIOA clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1|RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	/* Configure SPI1 pins: SCK, MISO and MOSI -------------*/
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* 初始化CS引脚 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	/* Set SPI interface */
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial=7;
	SPI_Init(SPI1,&SPI_InitStructure);						
	SPI_Cmd(SPI1,ENABLE);					//Enable  SPI1
   /*   NSS PA4使能输出   */ 
	/* W5500_RST引脚初始化配置(PC0) */
}

void SetKeepAlive(SOCKET s,u16 reg,u8 dat)
{
  TCP_SLAVE_NSS_LOW;	     
  /* Write Address */
  SPI_I2S_SendData(SPI1,reg/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,reg);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); 
  /* Write Control Byte */
  SPI_I2S_SendData(SPI1,(RWB_WRITE|(s*0x20+0x08)));
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);   
  /* Write 1 byte */
  SPI_I2S_SendData(SPI1,dat);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); 
  /* Set W5500 SCS High */
  TCP_SLAVE_NSS_HIGH;
}

void SlaveSocket_Config(u8 socket,uint16_t usTCPPort,u8 mode)
{
  /* set Socket n Port Number */
	SlaveWrite_SOCK_2_Byte(socket, Sn_PORT, usTCPPort);

	SlaveWrite_SOCK_2_Byte(socket,RTR,2000);//重试时间200mS

	SlaveWrite_SOCK_2_Byte(socket,RCR,8);	//重试次数：3

	/* Set Maximum Segment Size as 1460 */
	SlaveWrite_SOCK_2_Byte(socket, Sn_MSSR, 1460);

	/* Set RX Buffer Size as 2K */
	SlaveWrite_SOCK_1_Byte(socket,Sn_RXBUF_SIZE, 0x01);
	/* Set TX Buffer Size as 2K */
	SlaveWrite_SOCK_1_Byte(socket,Sn_TXBUF_SIZE, 0x01);
	/* IP address conflict and Destination unreachable interrupt*/
    SlaveWrite_SOCK_1_Byte(socket,Sn_MR,mode);
    SlaveWrite_SOCK_1_Byte(socket,Sn_CR,OPEN);
	while(SlaveRead_SOCK_1_Byte(socket,Sn_CR));
	SetKeepAlive(socket,Sn_KPALVTR, 0x01);//在线检测,启动后 10/s 发送一次
}

void MB_TCP_Slave_W5500_Init(void)
{
	 TCP_SlaveW5500_Configuration();	  
}

void MB_TCP_Slave_Socket_Init(u8 socket,u16 port,u8 mode)
{
  SlaveSocket_Config(socket,port,mode);
 //SlaveSocket1_Config(g_u16TcpSlaveLclPortDwn);
 SlaveSocket_Listen(socket);	   
}

void TCP_SlaveW5500_Configuration(void)
{
  unsigned char array[6];
  GPIO_ResetBits(TCP_SLAVE_RST_PORT, TCP_SLAVE_RST_PIN); 
  Delay1ms(50);	
  GPIO_SetBits(TCP_SLAVE_RST_PORT, TCP_SLAVE_RST_PIN);
  Delay1ms(5);  /*delay 100ms 使用systick 1ms时基的延时*/
  while((SlaveRead_1_Byte(PHYCFGR)&LINK)==0);   	/* Waiting for Ethernet Link */		
  SlaveWrite_1_Byte(MR, RST);
  Delay1ms(101);  	/*delay 20ms */	 
  /* Set Gateway IP as: 192.168.1.1 */
  array[0]=g_u8TcpSlaveGw[0];
  array[1]=g_u8TcpSlaveGw[1];
  array[2]=g_u8TcpSlaveGw[2];
  array[3]=g_u8TcpSlaveGw[3];
  SlaveWrite_Bytes(GAR, array, 4);	  
  /* Set Subnet Mask as: 255.255.255.0 */
  array[0]=g_u8TcpSlaveSub[0];
  array[1]=g_u8TcpSlaveSub[1];
  array[2]=g_u8TcpSlaveSub[2];
  array[3]=g_u8TcpSlaveSub[3];
  SlaveWrite_Bytes(SUBR, array, 4);	 
  /* Set MAC Address as: 0x48,0x53,0x00,0x57,0x55,0x00 */
  array[0]=g_u8TcpSlaveLclMac[0];
  array[1]=g_u8TcpSlaveLclMac[1];
  array[2]=g_u8TcpSlaveLclMac[2];
  array[3]=g_u8TcpSlaveLclIP[1];
  array[4]=g_u8TcpSlaveLclIP[2];
  array[5]=g_u8TcpSlaveLclIP[3];//防止网络中出现mac地址相同
  SlaveWrite_Bytes(SHAR, array, 6);	
  /* Set W5500 IP as: 192.168.1.129 */
  array[0]=g_u8TcpSlaveLclIP[0];
  array[1]=g_u8TcpSlaveLclIP[1];
  array[2]=g_u8TcpSlaveLclIP[2];		  
  array[3]=g_u8TcpSlaveLclIP[3];
  SlaveWrite_Bytes(SIPR, array, 4);
} 
/******************************* W5500 Write Operation *******************************/
/* Write W5500 Common Register a byte */
void SlaveWrite_1_Byte(unsigned short reg, unsigned char dat)
{
  /* Set W5500 SCS Low */
  TCP_SLAVE_NSS_LOW;	  
  /* Write Address */
  SPI_I2S_SendData(SPI1,reg/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,reg);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);	 
  /* Write Control Byte */
  SPI_I2S_SendData(SPI1,(FDM1|RWB_WRITE|COMMON_R));
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);	 
  /* Write 1 byte */
  SPI_I2S_SendData(SPI1,dat);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);	 
  /* Set W5500 SCS High */
  TCP_SLAVE_NSS_HIGH;
}			   
/* Write W5500 Common Register 2 bytes */
void SlaveWrite_2_Byte(unsigned short reg, unsigned short dat)
{
  /* Set W5500 SCS Low */
  TCP_SLAVE_NSS_LOW;	     
  /* Write Address */
  SPI_I2S_SendData(SPI1,reg/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,reg);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  /* Write Control Byte */
  SPI_I2S_SendData(SPI1,(FDM2|RWB_WRITE|COMMON_R));
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); 
  /* Write 2 bytes */
  SPI_I2S_SendData(SPI1,dat/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,dat);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); 
  /* Set W5500 SCS High */
  TCP_SLAVE_NSS_HIGH;
}

/* Write W5500 Common Register n bytes */
void SlaveWrite_Bytes(unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
  unsigned short i;		  
  /* Set W5500 SCS Low */
  TCP_SLAVE_NSS_LOW;		  
  /* Write Address */
  SPI_I2S_SendData(SPI1,reg/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,reg);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);		 
  /* Write Control Byte */
  SPI_I2S_SendData(SPI1,(VDM|RWB_WRITE|COMMON_R));
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);  
  /* Write n bytes */
  for(i=0;i<size;i++)
  {
  	SPI_I2S_SendData(SPI1,*dat_ptr);
  	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);	  
  	dat_ptr++;
  }			 
  /* Set W5500 SCS High */
  TCP_SLAVE_NSS_HIGH;
}

/* Write W5500 Socket Register 1 byte */
void SlaveWrite_SOCK_1_Byte(SOCKET s, unsigned short reg, unsigned char dat)
{
  /* Set W5500 SCS Low */
  TCP_SLAVE_NSS_LOW;	     
  /* Write Address */
  SPI_I2S_SendData(SPI1,reg/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,reg);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); 
  /* Write Control Byte */
  SPI_I2S_SendData(SPI1,(FDM1|RWB_WRITE|(s*0x20+0x08)));
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);   
  /* Write 1 byte */
  SPI_I2S_SendData(SPI1,dat);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); 
  /* Set W5500 SCS High */
  TCP_SLAVE_NSS_HIGH;
}				   
/* Write W5500 Socket Register 2 byte */
void SlaveWrite_SOCK_2_Byte(SOCKET s, unsigned short reg, unsigned short dat)
{
  /* Set W5500 SCS Low */
  TCP_SLAVE_NSS_LOW;     
  /* Write Address */
  SPI_I2S_SendData(SPI1,reg/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,reg);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);  
  /* Write Control Byte */
  SPI_I2S_SendData(SPI1,(FDM2|RWB_WRITE|(s*0x20+0x08)));
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  /* Write 2 bytes */
  SPI_I2S_SendData(SPI1,dat/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,dat);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); 
  /* Set W5500 SCS High */
  TCP_SLAVE_NSS_HIGH;
}

/* Write W5500 Socket Register 4 byte */
void SlaveWrite_SOCK_4_Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
  /* Set W5500 SCS Low */
  TCP_SLAVE_NSS_LOW;	   
  /* Write Address */
  SPI_I2S_SendData(SPI1,reg/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,reg);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);   
  /* Write Control Byte */
  SPI_I2S_SendData(SPI1,(FDM4|RWB_WRITE|(s*0x20+0x08)));
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);   
  /* Write 4 bytes */
  SPI_I2S_SendData(SPI1,*dat_ptr);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); 
  dat_ptr++;
  SPI_I2S_SendData(SPI1,*dat_ptr);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);  
  dat_ptr++;
  SPI_I2S_SendData(SPI1,*dat_ptr);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);  
  dat_ptr++;
  SPI_I2S_SendData(SPI1,*dat_ptr);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);  
  /* Set W5500 SCS High */
  TCP_SLAVE_NSS_HIGH;
}					 
/******************************* W5500 Read Operation *******************************/
/* Read W5500 Common register 1 Byte */
unsigned char SlaveRead_1_Byte(unsigned short reg)
{
  unsigned char i;	 
  /* Set W5500 SCS Low */
  TCP_SLAVE_NSS_LOW;		  
  /* Write Address */
  SPI_I2S_SendData(SPI1,reg/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,reg);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); 
  /* Write Control Byte */
  SPI_I2S_SendData(SPI1,(FDM1|RWB_READ|COMMON_R));
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);  
  /* Write a dummy byte */
  i=SPI_I2S_ReceiveData(SPI1);
  SPI_I2S_SendData(SPI1,0x00);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); 
  /* Read 1 byte */
  i=SPI_I2S_ReceiveData(SPI1);		   
  /* Set W5500 SCS High*/
  TCP_SLAVE_NSS_HIGH;
  return i;
}		
/* Read W5500 Socket register 1 Byte */
unsigned char SlaveRead_SOCK_1_Byte(SOCKET s, unsigned short reg)
{
  unsigned char i;		 
  /* Set W5500 SCS Low */
  TCP_SLAVE_NSS_LOW;	
  /* Write Address */
  SPI_I2S_SendData(SPI1,reg/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,reg);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);	
  /* Write Control Byte */
  SPI_I2S_SendData(SPI1,(FDM1|RWB_READ|(s*0x20+0x08)));
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);  
  /* Write a dummy byte */
  i=SPI_I2S_ReceiveData(SPI1);
  SPI_I2S_SendData(SPI1,0x00);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);   
  /* Read 1 byte */
  i=SPI_I2S_ReceiveData(SPI1);	   
  /* Set W5500 SCS High*/
  TCP_SLAVE_NSS_HIGH;
  return i;
}

/* Read W5500 Socket register 2 Bytes (short) */
unsigned short SlaveRead_SOCK_2_Byte(SOCKET s, unsigned short reg)
{
  unsigned short i;		   
  /* Set W5500 SCS Low */
  TCP_SLAVE_NSS_LOW;	
  /* Write Address */
  SPI_I2S_SendData(SPI1,reg/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,reg);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);   
  /* Write Control Byte */
  SPI_I2S_SendData(SPI1,(FDM2|RWB_READ|(s*0x20+0x08)));
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);  
  /* Write a dummy byte */
  i=SPI_I2S_ReceiveData(SPI1);
  SPI_I2S_SendData(SPI1,0x00);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  i=SPI_I2S_ReceiveData(SPI1);				 
  SPI_I2S_SendData(SPI1,0x00);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  i*=256;
  i+=SPI_I2S_ReceiveData(SPI1);		
  /* Set W5500 SCS High*/
  TCP_SLAVE_NSS_HIGH;
  return i;
}

/******************** Read data from W5500 Socket data RX Buffer *******************/
unsigned short SlaveRead_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr)
{
  unsigned short rx_size;
  unsigned short offset, offset1;
  unsigned short i;
  unsigned char j;			
  rx_size=SlaveRead_SOCK_2_Byte(s,Sn_RX_RSR);
  if(rx_size==0)  	/* if no data received, return */
  	return 0;
  if(rx_size>1460)
  	rx_size=1460;	
  offset=SlaveRead_SOCK_2_Byte(s,Sn_RX_RD);
  offset1=offset;
  offset&=(S_RX_SIZE-1);  	/* Calculate the real physical address */	 
  /* Set W5500 SCS Low */
  TCP_SLAVE_NSS_LOW;	   
  /* Write Address */
  SPI_I2S_SendData(SPI1,offset/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,offset);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  /* Write Control Byte */
  SPI_I2S_SendData(SPI1,(VDM|RWB_READ|(s*0x20+0x18)));  	/* Read variable size */
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  j=SPI_I2S_ReceiveData(SPI1);		    
  if((offset+rx_size)<S_RX_SIZE)
  {
  	/* Read Data */
  	for(i=0;i<rx_size;i++)
  	{
  	  SPI_I2S_SendData(SPI1,0x00);  	/* Send a dummy byte */
  	  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  	  j=SPI_I2S_ReceiveData(SPI1);
  	  *dat_ptr=j;
  	  dat_ptr++;
  	}
  }
  else
  {
  	offset=S_RX_SIZE-offset;
  	for(i=0;i<offset;i++)
  	{
  	  SPI_I2S_SendData(SPI1,0x00);   	/* Send a dummy byte */
  	  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  	  j=SPI_I2S_ReceiveData(SPI1);
  	  *dat_ptr=j;
  	  dat_ptr++;
  	}
  	/* Set W5500 SCS High*/
  	TCP_SLAVE_NSS_HIGH;		
  	/* Set W5500 SCS Low */
  	TCP_SLAVE_NSS_LOW;
  	/* Write Address */
  	SPI_I2S_SendData(SPI1,0x00);
  	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  	SPI_I2S_SendData(SPI1,0x00);
  	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  	/* Write Control Byte */
  	SPI_I2S_SendData(SPI1,(VDM|RWB_READ|(s*0x20+0x18)));  	/* Read variable size */
  	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  	j=SPI_I2S_ReceiveData(SPI1);
  	/* Read Data */
  	for(;i<rx_size;i++)
  	{
  	  SPI_I2S_SendData(SPI1,0x00);  	/* Send a dummy byte */
  	  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  	  j=SPI_I2S_ReceiveData(SPI1);
  	  *dat_ptr=j;
  	  dat_ptr++;
  	}
  }
  /* Set W5500 SCS High*/
  TCP_SLAVE_NSS_HIGH;	 
  /* Update offset*/
  offset1+=rx_size;
  SlaveWrite_SOCK_2_Byte(s, Sn_RX_RD, offset1);
  SlaveWrite_SOCK_1_Byte(s, Sn_CR, RECV);  	  	  /* Write RECV Command */
  return rx_size;
}

/******************** Write data to W5500 Socket data TX Buffer *******************/
void SlaveWrite_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size)
{
  unsigned short offset,offset1;
  unsigned short i;	 
  offset=SlaveRead_SOCK_2_Byte(s,Sn_TX_WR);
  offset1=offset;
  offset&=(S_TX_SIZE-1);  	/* Calculate the real physical address */	   
  /* Set W5500 SCS Low */
  TCP_SLAVE_NSS_LOW;
  /* Write Address */
  SPI_I2S_SendData(SPI1,offset/256);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1,offset);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);	 
  /* Write Control Byte */
  SPI_I2S_SendData(SPI1,(VDM|RWB_WRITE|(s*0x20+0x10)));  	/* Read variable size */
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);		 
  if((offset+size)<S_TX_SIZE)
  {
  	/* Write Data */
  	for(i=0;i<size;i++)
  	{
  	  SPI_I2S_SendData(SPI1,*dat_ptr);  	/* Send a byte*/
  	  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);   	
  	  dat_ptr++;
  	}
  }
  else
  {
  	offset=S_TX_SIZE-offset;
  	for(i=0;i<offset;i++)
  	{
  	  SPI_I2S_SendData(SPI1,*dat_ptr);   	/* Send a byte */
  	  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);   
  	  dat_ptr++;
  	}
  	/* Set W5500 SCS High*/
  	TCP_SLAVE_NSS_HIGH;

  	/* Set W5500 SCS Low */
  	TCP_SLAVE_NSS_LOW;
  	/* Write Address */
  	SPI_I2S_SendData(SPI1,0x00);
  	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  	SPI_I2S_SendData(SPI1,0x00);
  	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  	/* Write Control Byte */
  	SPI_I2S_SendData(SPI1,(VDM|RWB_WRITE|(s*0x20+0x10)));  	/* Read variable size */
  	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  	/* Read Data */
  	for(;i<size;i++)
  	{
  	  SPI_I2S_SendData(SPI1,*dat_ptr);  	/* Send a byte */
  	  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  	
  	  dat_ptr++;
  	}
  }
  /* Set W5500 SCS High*/
  TCP_SLAVE_NSS_HIGH;		
  /* Updata offset */
  offset1+=size;
  SlaveWrite_SOCK_2_Byte(s, Sn_TX_WR, offset1);
  SlaveWrite_SOCK_1_Byte(s, Sn_CR, SEND);  	  	  /* Write SEND Command */
}

/*********************** Set Socket n in TCP Client mode ************************/
unsigned int SlaveSocket_Connect(SOCKET s)
{
  /* Set Socket n in TCP mode */
  SlaveWrite_SOCK_1_Byte(s,Sn_MR,MR_TCP);
  /* Open Socket n */
  SlaveWrite_SOCK_1_Byte(s,Sn_CR,OPEN);	  
  while(SlaveRead_SOCK_1_Byte(s,Sn_CR));  /* Wait for a moment */
  if(SlaveRead_SOCK_1_Byte(s,Sn_SR)!=SOCK_INIT)
  {
  	SlaveWrite_SOCK_1_Byte(s,Sn_CR,CLOSE);  	/* Close Socket n */
  	return FALSE;
  }			  
  /* Set Socket n in Server mode */
  SlaveWrite_SOCK_1_Byte(s,Sn_CR,CONNECT);
  return TRUE;
}

/*********************** Set Socket n in TCP Server mode ************************/
u16 SlaveSocket_Listen(SOCKET s)
{
	/* Open Socket n */
//	SlaveWrite_SOCK_1_Byte(s,Sn_CR,OPEN); 
//	while(SlaveRead_SOCK_1_Byte(s,Sn_CR));
	if(SlaveRead_SOCK_1_Byte(s,Sn_SR)==SOCK_INIT)
	{
	   SlaveWrite_SOCK_1_Byte(s,Sn_CR,LISTEN);
	   while(SlaveRead_SOCK_1_Byte(s,Sn_CR));
	   return TRUE;
	}
	else 
	{
		return FALSE;
	}
}




