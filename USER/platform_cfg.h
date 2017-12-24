#ifndef __PLATFORM_CFG_H_
#define __PLATFORM_CFG_H_

#include "stm32f10x.h"

#ifdef STM32F10X_LD
    #define FLASH_PAGE_SIZE                 0x400
    #define CONFIG_BASEADDR                 0x800EC00ul  //常用配置文件的存储起始地址
#endif

#ifdef STM32F10X_MD
    #define FLASH_PAGE_SIZE                 0x400
    #define CONFIG_BASEADDR                 0x800EC00ul
#endif

#ifdef STM32F10X_HD
    #define FLASH_PAGE_SIZE                 0x800
    #define CONFIG_BASEADDR                 0x803A000ul
#endif

#ifdef STM32F10X_CL
    #define FLASH_PAGE_SIZE                 0x800     
    #define CONFIG_BASEADDR                 0x803A000ul
#endif

#define AIRCR_VECTKEY_MASK    ((uint32_t)0x05FA0000)

/***********************************************************************/
/***************************功能使能区**********************************/
/***********************************************************************/
// #define DEBUG_ENABLE   //debug模式下禁止看门狗功能和W5500自动keepalive功能

#ifndef DEBUG_ENABLE
 #define WATCHDOG_ENABLE                            //看门狗宏开关，不需要打开看门狗时需要将此宏屏蔽       
#endif 
#define W5500_ENABLE                                  //W5500芯片使能，当需要网口进行通讯时该宏定义必须是激活状态
#if (defined STM32F10X_HD) && (defined STM32F10X_CL)
#define UART_DMA_ENABLE                               //串口DMA使能
#endif

#ifdef W5500_ENABLE
#define TCPIP_ENABLE                                  //TCPIP功能使能
#define MBTCP_ENABLE                                  //MBTCP功能使能
#define MQTT_ENABLE                                   //MQTT功能使能
#define HTTP_ENABLE                                   //HTTP功能使能
#endif

#define CJSON_ENABLE
/***********************************************************************/

/**************** 中断等级分配 *******************/
#define USART1_PRE					0
#define USART1_SUB					0
#define USART2_PRE					0
#define USART2_SUB					0
#define USART3_PRE					0
#define USART3_SUB					0
#define UART4_PRE					0
#define UART4_SUB					0
#define UART5_PRE					0
#define UART5_SUB					0

#define TIM2_PRE					0
#define TIM2_SUB					2
#define TIM3_PRE					0
#define TIM3_SUB					2
#define TIM4_PRE					0
#define TIM4_SUB					2

#define CAN1_RX0_PRE				0
#define CAN1_RX0_SUB				1

#define WWDG_PRE				    0
#define WWDG_SUB				    3


/**************** 本地套接字分配 *******************/
#define SOCK_TCPIP                     0x00u
#define SOCK_MBTCP                     0x01u
#define SOCK_MQTT                      0x06u
#define SOCK_HTTP                      0x07u

/**************** 本地端口号分配 *******************/
#define PORT_LOCAL        5000

#define APP_CRC_ADDR                                          0x8024000

/* ASCII */
#define SOH                             0x01
#define STX                             0x02
#define ETX                             0x03
#define EOT                             0x04
#define ENQ                             0x05
#define ACK                             0x06
#define NAK                             0x15
#define SYN                             0x16
#define ETB                             0x17

/* */
#define NONE                            0
#define TRUE                            1
#define FALSE                           0
#define ON                              1
#define OFF                             0
#define ALL                             0xff

/**************************************** LED ****************************************/
#define LEDG_PORT                   GPIOD
#define LEDG_PIN                    GPIO_Pin_9
#define LED_G(n)                    { (n) ? ( GPIO_ResetBits(LEDG_PORT,LEDG_PIN)) : (GPIO_SetBits(LEDG_PORT,LEDG_PIN)); }

