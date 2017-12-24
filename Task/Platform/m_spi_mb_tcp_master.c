#include "main.h"
#include "stm32f10x.h"   
#include "stm32f10x_spi.h" 
#include "m_w5500.h"              
#include "m_spi_mb_tcp_master.h"		     
#include "t_mb_tcp_master.h"		     
#include "mes.h"	 


/***************----- 网络参数变量定义 -----***************/
unsigned char mb_tcp_master_Gateway_IP[4];//网关IP地址 
unsigned char mb_tcp_master_Sub_Mask[4];	//子网掩码 
unsigned char mb_tcp_master_Phy_Addr[6];	//物理地址(MAC) 
unsigned char mb_tcp_master_IP_Addr[4];	//本机IP地址 

unsigned char mb_tcp_master_Socket_Port[8][2];	//端口0的端口号(5000) 
unsigned char mb_tcp_master_Socket_DIP[8][4];	//端口0目的IP地址 
unsigned char mb_tcp_master_Socket_DPort[8][2];	//端口0目的端口号(6000) 

unsigned char mb_tcp_master_UDP_DIPR[4];	//UDP(广播)模式,目的主机IP地址
unsigned char mb_tcp_master_UDP_DPORT[2];	//UDP(广播)模式,目的主机端口号

/***************----- 端口的运行模式 -----***************/
unsigned char mb_tcp_master_Socket_Mode[8] ={3,3,3,3,3,3,3,3};	//端口0的运行模式,0:TCP服务器模式,1:TCP客户端模式,2:UDP(广播)模式
#define TCP_SERVER	0x00	//TCP服务器模式
#define TCP_CLIENT	0x01	//TCP客户端模式 
#define UDP_MODE	0x02	//UDP(广播)模式 

/***************----- 端口的运行状态 -----***************/
unsigned char mb_tcp_master_Socket_State[8] ={0,0,0,0,0,0,0,0};	//端口0状态记录,1:端口完成初始化,2端口完成连接(可以正常传输数据) 


/***************----- 端口收发数据的状态 -----***************/
unsigned char mb_tcp_master_Socket_Data[8];		//端口0接收和发送数据的状态,1:端口接收到数据,2:端口发送数据完成 
#define S_RECEIVE	 0x01	//端口接收到一个数据包 
#define S_TRANSMITOK 0x02	//端口发送一个数据包完成 

/***************----- 端口数据缓冲区 -----***************/
unsigned char mb_tcp_master_Rx_Buffer[S_RX_SIZE];	//端口接收数据缓冲区 
unsigned char mb_tcp_master_Tx_Buffer[S_TX_SIZE];	//端口发送数据缓冲区 

unsigned char mb_tcp_master_W5500_Interrupt;	//W5500中断标志(0:无中断,1:有中断)

extern void  Delay1ms(uint32_t nTime);
void mb_tcp_master_Load_Net_Parameters(void);
void mb_tcp_master_W5500_Init(void);
unsigned char mb_tcp_master_Detect_Gateway(void);
unsigned char mb_tcp_master_Socket_Init(SOCKET s);
unsigned int mb_tcp_master_Socket_Connect(SOCKET s);
void m_spi_mb_tcp_master_init(void)
{   
	u8 i;
	for(i=0;i<8;i++)
	{
		mb_tcp_master_Socket_Mode[i] =3;
		mb_tcp_master_Socket_State[i] =0;
	}
}
		 /*******************************************************************************
* 函数名  : W5500_Initialization
* 描述    : W5500初始货配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void mb_tcp_master_W5500_Init(void)
{
  mb_tcp_master_Load_Net_Parameters();
	mb_tcp_master_W5500_Initialization();		//初始化W5500寄存器函数
	mb_tcp_master_Detect_Gateway();	//检查网关服务器 
	//mb_tcp_master_Socket_Init(0);		//指定Socket(0~7)初始化
}
/*******************************************************************************
* 函数名  : W5500_Init
* 描述    : 初始化W5500寄存器函数
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 在使用W5500之前，先对W5500初始化
*******************************************************************************/
void mb_tcp_master_W5500_Initialization(void)
{
	u8 i=0;

	MasterWrite_1_Byte(MR, RST);//软件复位W5500,置1有效,复位后自动清0
	Delay1ms(10);//延时10ms,自己定义该函数

	//设置网关(Gateway)的IP地址,Gateway_IP为4字节unsigned char数组,自己定义 
	//使用网关可以使通信突破子网的局限，通过网关可以访问到其它子网或进入Internet
	MasterWrite_Bytes(GAR, mb_tcp_master_Gateway_IP, 4);
			
	//设置子网掩码(MASK)值,SUB_MASK为4字节unsigned char数组,自己定义
	//子网掩码用于子网运算
	MasterWrite_Bytes(SUBR,mb_tcp_master_Sub_Mask,4);		
	
	//设置物理地址,PHY_ADDR为6字节unsigned char数组,自己定义,用于唯一标识网络设备的物理地址值
	//该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
	//如果自己定义物理地址，注意第一个字节必须为偶数
	MasterWrite_Bytes(SHAR,mb_tcp_master_Phy_Addr,6);		

	//设置本机的IP地址,IP_ADDR为4字节unsigned char数组,自己定义
	//注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关
	MasterWrite_Bytes(SIPR,mb_tcp_master_IP_Addr,4);		
	
	//设置发送缓冲区和接收缓冲区的大小，参考W5500数据手册
	for(i=0;i<8;i++)
	{
		MasterWrite_SOCK_1_Byte(i,Sn_RXBUF_SIZE, 0x01);//Socket Rx memory size=2k
		MasterWrite_SOCK_1_Byte(i,Sn_TXBUF_SIZE, 0x01);//Socket Tx mempry size=2k
	}

	//设置重试时间，默认为2000(200ms) 
	//每一单位数值为100微秒,初始化时值设为2000(0x07D0),等于200毫秒
	MasterWrite_2_Byte(RTR, 0x07d0);

	//设置重试次数，默认为8次 
	//如果重发的次数超过设定值,则产生超时中断(相关的端口中断寄存器中的Sn_IR 超时位(TIMEOUT)置“1”)
	MasterWrite_1_Byte(RCR,8);
}

