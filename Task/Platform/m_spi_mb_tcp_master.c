#include "main.h"
#include "stm32f10x.h"   
#include "stm32f10x_spi.h" 
#include "m_w5500.h"              
#include "m_spi_mb_tcp_master.h"		     
#include "t_mb_tcp_master.h"		     
#include "mes.h"	 


/***************----- ��������������� -----***************/
unsigned char mb_tcp_master_Gateway_IP[4];//����IP��ַ 
unsigned char mb_tcp_master_Sub_Mask[4];	//�������� 
unsigned char mb_tcp_master_Phy_Addr[6];	//�����ַ(MAC) 
unsigned char mb_tcp_master_IP_Addr[4];	//����IP��ַ 

unsigned char mb_tcp_master_Socket_Port[8][2];	//�˿�0�Ķ˿ں�(5000) 
unsigned char mb_tcp_master_Socket_DIP[8][4];	//�˿�0Ŀ��IP��ַ 
unsigned char mb_tcp_master_Socket_DPort[8][2];	//�˿�0Ŀ�Ķ˿ں�(6000) 

unsigned char mb_tcp_master_UDP_DIPR[4];	//UDP(�㲥)ģʽ,Ŀ������IP��ַ
unsigned char mb_tcp_master_UDP_DPORT[2];	//UDP(�㲥)ģʽ,Ŀ�������˿ں�

/***************----- �˿ڵ�����ģʽ -----***************/
unsigned char mb_tcp_master_Socket_Mode[8] ={3,3,3,3,3,3,3,3};	//�˿�0������ģʽ,0:TCP������ģʽ,1:TCP�ͻ���ģʽ,2:UDP(�㲥)ģʽ
#define TCP_SERVER	0x00	//TCP������ģʽ
#define TCP_CLIENT	0x01	//TCP�ͻ���ģʽ 
#define UDP_MODE	0x02	//UDP(�㲥)ģʽ 

/***************----- �˿ڵ�����״̬ -----***************/
unsigned char mb_tcp_master_Socket_State[8] ={0,0,0,0,0,0,0,0};	//�˿�0״̬��¼,1:�˿���ɳ�ʼ��,2�˿��������(����������������) 


/***************----- �˿��շ����ݵ�״̬ -----***************/
unsigned char mb_tcp_master_Socket_Data[8];		//�˿�0���պͷ������ݵ�״̬,1:�˿ڽ��յ�����,2:�˿ڷ���������� 
#define S_RECEIVE	 0x01	//�˿ڽ��յ�һ�����ݰ� 
#define S_TRANSMITOK 0x02	//�˿ڷ���һ�����ݰ���� 

/***************----- �˿����ݻ����� -----***************/
unsigned char mb_tcp_master_Rx_Buffer[S_RX_SIZE];	//�˿ڽ������ݻ����� 
unsigned char mb_tcp_master_Tx_Buffer[S_TX_SIZE];	//�˿ڷ������ݻ����� 

unsigned char mb_tcp_master_W5500_Interrupt;	//W5500�жϱ�־(0:���ж�,1:���ж�)

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
* ������  : W5500_Initialization
* ����    : W5500��ʼ������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void mb_tcp_master_W5500_Init(void)
{
  mb_tcp_master_Load_Net_Parameters();
	mb_tcp_master_W5500_Initialization();		//��ʼ��W5500�Ĵ�������
	mb_tcp_master_Detect_Gateway();	//������ط����� 
	//mb_tcp_master_Socket_Init(0);		//ָ��Socket(0~7)��ʼ��
}
/*******************************************************************************
* ������  : W5500_Init
* ����    : ��ʼ��W5500�Ĵ�������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��ʹ��W5500֮ǰ���ȶ�W5500��ʼ��
*******************************************************************************/
void mb_tcp_master_W5500_Initialization(void)
{
	u8 i=0;

	MasterWrite_1_Byte(MR, RST);//�����λW5500,��1��Ч,��λ���Զ���0
	Delay1ms(10);//��ʱ10ms,�Լ�����ú���

	//��������(Gateway)��IP��ַ,Gateway_IPΪ4�ֽ�unsigned char����,�Լ����� 
	//ʹ�����ؿ���ʹͨ��ͻ�������ľ��ޣ�ͨ�����ؿ��Է��ʵ��������������Internet
	MasterWrite_Bytes(GAR, mb_tcp_master_Gateway_IP, 4);
			
	//������������(MASK)ֵ,SUB_MASKΪ4�ֽ�unsigned char����,�Լ�����
	//��������������������
	MasterWrite_Bytes(SUBR,mb_tcp_master_Sub_Mask,4);		
	
	//���������ַ,PHY_ADDRΪ6�ֽ�unsigned char����,�Լ�����,����Ψһ��ʶ�����豸�������ֵַ
	//�õ�ֵַ��Ҫ��IEEE���룬����OUI�Ĺ涨��ǰ3���ֽ�Ϊ���̴��룬�������ֽ�Ϊ��Ʒ���
	//����Լ����������ַ��ע���һ���ֽڱ���Ϊż��
	MasterWrite_Bytes(SHAR,mb_tcp_master_Phy_Addr,6);		

	//���ñ�����IP��ַ,IP_ADDRΪ4�ֽ�unsigned char����,�Լ�����
	//ע�⣬����IP�����뱾��IP����ͬһ�����������򱾻����޷��ҵ�����
	MasterWrite_Bytes(SIPR,mb_tcp_master_IP_Addr,4);		
	
	//���÷��ͻ������ͽ��ջ������Ĵ�С���ο�W5500�����ֲ�
	for(i=0;i<8;i++)
	{
		MasterWrite_SOCK_1_Byte(i,Sn_RXBUF_SIZE, 0x01);//Socket Rx memory size=2k
		MasterWrite_SOCK_1_Byte(i,Sn_TXBUF_SIZE, 0x01);//Socket Tx mempry size=2k
	}

	//��������ʱ�䣬Ĭ��Ϊ2000(200ms) 
	//ÿһ��λ��ֵΪ100΢��,��ʼ��ʱֵ��Ϊ2000(0x07D0),����200����
	MasterWrite_2_Byte(RTR, 0x07d0);

	//�������Դ�����Ĭ��Ϊ8�� 
	//����ط��Ĵ��������趨ֵ,�������ʱ�ж�(��صĶ˿��жϼĴ����е�Sn_IR ��ʱλ(TIMEOUT)�á�1��)
	MasterWrite_1_Byte(RCR,8);
}

