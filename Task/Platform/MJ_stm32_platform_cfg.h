#ifndef MJ_STM32_PLATFORM_CFG_H
#define MJ_STM32_PLATFORM_CFG_H

#define SOCKETHOOK
/***********************************************************************/
/**************************** ����������ʼ *****************************/
#define GATEWAY_STATION_ADDRESS                0x18U      //�忨վ��ַ

enum ProtocalIndexTyp
{
  NODE_FX2NPROGRAM,         //0  ����FXϵ�б�̿�Э��
  NODE_FX2N485BD,           //1  ����FXϵ��485BD
  NODE_S7200PPI,            //2  ������S7200PPIͨѶ
  NODE_FPPROGRAM,           //3  ����FPX��̿�
  NODE_CP1LCOM,             //4  ŷķ������hostlinkЭ��
  NODE_MODBUSRTU,           //5  MODBUS_RTUЭ�飬���ܵ����
  NODE_M70V_NC,             //6  С�������ػ���
  NODE_S7200TCP,            //7  ������S7200��̫��PNЭ��
  NODE_S7300TCP,            //8  ������S7300��̫��PNЭ��
  NODE_MBTCP,               //9  MODBUS TCPЭ��
  NODE_MITSUBISHIQ,         //10 ����MC_ASCII��ʽЭ��
  NODE_MitsubishiA,         //11 ����Aϵ�б�̿�Э��
  NODE_FX3UENET_ADP2,       //12 ����FXϵ����չ��̫��Э��
  NODE_AB_COMPACTTCP,       //13 AB 1769����PLCЭ��
  NODE_FX2PROG,             //14 ����FX2ϵ�б�̿� Э��
  NODE_AB_Micro232,         //15 AB_MICROϵ�еͶ˴���Э��
  NODE_S7300MPI,            //16 S7300����MPIЭ��
  NODE_CP1L_FINS,           //17 ŷķ����̫��FINSЭ��
  NODE_MITSUBISHIQBIN,      //18 ����MC�����Ƹ�ʽЭ��
  NODE_MITSUBISHIQSERIAL,   //19 ���⴮��Э��
  NODE_S7400TCP,            //20 ������S7400��̫��PNЭ��
  NODE_S1200TCP,            //21 ������S1200��̫��PNЭ��
  NODE_SMART200TCP,         //22 ������200 SMART��̫��PNЭ��
  NODE_MAXINDEX
};
extern enum ProtocalIndexTyp ProtocalIndex;

#define WRITE_CYCLE                              1U      //д��ʱʱ��

/**************************** ������������ *****************************/
/***********************************************************************/
#define SW_VERSION                            "V3.920171108"
#define NODE_USER_DEFINED                         2U       //������״̬
#define NODE_DIG_ANG                              3U       //������״̬
#define MAXNUM_NODE                               4U      //֧�ֵ����ڵ�����
#define MAXNUM_READPROTOCAL                      80U      //�����ڵ�֧�ֵĲ����Ĵ���������
#define LENTH_BEFORE_DATA                         9U
#define LENTH_AFTER_DATA                          0U
#define LENTH_NONDATA       (LENTH_BEFORE_DATA+LENTH_AFTER_DATA)//�忨��һ������ͱ����г���������ֽ�����
#define THRES_TIMOUTCNT                        (1500u)      //��ʱ�����ﵽ����ֵ�������һ������
#define MAXNUMM_REGCFG                          6000       //֧�ֵ�������������üĴ������˴����ֽڱ�ʾ��ʵ����֧���������2000���Ĵ���
#define MAXNUM_ETHRECVDATA                      (256+LENTH_NONDATA)      //������̫����Ч���ݵ���������СֵΪ32
#define MAXNUM_BOARDSENDDATA                    (256+LENTH_NONDATA)      //�忨����̫���������ݵ��������

#define DIG_INPUT_ADDR                        0x0000
#define RELAY_OUT_ADDR                        0x0100

#define CHANNEL_UART1                             0U
#define CHANNEL_UART2                             1U
#define CHANNEL_UART3                             2U
#define CHANNEL_UART4                             3U
#define CHANNEL_UART5                             4U

//=================================================
// Port define 
#define RTS_ENABLE  	GPIO_ResetBits(GPIOA,GPIO_Pin_1)
#define RTS_DISABLE 	GPIO_SetBits(GPIOA,GPIO_Pin_1)
#define CTS_STATUS 	  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)
#define CTS_IDLE      0U
#define CTS_BUSY      1U