/*******************************************************************************
* 函数名  : Detect_Gateway
* 描述    : 检查网关服务器
* 输入    : 无
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 无
*******************************************************************************/
unsigned char mb_tcp_master_Detect_Gateway(void)
{
	unsigned char ip_adde[4];
	ip_adde[0]=mb_tcp_master_IP_Addr[0]+1;
	ip_adde[1]=mb_tcp_master_IP_Addr[1]+1;
	ip_adde[2]=mb_tcp_master_IP_Addr[2]+1;
	ip_adde[3]=mb_tcp_master_IP_Addr[3]+1;

	//检查网关及获取网关的物理地址
	MasterWrite_SOCK_4_Byte(0,Sn_DIPR,ip_adde);//向目的地址寄存器写入与本机IP不同的IP值
	MasterWrite_SOCK_1_Byte(0,Sn_MR,MR_TCP);//设置socket为TCP模式
	MasterWrite_SOCK_1_Byte(0,Sn_CR,OPEN);//打开Socket	
	Delay1ms(5);//延时5ms 	
	
}

/*******************************************************************************
* 函数名  : Socket_Init
* 描述    : 指定Socket(0~7)初始化
* 输入    : s:待初始化的端口
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
unsigned char mb_tcp_master_Socket_Init(SOCKET s)
{
  u8 ret;
  ret=0xff;
  if(MasterRead_SOCK_1_Byte(s,Sn_SR) != SOCK_INIT)//如果socket打开失败
	{
		MasterWrite_SOCK_1_Byte(s,Sn_CR,CLOSE);//打开不成功,关闭Socket
		return FALSE;//返回FALSE(0x00)
	}

	MasterWrite_SOCK_1_Byte(s,Sn_CR,CONNECT);//设置Socket为Connect模式						

	do
	{
		u8 j=0;
		j=MasterRead_SOCK_1_Byte(s,Sn_IR);//读取Socket0中断标志寄存器
		if(j!=0)
		MasterWrite_SOCK_1_Byte(s,Sn_IR,j);
		Delay1ms(4);//延时5ms 
		if((j&IR_TIMEOUT) == IR_TIMEOUT)
		{
			MasterWrite_SOCK_2_Byte(s, Sn_MSSR, 1460);//设置分片长度，参考W5500数据手册，该值可以不修改	
    	//设置端口0的端口号
    	MasterWrite_SOCK_2_Byte(s, Sn_PORT, mb_tcp_master_Socket_Port[s][0]*256+mb_tcp_master_Socket_Port[s][1]);
    	//设置端口0目的(远程)端口号
    	MasterWrite_SOCK_2_Byte(s, Sn_DPORTR, mb_tcp_master_Socket_DPort[s][0]*256+mb_tcp_master_Socket_DPort[s][1]);
    	//设置端口0目的(远程)IP地址
    	MasterWrite_SOCK_4_Byte(s, Sn_DIPR, mb_tcp_master_Socket_DIP[s]);	
    	return FALSE;
		}
		else if(MasterRead_SOCK_1_Byte(s,Sn_DHAR) != 0xff)
		{
			MasterWrite_SOCK_1_Byte(s,Sn_CR,CLOSE);//关闭Socket
			return TRUE;							
		}
		else{

		}
	}while((ret!=TRUE)&&(ret!=FALSE));
}
/*******************************************************************************
* 函数名  : Load_Net_Parameters
* 描述    : 装载网络参数
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 网关、掩码、物理地址、本机IP地址、端口号、目的IP地址、目的端口号、端口工作模式
*******************************************************************************/
void mb_tcp_master_Load_Net_Parameters(void)
{
	u8 i;
	GPIO_SetBits(TCP_MASTER_RST_PORT, TCP_MASTER_RST_PIN);
	Delay1ms(4);
	mb_tcp_master_Gateway_IP[0] = g_u8TcpMasterGw[0];//加载网关参数
	mb_tcp_master_Gateway_IP[1] = g_u8TcpMasterGw[1];
	mb_tcp_master_Gateway_IP[2] = g_u8TcpMasterGw[2];
	mb_tcp_master_Gateway_IP[3] = g_u8TcpMasterGw[3];

	mb_tcp_master_Sub_Mask[0]=g_u8TcpMasterSub[0];//加载子网掩码
	mb_tcp_master_Sub_Mask[1]=g_u8TcpMasterSub[1];
	mb_tcp_master_Sub_Mask[2]=g_u8TcpMasterSub[2];
	mb_tcp_master_Sub_Mask[3]=g_u8TcpMasterSub[3];

	mb_tcp_master_Phy_Addr[0]=0x0c;//加载物理地址
	mb_tcp_master_Phy_Addr[1]=0x29;
	mb_tcp_master_Phy_Addr[2]=0xab;
	mb_tcp_master_Phy_Addr[3]=g_u8TcpMasterLclIP[1];
	mb_tcp_master_Phy_Addr[4]=g_u8TcpMasterLclIP[2];
	mb_tcp_master_Phy_Addr[5]=g_u8TcpMasterLclIP[3];//防止MAC地址冲突

	mb_tcp_master_IP_Addr[0]=g_u8TcpMasterLclIP[0];//加载本机IP地址
	mb_tcp_master_IP_Addr[1]=g_u8TcpMasterLclIP[1];
	mb_tcp_master_IP_Addr[2]=g_u8TcpMasterLclIP[2];
	mb_tcp_master_IP_Addr[3]=g_u8TcpMasterLclIP[3];

	for(i=0;i<8;i++)
	{
		mb_tcp_master_Socket_Port[i][0]=g_u16TcpMasterLclPort[i]/256;//加载端口0的端口号5000 
		mb_tcp_master_Socket_Port[i][1]=g_u16TcpMasterLclPort[i]%256;

		mb_tcp_master_Socket_DIP[i][0]=g_u8TcpMasterRmtIP[0];//加载端口0的目的IP地址
		mb_tcp_master_Socket_DIP[i][1]=g_u8TcpMasterRmtIP[1];
		mb_tcp_master_Socket_DIP[i][2]=g_u8TcpMasterRmtIP[2];
		mb_tcp_master_Socket_DIP[i][3]=g_u8TcpMasterRmtIP[3];
	
		mb_tcp_master_Socket_DPort[i][0] = g_u16TcpMasterRmtPort[i] / 256;//加载端口0的目的端口号6000
		mb_tcp_master_Socket_DPort[i][1] = g_u16TcpMasterRmtPort[i] % 256;

		mb_tcp_master_Socket_Mode[i]=TCP_CLIENT;//加载端口0的工作模式,TCP客户端模式
	}
}


