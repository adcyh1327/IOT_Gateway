#ifndef __TASK_MAIN_H
#define __TASK_MAIN_H


/************设置任务优先级,建议(10-50) *************************/
/* #define OS_LOWEST_PRIO   63u */
#define TASK_PRIO_MAIN                           2u

#define TASK_PRIO_MBTCP_SEND                    10u
#define TASK_PRIO_TCPIP_SEND                    11u
#define	TASK_PRIO_MQTTPUB                       12u
#define	TASK_PRIO_MQTTSUB                       13u
#define	TASK_PRIO_MQTT                          9u
#define TASK_PRIO_TCPIP                         21u
#define TASK_PRIO_MBTCP                         22u
#define TASK_PRIO_HTTP                          23u

#define TASK_PRIO_MODBUSRTU                     30u
#define TASK_PRIO_IO                            31u
#define TASK_PRIO_FLASH                         32u
#define TASK_PRIO_LEDR                          40u
#define TASK_PRIO_LEDG                          41u

#define TASK_PRIO_BACKGRD                       60u


/************设置栈大小（单位为 OS_STK ）************/
#define STK_SIZE_32                             32u
#define STK_SIZE_64                             64u
#define STK_SIZE_128                            128u
#define STK_SIZE_256                            256u
#define STK_SIZE_384                            384u
#define STK_SIZE_512                            512u
#define STK_SIZE_640                            640u
#define STK_SIZE_768                            768u
#define STK_SIZE_896                            896u
#define STK_SIZE_1024                           1024u
#define STK_SIZE_2048                           2048u
#define STK_SIZE_3072                           3072u
#define STK_SIZE_4096                           4096u



void Task_Main(void *p_arg);
void ETH2Usartcan_send(u8 uartcan_chn,u8 *databuf,u16 lenth);
void HTTP_DataHandler(void);
void ReadFlashCfg(void);

#endif