/* input Port E start	************************/
#define PE13				GPIO_Pin_13	// input
#define PE12				GPIO_Pin_12	// input
#define PE15				GPIO_Pin_15	// input
#define PE14				GPIO_Pin_14	// input
#define PB11				GPIO_Pin_11	// input
#define PB10				GPIO_Pin_10	// input
// input Port E alia 
#define INPUT1		PE13
#define INPUT2		PE12
#define INPUT3		PE15
#define INPUT4		PE14
#define INPUT5		PB11
#define INPUT6		PB10
/* input Port E end	****************************/

/* output Port  start	************************/
//relay output port define
#define PD11				GPIO_Pin_11	
#define PD12				GPIO_Pin_12	
#define PD13				GPIO_Pin_13	
#define PD14				GPIO_Pin_14	
#define PD15				GPIO_Pin_15	
#define PC6				  GPIO_Pin_6	
// relay output port alia 
#define RELAY_OUTPUT1		PD12
#define RELAY_OUTPUT2		PD11
#define RELAY_OUTPUT3		PD14
#define RELAY_OUTPUT4		PD13
#define RELAY_OUTPUT5		PC6
#define RELAY_OUTPUT6		PD15

#define RELAY1_ON		  GPIO_SetBits(GPIOD,RELAY_OUTPUT1)
#define RELAY2_ON		  GPIO_SetBits(GPIOD,RELAY_OUTPUT2)
#define RELAY3_ON		  GPIO_SetBits(GPIOD,RELAY_OUTPUT3)
#define RELAY4_ON		  GPIO_SetBits(GPIOD,RELAY_OUTPUT4)
#define RELAY5_ON		  GPIO_SetBits(GPIOC,RELAY_OUTPUT5)
#define RELAY6_ON		  GPIO_SetBits(GPIOD,RELAY_OUTPUT6)
#define RELAY1_OFF		GPIO_ResetBits(GPIOD,RELAY_OUTPUT1)
#define RELAY2_OFF  	GPIO_ResetBits(GPIOD,RELAY_OUTPUT2)
#define RELAY3_OFF		GPIO_ResetBits(GPIOD,RELAY_OUTPUT3)
#define RELAY4_OFF		GPIO_ResetBits(GPIOD,RELAY_OUTPUT4)
#define RELAY5_OFF		GPIO_ResetBits(GPIOC,RELAY_OUTPUT5)
#define RELAY6_OFF		GPIO_ResetBits(GPIOD,RELAY_OUTPUT6)

//LED indication output port  
#define PB7				GPIO_Pin_7	
//LED indication output port alia
#define LED   		PB7	
#define LED_ON		GPIO_ResetBits(GPIOB,LED)
#define LED_OFF   GPIO_SetBits(GPIOB,LED)
//Transistor output port define
#define PD7				  GPIO_Pin_7	
#define PC7				  GPIO_Pin_7	
#define PC8				  GPIO_Pin_8	
#define PC9				  GPIO_Pin_9	
#define PA8				  GPIO_Pin_8	
//Transistor output port alia 
#define FB_DRV1							PA8
#define FB_DRV2							PC7
#define DRV1							  PC8
#define DRV2							  PC9
#define VND5160_HIEN_DIS		PD7 
#define VND5160_EN					GPIO_ResetBits(GPIOD,VND5160_HIEN_DIS)
#define VND5160_DIS					GPIO_SetBits(GPIOD,VND5160_HIEN_DIS)
#define TOUT1_ON						GPIO_SetBits(GPIOC,DRV1)
#define TOUT1_OFF						GPIO_ResetBits(GPIOC,DRV1)
#define TOUT2_ON						GPIO_SetBits(GPIOC,DRV2)
#define TOUT2_OFF						GPIO_ResetBits(GPIOC,DRV2)
/* output Port  end	****************************/

/* 422 bus Port start	A B Z Y ************************/
#define PA9				  GPIO_Pin_0	// input
#define PA10				GPIO_Pin_1	// input
#define PA11 			  GPIO_Pin_2	// input
#define PA12				GPIO_Pin_3	// input
// 422 bus port alia 
#define UART1_TX			PE0
#define UART1_RX			PE1
#define UART1_RX_EN		PE2
#define UART1_TX_EN		PE3
/* RS422 bus Port end	  A B Z Y ****************************/