/*******************************************************************************
* 函数名  : W5500_Socket_Set
* 描述    : W5500端口初始化配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 分别设置4个端口,根据端口工作模式,将端口置于TCP服务器、TCP客户端或UDP模式.
*			从端口状态字节Socket_State可以判断端口的工作情况
*******************************************************************************/
void mb_tcp_master_W5500_Socket_Set(u8 s)
{
	if((mb_tcp_master_Socket_State[s]&2)!=2)//端口0初始化配置 
	{
		if(mb_tcp_master_Socket_Connect(s)==TRUE)
		{	   
			mb_tcp_master_Socket_State[s]=S_INIT;
		}
		else
		{
			mb_tcp_master_Socket_State[s]=0;
		}
	  Delay1ms(20);
	}
}

/*   SPI Initialization  */
void SPI2_Configuration(void)
{
	GPIO_InitTypeDef 		GPIO_InitStructure;
	SPI_InitTypeDef   		SPI_InitStructure;

	/*开启GPIOx的外设时钟*/
	/* Enable SPI2 and GPIOA clocks */
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);	
	/* 初始化SCK、MISO、MOSI引脚 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/* 初始化CS引脚 */
	GPIO_InitStructure.GPIO_Pin = TCP_MASTER_NSS_PIN;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(TCP_MASTER_NSS_PORT, &GPIO_InitStructure);
	GPIO_SetBits(TCP_MASTER_NSS_PORT, TCP_MASTER_NSS_PIN);

	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;	//SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;							//设置为主SPI
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;						//SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;							//时钟悬空低
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;							//数据捕获于第1个时钟沿
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;								//NSS由外部管脚管理
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;	//波特率预分频值为2
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;					//数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial=7;								//CRC多项式为7
	SPI_Init(SPI2,&SPI_InitStructure);									//根据SPI_InitStruct中指定的参数初始化外设SPI2寄存器
	SPI_Cmd(SPI2,ENABLE);	//STM32使能SPI2
   /*   NSS PB12使能输出   */ 
	/* W5500_RST引脚初始化配置(PC0) */