/*******************************************************************************
* ������  : Detect_Gateway
* ����    : ������ط�����
* ����    : ��
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ��
*******************************************************************************/
unsigned char mb_tcp_master_Detect_Gateway(void)
{
	unsigned char ip_adde[4];
	ip_adde[0]=mb_tcp_master_IP_Addr[0]+1;
	ip_adde[1]=mb_tcp_master_IP_Addr[1]+1;
	ip_adde[2]=mb_tcp_master_IP_Addr[2]+1;
	ip_adde[3]=mb_tcp_master_IP_Addr[3]+1;

	//������ؼ���ȡ���ص������ַ
	MasterWrite_SOCK_4_Byte(0,Sn_DIPR,ip_adde);//��Ŀ�ĵ�ַ�Ĵ���д���뱾��IP��ͬ��IPֵ
	MasterWrite_SOCK_1_Byte(0,Sn_MR,MR_TCP);//����socketΪTCPģʽ
	MasterWrite_SOCK_1_Byte(0,Sn_CR,OPEN);//��Socket	
	Delay1ms(5);//��ʱ5ms 	
	
}

/*******************************************************************************
* ������  : Socket_Init
* ����    : ָ��Socket(0~7)��ʼ��
* ����    : s:����ʼ���Ķ˿�
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
unsigned char mb_tcp_master_Socket_Init(SOCKET s)
{
  u8 ret;
  ret=0xff;
  if(MasterRead_SOCK_1_Byte(s,Sn_SR) != SOCK_INIT)//���socket��ʧ��
	{
		MasterWrite_SOCK_1_Byte(s,Sn_CR,CLOSE);//�򿪲��ɹ�,�ر�Socket
		return FALSE;//����FALSE(0x00)
	}

	MasterWrite_SOCK_1_Byte(s,Sn_CR,CONNECT);//����SocketΪConnectģʽ						

	do
	{
		u8 j=0;
		j=MasterRead_SOCK_1_Byte(s,Sn_IR);//��ȡSocket0�жϱ�־�Ĵ���
		if(j!=0)
		MasterWrite_SOCK_1_Byte(s,Sn_IR,j);
		Delay1ms(4);//��ʱ5ms 
		if((j&IR_TIMEOUT) == IR_TIMEOUT)
		{
			MasterWrite_SOCK_2_Byte(s, Sn_MSSR, 1460);//���÷�Ƭ���ȣ��ο�W5500�����ֲᣬ��ֵ���Բ��޸�	
    	//���ö˿�0�Ķ˿ں�
    	MasterWrite_SOCK_2_Byte(s, Sn_PORT, mb_tcp_master_Socket_Port[s][0]*256+mb_tcp_master_Socket_Port[s][1]);
    	//���ö˿�0Ŀ��(Զ��)�˿ں�
    	MasterWrite_SOCK_2_Byte(s, Sn_DPORTR, mb_tcp_master_Socket_DPort[s][0]*256+mb_tcp_master_Socket_DPort[s][1]);
    	//���ö˿�0Ŀ��(Զ��)IP��ַ
    	MasterWrite_SOCK_4_Byte(s, Sn_DIPR, mb_tcp_master_Socket_DIP[s]);	
    	return FALSE;
		}
		else if(MasterRead_SOCK_1_Byte(s,Sn_DHAR) != 0xff)
		{
			MasterWrite_SOCK_1_Byte(s,Sn_CR,CLOSE);//�ر�Socket
			return TRUE;							
		}
		else{

		}
	}while((ret!=TRUE)&&(ret!=FALSE));
}
/*******************************************************************************
* ������  : Load_Net_Parameters
* ����    : װ���������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ���ء����롢�����ַ������IP��ַ���˿ںš�Ŀ��IP��ַ��Ŀ�Ķ˿ںš��˿ڹ���ģʽ
*******************************************************************************/
void mb_tcp_master_Load_Net_Parameters(void)
{
	u8 i;
	GPIO_SetBits(TCP_MASTER_RST_PORT, TCP_MASTER_RST_PIN);
	Delay1ms(4);
	mb_tcp_master_Gateway_IP[0] = g_u8TcpMasterGw[0];//�������ز���
	mb_tcp_master_Gateway_IP[1] = g_u8TcpMasterGw[1];
	mb_tcp_master_Gateway_IP[2] = g_u8TcpMasterGw[2];
	mb_tcp_master_Gateway_IP[3] = g_u8TcpMasterGw[3];

	mb_tcp_master_Sub_Mask[0]=g_u8TcpMasterSub[0];//������������
	mb_tcp_master_Sub_Mask[1]=g_u8TcpMasterSub[1];
	mb_tcp_master_Sub_Mask[2]=g_u8TcpMasterSub[2];
	mb_tcp_master_Sub_Mask[3]=g_u8TcpMasterSub[3];

	mb_tcp_master_Phy_Addr[0]=0x0c;//���������ַ
	mb_tcp_master_Phy_Addr[1]=0x29;
	mb_tcp_master_Phy_Addr[2]=0xab;
	mb_tcp_master_Phy_Addr[3]=g_u8TcpMasterLclIP[1];
	mb_tcp_master_Phy_Addr[4]=g_u8TcpMasterLclIP[2];
	mb_tcp_master_Phy_Addr[5]=g_u8TcpMasterLclIP[3];//��ֹMAC��ַ��ͻ

	mb_tcp_master_IP_Addr[0]=g_u8TcpMasterLclIP[0];//���ر���IP��ַ
	mb_tcp_master_IP_Addr[1]=g_u8TcpMasterLclIP[1];
	mb_tcp_master_IP_Addr[2]=g_u8TcpMasterLclIP[2];
	mb_tcp_master_IP_Addr[3]=g_u8TcpMasterLclIP[3];

	for(i=0;i<8;i++)
	{
		mb_tcp_master_Socket_Port[i][0]=g_u16TcpMasterLclPort[i]/256;//���ض˿�0�Ķ˿ں�5000 
		mb_tcp_master_Socket_Port[i][1]=g_u16TcpMasterLclPort[i]%256;

		mb_tcp_master_Socket_DIP[i][0]=g_u8TcpMasterRmtIP[0];//���ض˿�0��Ŀ��IP��ַ
		mb_tcp_master_Socket_DIP[i][1]=g_u8TcpMasterRmtIP[1];
		mb_tcp_master_Socket_DIP[i][2]=g_u8TcpMasterRmtIP[2];
		mb_tcp_master_Socket_DIP[i][3]=g_u8TcpMasterRmtIP[3];
	
		mb_tcp_master_Socket_DPort[i][0] = g_u16TcpMasterRmtPort[i] / 256;//���ض˿�0��Ŀ�Ķ˿ں�6000
		mb_tcp_master_Socket_DPort[i][1] = g_u16TcpMasterRmtPort[i] % 256;

		mb_tcp_master_Socket_Mode[i]=TCP_CLIENT;//���ض˿�0�Ĺ���ģʽ,TCP�ͻ���ģʽ
	}
}