/* RS485 bus Port start d- d+*******************************/
//RS485 Port1 define 
#define PD4 					GPIO_Pin_4
#define PD5 					GPIO_Pin_5
#define PD6						GPIO_Pin_6
//RS485 Port1 alia 
#define UART2_TX		        PD5	
#define UART2_RX		        PD6	
#define UART2_RX_EN					PD4
//RS485 Port1 TX_EN/RX_EN 
#define USART2_485_TX_EN		GPIO_SetBits(GPIOD , UART2_RX_EN)	
#define USART2_485_RX_EN		GPIO_ResetBits(GPIOD , UART2_RX_EN)	
//RS485 Port2 define 
#define PD8 					  GPIO_Pin_8
#define PD9 					  GPIO_Pin_9
#define PD10						GPIO_Pin_10
//RS485 Port2 alia 
#define UART3_TX		        PD8	
#define UART3_RX		        PD9	
#define UART3_RX_EN					PD10
//RS485 Port2 TX_EN/RX_EN 
#define USART3_485_TX_EN		GPIO_SetBits(GPIOD , UART3_RX_EN)	
#define USART3_485_RX_EN		GPIO_ResetBits(GPIOD , UART3_RX_EN)	

/* RS485 bus Port end   d- d+*******************************/
			 
/* CAN bus Port start d- d+*******************************/
//CAN Port2 define 
#define PB5 					GPIO_Pin_5
#define PB6 					GPIO_Pin_6
//CAN Port2 alia 
#define CAN2_TX		        PB6	
#define CAN2_RX		        PB5	
//
//CAN Port1 define 
#define PD0 					  GPIO_Pin_0
#define PD1 					  GPIO_Pin_1
//CAN Port1 alia 
#define CAN1_TX		        PD1	
#define CAN1_RX		        PD0	
/* CAN bus Port end   d- d+*******************************/

/* RS232 bus Port start d- d+*******************************/
//RS232 Port1 define 
#define PC10 					GPIO_Pin_10
#define PC11 					GPIO_Pin_11
//RS232 Port1 alia 
#define UART4_TX		        PC10	
#define UART4_RX		        PC11	
//
//RS232 Port2 define 
#define PD2 					  GPIO_Pin_2
#define PC12 					  GPIO_Pin_12
//RS232 Port2 alia 
#define UART5_TX		        PC12	
#define UART5_RX		        PD2	
/* RS232 bus Port end   d- d+*******************************/

/* W5500 ethernet RJ45_1 port define start*************************/
//Port define 
#define PA0  				GPIO_Pin_0
#define PA3					GPIO_Pin_3
#define PA4					GPIO_Pin_4
#define PA5					GPIO_Pin_5
#define PA6					GPIO_Pin_6
#define PA7					GPIO_Pin_7
//Port alia define 
#define WIZ_INT_SLAVE				PA0 
#define WIZ_RESET_SLAVE		  PA3	
#define WIZ_SCS_SLAVE			  PA4	
#define WIZ_SCLK_SLAVE			PA5	
#define WIZ_MISO_SLAVE			PA6	
#define WIZ_MOSI_SLAVE			PA7	
/* W5500 ethernet RJ45_1 port define end*************************/

/* W5500 ethernet RJ45_2 port define start*************************/
//Port define 
#define PA1					  GPIO_Pin_1
#define PD3					  GPIO_Pin_3
#define PB12  				GPIO_Pin_12
#define PB13			   	GPIO_Pin_13
#define PB14					GPIO_Pin_14
#define PB15					GPIO_Pin_15
//Port alia define 
#define WIZ_INT_MASTER				PA0 
#define WIZ_RESET_MASTER		  PD3	
#define WIZ_SCS_MASTER			  PB12	
#define WIZ_SCLK_MASTER				PB13	
#define WIZ_MISO_MASTER				PB14	
#define WIZ_MOSI_MASTER				PB15	
/* W5500 ethernet RJ45_2 port define end*************************/

/* AD input port define start ******************************/
//Port define 
#define PC0					  GPIO_Pin_0
#define PC1					  GPIO_Pin_1
#define PC2  					GPIO_Pin_2
#define PC3			   		GPIO_Pin_3
#define PB1			   		GPIO_Pin_1
//Port alia define 
#define AD_INPUT1				PC0 
#define AD_INPUT2		    PC1	
#define AD_INPUT3			  PC2	
#define AD_INPUT4			  PC3	
#define AD_POWER			  PB1	
/* AD input port define end ******************************/


#endif