/*	GPIO_InitStructure.GPIO_Pin  = TCP_MASTER_RST_PIN;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(TCP_MASTER_RST_PORT, &GPIO_InitStructure);*/

}
//void TCP_MasterW5500_Configuration(void)
//{
//  unsigned char array[6];	
//  GPIO_SetBits(TCP_MASTER_RST_PORT, TCP_MASTER_RST_PIN);
//  Delay1ms(4);  /*delay 100ms 使用systick 1ms时基的延时*/
//  while((MasterRead_1_Byte(PHYCFGR)&LINK)==0);   	/* Waiting for Ethernet Link */		
//  MasterWrite_1_Byte(MR, RST);
//  Delay1ms(4);  	/*delay 20ms */	 
//  /* Set Gateway IP as: 192.168.1.1 */
//  array[0]=192;
//  array[1]=168;
//  array[2]=0;
//  array[3]=1;
//  MasterWrite_Bytes(GAR, array, 4);	  
//  /* Set Subnet Mask as: 255.255.255.0 */
//  array[0]=255;
//  array[1]=255;
//  array[2]=255;
//  array[3]=0;
//  MasterWrite_Bytes(SUBR, array, 4);	 
//  /* Set MAC Address as: 0x48,0x53,0x00,0x57,0x55,0x00 */
//  array[0]=0x48;
//  array[1]=0x53;
//  array[2]=0x00;
//  array[3]=0x57;
//  array[4]=0x55;
//  array[5]=0x01;
//  MasterWrite_Bytes(SHAR, array, 6);	
//  /* Set W5500 IP as: 192.168.1.129 */
//  array[0]=192;
//  array[1]=168;
//  array[2]=0;
//  array[3]=251;
//  MasterWrite_Bytes(SIPR, array, 4);
//  //设置发送缓冲区和接收缓冲区的大小，参考W5500数据手册
//  //for(i=0;i<8;i++)
//  //{
//  //  Write_W5500_SOCK_1Byte(i,Sn_RXBUF_SIZE, 0x02);//Socket Rx memory size=2k
//  //  Write_W5500_SOCK_1Byte(i,Sn_TXBUF_SIZE, 0x02);//Socket Tx mempry size=2k
//  //}				
//  //设置重试时间，默认为2000(200ms) 
//  //每一单位数值为100微秒,初始化时值设为2000(0x07D0),等于200毫秒
//  //Write_W5500_2Byte(RTR, 0x07d0);
//  /* Set W5500 dest IP as 192.168.0.101 */
//  array[0]=192;
//  array[1]=168;
//  array[2]=0;
//  array[3]=101;	 
//  MasterWrite_Bytes(Sn_DIPR, array, 4);
//  /* Set local port 5000 */
//  array[0]=0x13;
//  array[1]=0x88;
//  MasterWrite_Bytes(Sn_PORT, array,2);
//  /* set destination port 6000 */
//  array[0]=0x00;
//  array[1]=0x66;
//  MasterWrite_Bytes(Sn_DPORTR,array,2);
//
//  //S0_Mode=TCP_CLIENT;//加载端口0的工作模式,TCP客户端模式
//} 
/******************************* W5500 Write Operation *******************************/
/* Write W5500 Common Register a byte */
void MasterWrite_1_Byte(unsigned short reg, unsigned char dat)
{
	/* Set W5500 SCS Low */
	TCP_MASTER_NSS_LOW;
	
	/* Write Address */
	SPI_I2S_SendData(SPI2,reg/256);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2,reg);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write Control Byte */
	SPI_I2S_SendData(SPI2,(FDM1|RWB_WRITE|COMMON_R));
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write 1 byte */
	SPI_I2S_SendData(SPI2,dat);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Set W5500 SCS High */
	TCP_MASTER_NSS_HIGH;
}