/*******************************************************************************
* ������  : W5500_Socket_Set
* ����    : W5500�˿ڳ�ʼ������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : �ֱ�����4���˿�,���ݶ˿ڹ���ģʽ,���˿�����TCP��������TCP�ͻ��˻�UDPģʽ.
*			�Ӷ˿�״̬�ֽ�Socket_State�����ж϶˿ڵĹ������
*******************************************************************************/
void mb_tcp_master_W5500_Socket_Set(u8 s)
{
	if((mb_tcp_master_Socket_State[s]&2)!=2)//�˿�0��ʼ������ 
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

	/*����GPIOx������ʱ��*/
	/* Enable SPI2 and GPIOA clocks */
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);	
	/* ��ʼ��SCK��MISO��MOSI���� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/* ��ʼ��CS���� */
	GPIO_InitStructure.GPIO_Pin = TCP_MASTER_NSS_PIN;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(TCP_MASTER_NSS_PORT, &GPIO_InitStructure);
	GPIO_SetBits(TCP_MASTER_NSS_PORT, TCP_MASTER_NSS_PIN);

	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;	//SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;							//����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;						//SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;							//ʱ�����յ�
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;							//���ݲ����ڵ�1��ʱ����
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;								//NSS���ⲿ�ܽŹ���
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;	//������Ԥ��ƵֵΪ2
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;					//���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial=7;								//CRC����ʽΪ7
	SPI_Init(SPI2,&SPI_InitStructure);									//����SPI_InitStruct��ָ���Ĳ�����ʼ������SPI2�Ĵ���
	SPI_Cmd(SPI2,ENABLE);	//STM32ʹ��SPI2
   /*   NSS PB12ʹ�����   */ 
	/* W5500_RST���ų�ʼ������(PC0) */
