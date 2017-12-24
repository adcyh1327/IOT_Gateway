#ifndef MJ_STM32_PLATFORM_CFG_H
#define MJ_STM32_PLATFORM_CFG_H

#define SOCKETHOOK
/***********************************************************************/
/**************************** 可配置区开始 *****************************/
#define GATEWAY_STATION_ADDRESS                0x18U      //板卡站地址

enum ProtocalIndexTyp
{
  NODE_FX2NPROGRAM,         //0  三菱FX系列编程口协议
  NODE_FX2N485BD,           //1  三菱FX系列485BD
  NODE_S7200PPI,            //2  西门子S7200PPI通讯
  NODE_FPPROGRAM,           //3  松下FPX编程口
  NODE_CP1LCOM,             //4  欧姆龙串口hostlink协议
  NODE_MODBUSRTU,           //5  MODBUS_RTU协议，智能电表常用
  NODE_M70V_NC,             //6  小立加数控机床
  NODE_S7200TCP,            //7  西门子S7200以太网PN协议
  NODE_S7300TCP,            //8  西门子S7300以太网PN协议
  NODE_MBTCP,               //9  MODBUS TCP协议
  NODE_MITSUBISHIQ,         //10 三菱MC_ASCII格式协议
  NODE_MitsubishiA,         //11 三菱A系列编程口协议
  NODE_FX3UENET_ADP2,       //12 三菱FX系列扩展以太网协议
  NODE_AB_COMPACTTCP,       //13 AB 1769中型PLC协议
  NODE_FX2PROG,             //14 三菱FX2系列编程口 协议
  NODE_AB_Micro232,         //15 AB_MICRO系列低端串口协议
  NODE_S7300MPI,            //16 S7300串口MPI协议
  NODE_CP1L_FINS,           //17 欧姆龙以太网FINS协议
  NODE_MITSUBISHIQBIN,      //18 三菱MC二进制格式协议
  NODE_MITSUBISHIQSERIAL,   //19 三菱串口协议
  NODE_S7400TCP,            //20 西门子S7400以太网PN协议
  NODE_S1200TCP,            //21 西门子S1200以太网PN协议
  NODE_SMART200TCP,         //22 西门子200 SMART以太网PN协议
  NODE_MAXINDEX
};
extern enum ProtocalIndexTyp ProtocalIndex;

#define WRITE_CYCLE                              1U      //写超时时间

/**************************** 可配置区结束 *****************************/
/***********************************************************************/
#define SW_VERSION                            "V3.920171108"
#define NODE_USER_DEFINED                         2U       //开关量状态
#define NODE_DIG_ANG                              3U       //开关量状态
#define MAXNUM_NODE                               4U      //支持的最大节点数量
#define MAXNUM_READPROTOCAL                      80U      //单个节点支持的操作寄存器的数量
#define LENTH_BEFORE_DATA                         9U
#define LENTH_AFTER_DATA                          0U
#define LENTH_NONDATA       (LENTH_BEFORE_DATA+LENTH_AFTER_DATA)//板卡向一体机发送报文中除数据外的字节数量
#define THRES_TIMOUTCNT                        (1500u)      //超时计数达到该阈值后进行下一个命令
#define MAXNUMM_REGCFG                          6000       //支持的最大数量的配置寄存器，此处以字节表示，实际上支持最大配置2000个寄存器
#define MAXNUM_ETHRECVDATA                      (256+LENTH_NONDATA)      //接收以太网有效数据的数量，最小值为32
#define MAXNUM_BOARDSENDDATA                    (256+LENTH_NONDATA)      //板卡向以太网传输数据的最大数量

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