/* Write W5500 Common Register 2 bytes */
void MasterWrite_2_Byte(unsigned short reg, unsigned short dat)
{
	/* Set W5500 SCS Low */
	TCP_MASTER_NSS_LOW;
	
	/* Write Address */
	SPI_I2S_SendData(SPI2,reg/256);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2,reg);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write Control Byte */
	SPI_I2S_SendData(SPI2,(FDM2|RWB_WRITE|COMMON_R));
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write 2 bytes */
	SPI_I2S_SendData(SPI2,dat/256);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2,dat);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Set W5500 SCS High */
	TCP_MASTER_NSS_HIGH;
}

/* Write W5500 Common Register n bytes */
void MasterWrite_Bytes(unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short i;

	/* Set W5500 SCS Low */
	TCP_MASTER_NSS_LOW;
	
	/* Write Address */
	SPI_I2S_SendData(SPI2,reg/256);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2,reg);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write Control Byte */
	SPI_I2S_SendData(SPI2,(VDM|RWB_WRITE|COMMON_R));
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write n bytes */
	for(i=0;i<size;i++)
	{
		SPI_I2S_SendData(SPI2,*dat_ptr);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

		dat_ptr++;
	}

	/* Set W5500 SCS High */
	TCP_MASTER_NSS_HIGH;
}

/* Write W5500 Socket Register 1 byte */
void MasterWrite_SOCK_1_Byte(SOCKET s, unsigned short reg, unsigned char dat)
{
	/* Set W5500 SCS Low */
	TCP_MASTER_NSS_LOW;
	
	/* Write Address */
	SPI_I2S_SendData(SPI2,reg/256);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2,reg);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write Control Byte */
	SPI_I2S_SendData(SPI2,(FDM1|RWB_WRITE|(s*0x20+0x08)));
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write 1 byte */
	SPI_I2S_SendData(SPI2,dat);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Set W5500 SCS High */
	TCP_MASTER_NSS_HIGH;
}

/* Write W5500 Socket Register 2 byte */
void MasterWrite_SOCK_2_Byte(SOCKET s, unsigned short reg, unsigned short dat)
{
	/* Set W5500 SCS Low */
	TCP_MASTER_NSS_LOW;
	
	/* Write Address */
	SPI_I2S_SendData(SPI2,reg/256);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2,reg);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write Control Byte */
	SPI_I2S_SendData(SPI2,(FDM2|RWB_WRITE|(s*0x20+0x08)));
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write 2 bytes */
	SPI_I2S_SendData(SPI2,dat/256);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2,dat);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Set W5500 SCS High */
	TCP_MASTER_NSS_HIGH;
}