#define LEDR_PORT                   GPIOD
#define LEDR_PIN                    GPIO_Pin_8
#define LED_R(n)                    { (n) ? ( GPIO_ResetBits(LEDR_PORT,LEDR_PIN)) : (GPIO_SetBits(LEDR_PORT,LEDR_PIN)); }

#define IO_TRIGGER(port,pin)        { if(GPIO_ReadOutputDataBit(port,pin)) GPIO_ResetBits(port,pin); else GPIO_SetBits(port,pin); }

/**************************************** USART ****************************************/
//USART1
#define USART1_PORT                     GPIOA
#define USART1_GPIO_APB                 RCC_APB2Periph_GPIOA
#define USART1_TX_PIN                   GPIO_Pin_7
#define USART1_RX_PIN                   GPIO_Pin_7
#define USART1_RTS_PIN                  GPIO_Pin_7
#define USART1_CTS_PIN                  GPIO_Pin_7
//USART2
#define USART2_PORT                     GPIOA
#define USART2_GPIO_APB                 RCC_APB2Periph_GPIOA
#define USART2_TX_PIN                   GPIO_Pin_2
#define USART2_RX_PIN                   GPIO_Pin_3
#define USART2_RTS_PIN                  GPIO_Pin_1
#define USART2_CTS_PIN                  GPIO_Pin_0
//USART3
#define USART3_PORT                     GPIOB
#define USART3_GPIO_APB                 RCC_APB2Periph_GPIOB
#define USART3_TX_PIN                   GPIO_Pin_10
#define USART3_RX_PIN                   GPIO_Pin_11
#define USART3_RTS_PIN                  GPIO_Pin_14
#define USART3_CTS_PIN                  GPIO_Pin_13
//UART4
#define UART4_PORT                      GPIOC
#define UART4_GPIO_APB                  RCC_APB2Periph_GPIOC
#define UART4_TX_PIN                    GPIO_Pin_10
#define UART4_RX_PIN                    GPIO_Pin_11
#define UART4_EN_PORT                   GPIOC
#define UART4_EN_PIN                    GPIO_Pin_12
#define UART4_485_TX_ENABLE 	        GPIO_SetBits(UART4_EN_PORT , UART4_EN_PIN)	//发送使能
#define UART4_485_RX_ENABLE	            GPIO_ResetBits(UART4_EN_PORT , UART4_EN_PIN)  	//接收使能
//UART5
#define UART5_PORT                      GPIOC
#define UART5_GPIO_APB                  RCC_APB2Periph_GPIOC
#define UART5_TX_PIN                    GPIO_Pin_10
#define UART5_RX_PIN                    GPIO_Pin_11
#define UART5_EN_PORT                   GPIOC
#define UART5_EN_PIN                    GPIO_Pin_12
#define UART5_485_TX_ENABLE 	        GPIO_ResetBits(UART4_EN_PORT , UART4_EN_PIN)	//发送使能
#define UART5_485_RX_ENABLE	            GPIO_SetBits(UART4_EN_PORT , UART4_EN_PIN)  	//接收使能

/**************************************** CAN ****************************************/
#define CAN_TX_PIN		                GPIO_Pin_1
#define CAN_RX_PIN	                    GPIO_Pin_0
#define CAN_PORT     	                GPIOD
#define CAN_GPIO_APB                    RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO

/**************************************** Serve ****************************************/
/* GPIO Clock */
#define SPI1_PERIPH_RELATE1             0
#define SPI1_PERIPH_RELATE2             RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO
/* SPI1 PIN */
#define SPI1_GPIO_PORT                  GPIOA
#define SPI1_GPIO_PIN                   GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7
/* W5500 CS */ 
#define TCP_SLAVE_CS_PORT				GPIOA
#define TCP_SLAVE_CS_PIN				GPIO_Pin_4
#define TCP_SLAVE_CS_LOW				GPIO_ResetBits(TCP_SLAVE_CS_PORT, TCP_SLAVE_CS_PIN)
#define TCP_SLAVE_CS_HIGH				GPIO_SetBits(TCP_SLAVE_CS_PORT, TCP_SLAVE_CS_PIN)
/* W5500 RST */
#define TCP_SLAVE_RST_PORT				GPIOC
#define TCP_SLAVE_RST_PIN 				GPIO_Pin_5
#define TCP_SLAVE_RST_LOW               GPIO_ResetBits(TCP_SLAVE_RST_PORT, TCP_SLAVE_RST_PIN)
#define TCP_SLAVE_RST_HIGH              GPIO_SetBits(TCP_SLAVE_RST_PORT, TCP_SLAVE_RST_PIN)
/* W5500 INT */
#define TCP_SLAVE_INT_PORT				GPIOA
#define TCP_SLAVE_INT_PIN				GPIO_Pin_0

/**************************************** SPI1 ****************************************/
/* GPIO Clock */
#define RCC_SPI1_RELATE_IO_ENABLE       RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO,ENABLE)
/* SPI Clock */
#define RCC_SPI1_PERIPH_ENABLE          RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE)

/* SPI1 PIN */
#define SPI1_GPIO_PORT                  GPIOA
#define SPI1_GPIO_PIN                   GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7

/**************************************** W5500_1 ****************************************/
/* W5500片选 */
#define W5500_CS_PORT				    GPIOA
#define W5500_CS_PIN				    GPIO_Pin_4
#define W5500_CS_LOW				    GPIO_ResetBits(W5500_CS_PORT, W5500_CS_PIN)
#define W5500_CS_HIGH				    GPIO_SetBits(W5500_CS_PORT, W5500_CS_PIN)

/* W5500复位 */
#define W5500_RST_PORT				    GPIOC
#define W5500_RST_PIN 				    GPIO_Pin_5
#define W5500_RST_ENABLE                GPIO_ResetBits(W5500_RST_PORT, W5500_RST_PIN);
#define W5500_RST_DISABLE               GPIO_SetBits(W5500_RST_PORT, W5500_RST_PIN);

/* W5500中断 */
#define W5500_INT_PORT				    GPIOC
#define W5500_INT_PIN				    GPIO_Pin_4



/**************************************** SPI2 ****************************************/
/* GPIO Clock */
#define SPI2_PERIPH_RELATE1             RCC_APB2Periph_GPIOB

/* SPI2 PIN */
#define SPI2_GPIO_PORT                  GPIOB
#define SPI2_GPIO_PIN                   GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15

/**************************************** W5500_2 ****************************************/
///* W5500片选 */
//#define W5500_CS_PORT				    GPIOB
//#define W5500_CS_PIN				    GPIO_Pin_12
//#define W5500_CS_LOW				    GPIO_ResetBits(W5500_CS_PORT, W5500_CS_PIN)
//#define W5500_CS_HIGH				    GPIO_SetBits(W5500_CS_PORT, W5500_CS_PIN)

///* W5500复位 */
//#define W5500_RST_PORT				    GPIOD
//#define W5500_RST_PIN 				    GPIO_Pin_3
//#define W5500_RST_LOW                   GPIO_ResetBits(W5500_RST_PORT, W5500_RST_PIN);
//#define W5500_RST_HIGH                  GPIO_SetBits(W5500_RST_PORT, W5500_RST_PIN);

///* W5500中断 */
//#define W5500_INT_PORT				    GPIOA
//#define W5500_INT_PIN				    GPIO_Pin_1


typedef union {
u8 Byte;
struct {
		u8  bit0             :1;
		u8  bit1             :1;
		u8  bit2             :1;
		u8  bit3             :1;
		u8  bit4             :1;
		u8  bit5             :1;
		u8  bit6             :1;
		u8  bit7             :1;
} Bits;
}Tdef_Byte;



#endif


