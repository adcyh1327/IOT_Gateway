#ifndef __VARIABLE_H_
#define __VARIABLE_H_

#include "wizchip_conf.h"
#include "ucos_ii.h"
#include "MQTTPacket.h"

enum SessionMode//以太网工作模式
{
    S_mqtt=0,S_tcpip_client,S_tcpip_server,S_mb_client,S_mb_server,//后边若有需求，在此往后增加其他工作模式
    S_boundary//边界值，保持在最后，用于程序中识别当前支持的模式数量
};


struct EthernetCfg_t
{
    u8  cfgflag[4];                       //存放固定的0xAA55AA55，用于识别flash数据是否有效
    u8  session_mode;                     //以太网模式模式
    u16 stationID;                        //MQTT节点ID或MB站地址
    u8  reserve1[1];
    u16 mbtcp_addr;                       //modbus起始地址，大端
    u16 mbtcp_datalen;                    //modbus数据长度，大端
    u16 polltime;                         //modbus轮询时间，大端，单位：1ms
    u8  reserve2[6];
    u8  localIP[4];                       //本地IP地址
    u16 localport;                        //本地端口号，大端
    u8  reserve3[2];
    u8  remoteIP[4];                      //目标IP地址
    u16 remoteport;                       //目标端口号，大端
    u8  reserve4[2];
    u8  submask[4];                       //子网掩码
    u8  gatewayaddr[4];                   //网关地址
    u8  reserve5[4];
    u8  can_en;                           //CAN接口使能
    u8  canbaudrate;                      //CAN波特率，详见配置表
    u16 can_localID;                      //本地CAN-ID，大端
    u16 can_deviceID;                     //响应CAN-ID，大端
    u8  can_device_num;                   //响应CAN-ID的数量
    u8  can_datatype; 
    u8  rs232_1_en;                       //RS232C-1接口使能
    u8  rs232_1_baudrate;                 //RS232C-1接口波特率
    u8  rs232_1_databit;                  //RS232C-1接口数据位
    u8  rs232_1_chkbit;                   //RS232C-1接口校验位
    u8  rs232_1_stopbit;                  //RS232C-1接口停止位
    u8  rs232_1_flowctrl;                 //RS232C-1接口流控
    u8  rs232_1_datatype;                 //RS232C-1数据格式
    u8  reserve7[1];
    u8  rs232_2_en;                       //RS232C-2接口使能
    u8  rs232_2_baudrate;                 //RS232C-2接口波特率
    u8  rs232_2_databit;                  //RS232C-2接口数据位
    u8  rs232_2_chkbit;                   //RS232C-2接口校验位
    u8  rs232_2_stopbit;                  //RS232C-2接口停止位
    u8  rs232_2_flowctrl;                 //RS232C-2接口流控
    u8  rs232_2_datatype;                 //RS232C-2数据格式
    u8  reserve8[1];
    u8  rs485_en;                         //RS485接口使能
    u8  rs485_baudrate;                   //RS485接口波特率
    u8  rs485_databit;                    //RS485接口数据位
    u8  rs485_chkbit;                     //RS485接口校验位
    u8  rs485_stopbit;                    //RS485接口停止位
    u8  rs485_datatype;                   //RS485数据格式
    u8  reserve9[1];
    u8  to_thres;                         //串口通讯中帧内相邻字符间隔超时时间
};


enum UsartType{//串口配置数组各元素的定义，有增加向时请在下边第一行往后增加
    EnUart=0,uartBaudrate,Databits,Chkbits,Stopbits,Flowctrl,uartDatatype,
    uartcfgnum
};

enum CANType{//CAN的配置数组各元素定义，有增加向时请在下边第一行往后增加
    EnCAN=0,canBaudrate,LocalID,DeviceID,IDNum,canDatatype,
    cancfgnum
};

enum DataType_t{//数据类型，有增加向时请在下边第一行往后增加
    t_HEX=0,t_ASCII,t_BOOL,
    t_typemax
};
extern char DataType[t_typemax][20];

#define NUM_UARTCHANNEL               3  //串口总通道
#define SCI_BUF_MAXLEN              256  //串口发送、接收缓冲区的最大长度,根据需要可能会变更

enum USARTCAN_CHN{//串口和CAN的通道编号
    RS232_1=0,RS232_2,RS485,CAN_CHN,
    NUM_UARTCAN
};

/*********************************************************************************************/   
/*********************************************************************************************/ 
/*********************************************************************************************/    
typedef union {
  u16 T_byte;
  struct {
		u8  btn              :4;   //串口帧头或帧尾的字节个数
		u8  bit4             :1;
		u8  bit5             :1;
		u8  bit6             :1;
		u8  en               :1;   //串口帧头或帧尾标志
        u16 reserve          :8;   
	} Bits;
}Tdef_Prot;
#define FrameStartEn                  0x80u  //帧头使能宏
#define FrameEndEn                    0x80u  //帧尾使能宏
enum numbyte{
  byte_1=1,byte_2,byte_3 ,byte_4 ,byte_5 ,byte_6 ,byte_7 ,byte_8 ,byte_9,byte_10     //帧头或帧尾的字节数量
};
enum checksum_t{//校验和方式
    CheckSum_None=0,ChkSum_And,ChkSum_Crc16
};
/*********************************************************************************************/ 
/*********************************************************************************************/ 
/*********************************************************************************************/ 

struct ProtType_t
{
    Tdef_Prot FrameStartInfo;//最高位表示是否使能，低4位表示字节数，只有使能了字节数才有效
    u8 FrameStart[8];        //帧开始符字节数
    Tdef_Prot FrameEndInfo; //最高位表示是否使能，低4位表示字节数，只有使能了字节数才有效
    u8 FrameEnd[8];         //帧结束符字节数
    u8 checksum;          //校验(0-无，1-和，2-crc16)
};

typedef struct Interface_Info_t
{
    unsigned char Usart[NUM_UARTCHANNEL][uartcfgnum];  //串口配置数组
    const struct ProtType_t UsartProt[NUM_UARTCHANNEL];//串口帧头帧尾配置
    unsigned int can[cancfgnum];    //CAN配置数组
    unsigned int  addr;//modbus起始地址
    unsigned int  datalen;//每个通道mb的长度，实际长度为此值-2，因为有两个字为协议内容，一个为标志位，第二个为字节数量
    unsigned char  sid;//站地址
    unsigned char  tout;//串口通讯帧超时时间
}Interface_Info;
extern Interface_Info USARTCAN;//

extern wiz_NetInfo gWIZNETINFO;
extern wiz_NetTimeout gWIZNetTimeout;



struct USARTCAN_Recv_t
{
	u8 newupd;//数据更新标志位
    u16 lenth; //字节数量
    u8 datatype;//数据类型
    u8 databuf[SCI_BUF_MAXLEN];//有效数据
};
extern struct USARTCAN_Recv_t USARTCAN_Recv[NUM_UARTCAN];

extern u16 g_u16_TCPIPsendlen;           //tcpip报文发送长度

extern u16 cpu_sr;                        //cpu中断状态

extern char platform_version[];
extern char funcTion[];
extern const u32 RS232_baud[12] ;
extern const u16 RS232_lenth[2] ;
extern const u16 RS232_stop[2] ;
extern const u16 RS232_parity[3] ;
extern const u16 RS232_FlowCntl[4] ;

unsigned char AscToHex(unsigned char aChar);
unsigned char HexToAsc(unsigned char aHex);

#endif