/* Write W5500 Socket Register 4 byte */
void MasterWrite_SOCK_4_Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
	/* Set W5500 SCS Low */
	TCP_MASTER_NSS_LOW;
	
	/* Write Address */
	SPI_I2S_SendData(SPI2,reg/256);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2,reg);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write Control Byte */
	SPI_I2S_SendData(SPI2,(FDM4|RWB_WRITE|(s*0x20+0x08)));
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write 4 bytes */
	SPI_I2S_SendData(SPI2,*dat_ptr);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	dat_ptr++;
	SPI_I2S_SendData(SPI2,*dat_ptr);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	dat_ptr++;
	SPI_I2S_SendData(SPI2,*dat_ptr);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	dat_ptr++;
	SPI_I2S_SendData(SPI2,*dat_ptr);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Set W5500 SCS High */
	TCP_MASTER_NSS_HIGH;
}
/******************** Write data to W5500 Socket data TX Buffer *******************/
void MasterWrite_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short offset,offset1;
	unsigned short i;

	offset=MasterRead_SOCK_2_Byte(s,Sn_TX_WR);
	offset1=offset;
	offset&=(S_TX_SIZE-1);		/* Calculate the real physical address */

	/* Set W5500 SCS Low */
	TCP_MASTER_NSS_LOW;

	/* Write Address */
	SPI_I2S_SendData(SPI2,offset/256);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2,offset);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write Control Byte */
	SPI_I2S_SendData(SPI2,(VDM|RWB_WRITE|(s*0x20+0x10)));		/* Read variable size */
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	if((offset+size)<S_TX_SIZE)
	{
		/* Write Data */
		for(i=0;i<size;i++)
		{
			SPI_I2S_SendData(SPI2,*dat_ptr);		/* Send a byte*/
			while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		
			dat_ptr++;
		}
	}
	else
	{
		offset=S_TX_SIZE-offset;
		for(i=0;i<offset;i++)
		{
			SPI_I2S_SendData(SPI2,*dat_ptr); 		/* Send a byte */
			while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	
			dat_ptr++;
		}
		/* Set W5500 SCS High*/
		TCP_MASTER_NSS_HIGH;

		/* Set W5500 SCS Low */
		TCP_MASTER_NSS_LOW;
		/* Write Address */
		SPI_I2S_SendData(SPI2,0x00);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPI2,0x00);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		/* Write Control Byte */
		SPI_I2S_SendData(SPI2,(VDM|RWB_WRITE|(s*0x20+0x10)));		/* Read variable size */
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		/* Read Data */
		for(;i<size;i++)
		{
			SPI_I2S_SendData(SPI2,*dat_ptr);		/* Send a byte */
			while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		
			dat_ptr++;
		}
	}
	/* Set W5500 SCS High*/
	TCP_MASTER_NSS_HIGH;

	/* Updata offset */
	offset1+=size;
	MasterWrite_SOCK_2_Byte(s, Sn_TX_WR, offset1);
	MasterWrite_SOCK_1_Byte(s, Sn_CR, SEND);					/* Write SEND Command */
}
/******************************* W5500 Read Operation *******************************/
/* Read W5500 Common register 1 Byte */
unsigned char MasterRead_1_Byte(unsigned short reg)
{
	unsigned char i;

	/* Set W5500 SCS Low */
	TCP_MASTER_NSS_LOW;

	/* Write Address */
	SPI_I2S_SendData(SPI2,reg/256);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2,reg);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write Control Byte */
	SPI_I2S_SendData(SPI2,(FDM1|RWB_READ|COMMON_R));
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write a dummy byte */
	i=SPI_I2S_ReceiveData(SPI2);
	SPI_I2S_SendData(SPI2,0x00);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Read 1 byte */
	i=SPI_I2S_ReceiveData(SPI2);

	/* Set W5500 SCS High*/
	TCP_MASTER_NSS_HIGH;
	return i;
}

/* Read W5500 Socket register 1 Byte */
unsigned char MasterRead_SOCK_1_Byte(SOCKET s, unsigned short reg)
{
	unsigned char i;

	/* Set W5500 SCS Low */
	TCP_MASTER_NSS_LOW;

	/* Write Address */
	SPI_I2S_SendData(SPI2,reg/256);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2,reg);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write Control Byte */
	SPI_I2S_SendData(SPI2,(FDM1|RWB_READ|(s*0x20+0x08)));
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write a dummy byte */
	i=SPI_I2S_ReceiveData(SPI2);
	SPI_I2S_SendData(SPI2,0x00);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Read 1 byte */
	i=SPI_I2S_ReceiveData(SPI2);

	/* Set W5500 SCS High*/
	TCP_MASTER_NSS_HIGH;
	return i;
}

/* Read W5500 Socket register 2 Bytes (short) */
unsigned short MasterRead_SOCK_2_Byte(SOCKET s, unsigned short reg)
{
	unsigned short i;

	/* Set W5500 SCS Low */
	TCP_MASTER_NSS_LOW;

	/* Write Address */
	SPI_I2S_SendData(SPI2,reg/256);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2,reg);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write Control Byte */
	SPI_I2S_SendData(SPI2,(FDM2|RWB_READ|(s*0x20+0x08)));
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write a dummy byte */
	i=SPI_I2S_ReceiveData(SPI2);
	SPI_I2S_SendData(SPI2,0x00);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	i=SPI_I2S_ReceiveData(SPI2);

	SPI_I2S_SendData(SPI2,0x00);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	i*=256;
	i+=SPI_I2S_ReceiveData(SPI2);

	/* Set W5500 SCS High*/
	TCP_MASTER_NSS_HIGH;
	return i;
}