/*	GPIO_InitStructure.GPIO_Pin  = TCP_MASTER_RST_PIN;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(TCP_MASTER_RST_PORT, &GPIO_InitStructure);*/

}
//void TCP_MasterW5500_Configuration(void)
//{
//  unsigned char array[6];	
//  GPIO_SetBits(TCP_MASTER_RST_PORT, TCP_MASTER_RST_PIN);
//  Delay1ms(4);  /*delay 100ms ʹ��systick 1msʱ������ʱ*/
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
//  //���÷��ͻ������ͽ��ջ������Ĵ�С���ο�W5500�����ֲ�
//  //for(i=0;i<8;i++)
//  //{
//  //  Write_W5500_SOCK_1Byte(i,Sn_RXBUF_SIZE, 0x02);//Socket Rx memory size=2k
//  //  Write_W5500_SOCK_1Byte(i,Sn_TXBUF_SIZE, 0x02);//Socket Tx mempry size=2k
//  //}				
//  //��������ʱ�䣬Ĭ��Ϊ2000(200ms) 
//  //ÿһ��λ��ֵΪ100΢��,��ʼ��ʱֵ��Ϊ2000(0x07D0),����200����
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
//  //S0_Mode=TCP_CLIENT;//���ض˿�0�Ĺ���ģʽ,TCP�ͻ���ģʽ
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

	MasterWrite_SOCK_2_Byte(s,RTR,6000);//����ʱ��600mS

	MasterWrite_SOCK_2_Byte(s,RCR,3);	//���Դ�����3

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
* ������  : W5500_Interrupt_Process
* ����    : W5500�жϴ��������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void mb_tcp_master_W5500_Interrupt_Process(u8 s)
{
  unsigned char i,j;
  u8 MASK_INT;
  MASK_INT=1<<s;
  i=MasterRead_1_Byte(SIR);//��ȡ�˿��жϱ�־�Ĵ���	
	if((i & MASK_INT) == MASK_INT)//Socket0�¼����� 
	{
		j=MasterRead_SOCK_1_Byte(s,Sn_IR);//��ȡSocket0�жϱ�־�Ĵ���
		MasterWrite_SOCK_1_Byte(s,Sn_IR,j);
		if(j&IR_CON)//��TCPģʽ��,Socket0�ɹ����� 
		{
			SocketConnectHook(s);
			mb_tcp_master_Socket_State[s]|=S_CONN;//��������״̬0x02,�˿�������ӣ�����������������
		}
		if(j&IR_DISCON)//��TCPģʽ��Socket�Ͽ����Ӵ���
		{
			MasterWrite_SOCK_1_Byte(s,Sn_CR,CLOSE);//�رն˿�,�ȴ����´����� 
			mb_tcp_master_Socket_Init(s);		//ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
			mb_tcp_master_Socket_State[s]=0;//��������״̬0x00,�˿�����ʧ��
		}
		if(j&IR_SEND_OK)//Socket0���ݷ������,�����ٴ�����S_tx_process()������������ 
		{
			mb_tcp_master_Socket_Data[s]|=S_TRANSMITOK;//�˿ڷ���һ�����ݰ���� 
		}
		if(j&IR_RECV)//Socket���յ�����,��������S_rx_process()���� 
		{
			mb_tcp_master_Socket_Data[s]|=S_RECEIVE;//�˿ڽ��յ�һ�����ݰ�
		}
		if(j&IR_TIMEOUT)//Socket���ӻ����ݴ��䳬ʱ���� 
		{
			MasterWrite_SOCK_1_Byte(s,Sn_CR,CLOSE);// �رն˿�,�ȴ����´����� 			
			mb_tcp_master_Socket_State[s]=0;//��������״̬0x00,�˿�����ʧ��
		}
	}
}