/******************** Read data from W5500 Socket data RX Buffer *******************/
unsigned short MasterRead_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr)
{
	unsigned short rx_size;
	unsigned short offset, offset1;
	unsigned short i;
	unsigned char j;

	rx_size=MasterRead_SOCK_2_Byte(s,Sn_RX_RSR);
	if(rx_size==0)		/* if no data received, return */
		return 0;
	if(rx_size>1460)
		rx_size=1460;

	offset=MasterRead_SOCK_2_Byte(s,Sn_RX_RD);
	offset1=offset;
	offset&=(S_RX_SIZE-1);		/* Calculate the real physical address */

	/* Set W5500 SCS Low */
	TCP_MASTER_NSS_LOW;

	/* Write Address */
	SPI_I2S_SendData(SPI2,offset/256);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2,offset);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Write Control Byte */
	SPI_I2S_SendData(SPI2,(VDM|RWB_READ|(s*0x20+0x18)));		/* Read variable size */
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	j=SPI_I2S_ReceiveData(SPI2);
	
	if((offset+rx_size)<S_RX_SIZE)
	{
		/* Read Data */
		for(i=0;i<rx_size;i++)
		{
			SPI_I2S_SendData(SPI2,0x00);		/* Send a dummy byte */
			while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
			j=SPI_I2S_ReceiveData(SPI2);
			*dat_ptr=j;
			dat_ptr++;
		}
	}
	else
	{
		offset=S_RX_SIZE-offset;
		for(i=0;i<offset;i++)
		{
			SPI_I2S_SendData(SPI2,0x00); 		/* Send a dummy byte */
			while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
			j=SPI_I2S_ReceiveData(SPI2);
			*dat_ptr=j;
			dat_ptr++;
		}
		/* Set W5500 SCS High*/
		TCP_MASTER_NSS_HIGH;

		/* Set W5500 SCS Low */
		TCP_MASTER_NSS_LOW;
		/* Write Address */
		SPI_I2S_SendData(SPI2,0x00);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPI2,0x00);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		/* Write Control Byte */
		SPI_I2S_SendData(SPI2,(VDM|RWB_READ|(s*0x20+0x18)));		/* Read variable size */
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		j=SPI_I2S_ReceiveData(SPI2);
		/* Read Data */
		for(;i<rx_size;i++)
		{
			SPI_I2S_SendData(SPI2,0x00);		/* Send a dummy byte */
			while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
			j=SPI_I2S_ReceiveData(SPI2);
			*dat_ptr=j;
			dat_ptr++;
		}
	}
	/* Set W5500 SCS High*/
	TCP_MASTER_NSS_HIGH;

	/* Update offset*/
	offset1+=rx_size;
	MasterWrite_SOCK_2_Byte(s, Sn_RX_RD, offset1);
	MasterWrite_SOCK_1_Byte(s, Sn_CR, RECV);					/* Write RECV Command */
	return rx_size;
}
	   
/*********************** Set Socket n in TCP Client mode ************************/
unsigned int mb_tcp_master_Socket_Connect(SOCKET s)
{
	/* Set Socket n in TCP mode */
	MasterWrite_SOCK_1_Byte(s,Sn_MR,MR_TCP);
	/* Open Socket n */
	MasterWrite_SOCK_1_Byte(s,Sn_CR,OPEN);
	while(MasterRead_SOCK_1_Byte(s,Sn_CR));
	if(MasterRead_SOCK_1_Byte(s,Sn_SR)!=SOCK_INIT)
	{
		MasterWrite_SOCK_1_Byte(s,Sn_CR,CLOSE);		/* Close Socket n */
		return FALSE;
	}
	/* Set Socket n in Server mode */
	MasterWrite_SOCK_1_Byte(s,Sn_CR,CONNECT);
	while(MasterRead_SOCK_1_Byte(s,Sn_CR));
	return TRUE;
}

/********************* EXTI4_IRQHandler**********************************/
void EXTI1_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line0);
		//W5500_Interrupt=1;
	}
}

/*********************** Set Socket n in TCP Server mode ************************/
unsigned int MasterSocket_Listen(SOCKET s)
{
  /* Set Socket n in TCP mode */
  MasterWrite_SOCK_1_Byte(s,Sn_MR,MR_TCP);
  /* Open Socket n */
  MasterWrite_SOCK_1_Byte(s,Sn_CR,OPEN); 
  while(MasterRead_SOCK_1_Byte(0,Sn_CR));
  if(MasterRead_SOCK_1_Byte(s,Sn_SR)!=SOCK_INIT)
  {
  	MasterWrite_SOCK_1_Byte(s,Sn_CR,CLOSE);  	/* Close Socket n */
	return FALSE;
  }		   
  /* Set Socket n in Server mode */
  MasterWrite_SOCK_1_Byte(s,Sn_CR,LISTEN);
  while(MasterRead_SOCK_1_Byte(s,Sn_CR)); /* Wait for a moment */
  if(MasterRead_SOCK_1_Byte(s,Sn_SR)!=SOCK_LISTEN)
  {
  	MasterWrite_SOCK_1_Byte(s,Sn_CR,CLOSE);  	/* Close Socket n */
  	return FALSE;
  }
  return TRUE;
}
void MasterSetKeepAlive(SOCKET s,u16 reg,u8 dat)
{
  TCP_MASTER_NSS_LOW;	     
  /* Write Address */
  SPI_I2S_SendData(SPI2,reg/256);
  while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI2,reg);
  while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET); 
  /* Write Control SPI2 */
  SPI_I2S_SendData(SPI2,(RWB_WRITE|(s*0x20+0x08)));
  while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);   
  /* Write 1 byte */
  SPI_I2S_SendData(SPI2,dat);
  while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET); 
  /* Set W5500 SCS High */
  TCP_MASTER_NSS_HIGH;
}

void MasterSocket_Config(u8 s,uint16_t usTCPPort)
{
  /* set Socket n Port Number */
	MasterWrite_SOCK_2_Byte(s, Sn_PORT, usTCPPort);

	MasterWrite_SOCK_2_Byte(s,RTR,6000);//重试时间600mS

	MasterWrite_SOCK_2_Byte(s,RCR,3);	//重试次数：3

	/* Set Maximum Segment Size as 1460 */
	MasterWrite_SOCK_2_Byte(s, Sn_MSSR, 1460);

	/* Set RX Buffer Size as 2K */
	MasterWrite_SOCK_1_Byte(s,Sn_RXBUF_SIZE, 0x01);
	/* Set TX Buffer Size as 2K */
	MasterWrite_SOCK_1_Byte(s,Sn_TXBUF_SIZE, 0x01);
	/* IP address conflict and Destination unreachable interrupt*/
// 	MasterWrite_1_Byte(IMR,IM_IR7 | IM_IR6);
   /* Enable Socket 0 interrupt */
//	MasterWrite_1_Byte(SIMR,S0_IMR);

//	MasterWrite_SOCK_1_Byte(0, Sn_IMR, IMR_SENDOK | IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
}


/*******************************************************************************
* 函数名  : W5500_Interrupt_Process
* 描述    : W5500中断处理程序框架
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void mb_tcp_master_W5500_Interrupt_Process(u8 s)
{
  unsigned char i,j;
  u8 MASK_INT;
  MASK_INT=1<<s;
  i=MasterRead_1_Byte(SIR);//读取端口中断标志寄存器	
	if((i & MASK_INT) == MASK_INT)//Socket0事件处理 
	{
		j=MasterRead_SOCK_1_Byte(s,Sn_IR);//读取Socket0中断标志寄存器
		MasterWrite_SOCK_1_Byte(s,Sn_IR,j);
		if(j&IR_CON)//在TCP模式下,Socket0成功连接 
		{
			SocketConnectHook(s);
			mb_tcp_master_Socket_State[s]|=S_CONN;//网络连接状态0x02,端口完成连接，可以正常传输数据
		}
		if(j&IR_DISCON)//在TCP模式下Socket断开连接处理
		{
			MasterWrite_SOCK_1_Byte(s,Sn_CR,CLOSE);//关闭端口,等待重新打开连接 
			mb_tcp_master_Socket_Init(s);		//指定Socket(0~7)初始化,初始化端口0
			mb_tcp_master_Socket_State[s]=0;//网络连接状态0x00,端口连接失败
		}
		if(j&IR_SEND_OK)//Socket0数据发送完成,可以再次启动S_tx_process()函数发送数据 
		{
			mb_tcp_master_Socket_Data[s]|=S_TRANSMITOK;//端口发送一个数据包完成 
		}
		if(j&IR_RECV)//Socket接收到数据,可以启动S_rx_process()函数 
		{
			mb_tcp_master_Socket_Data[s]|=S_RECEIVE;//端口接收到一个数据包
		}
		if(j&IR_TIMEOUT)//Socket连接或数据传输超时处理 
		{
			MasterWrite_SOCK_1_Byte(s,Sn_CR,CLOSE);// 关闭端口,等待重新打开连接 			
			mb_tcp_master_Socket_State[s]=0;//网络连接状态0x00,端口连接失败
		}
	}
}


