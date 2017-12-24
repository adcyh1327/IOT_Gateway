#include "main.h"
#include "httputil.h"

#ifdef __VARIABLE_H_

char platform_version[] = {"MJ-Trans-V1.03 20170405"};
char funcTion[] = {"明匠智能-协议转换器通用版"};

const  u32   APPL_CRC __attribute__((at(APP_CRC_ADDR)))={0xA1A2A3A4};

//网络参数配置
wiz_NetInfo gWIZNETINFO = { 
                            .session_mode = S_mqtt,//以太网工作模式
                            .conn_status = OFF,//socket连接状态
                            .stationID = 1,//modbus站地址，或MQTT时对应的本节点ID
                            .mbtcp_addr = 0x0,//mb配置的起始地址
                            .polltime = 1000,//做mb client时向server周期发送读请求
                            .tcpsocket = SOCK_MQTT,//socket 
                            .mac = {0x48, 0x53, 0x00, 0x00, 0xab, 0xcd},//mac地址，最后3位依次为IP的后3个字节
                            .iplocal = {192, 168, 88, 60},//本地IP地址
                            .portlocal=PORT_LOCAL,//本地端口号，默认为5000
                            .sn = {255, 255, 255, 0},//子网掩码
                            .gw = {192, 168, 88, 1},//网关地址
                            .dns = {0, 0, 0, 0},
                            .dhcp = NETINFO_STATIC,
                            .ipgoal = {192, 168, 88, 76}, //目标IP地址
                            .portgoal=1883,//目标端口号
                           }; //目标端口

wiz_NetTimeout gWIZNetTimeout =
                           {
                               .retry_cnt = 2,         //重试次数
                               .time_100us = 25000     //重试2次，每次间隔2.5s
                           };

                           
Interface_Info USARTCAN = { 
                            .Usart[RS232_1] = {1,6, 0, 0, 0, 0,0}, //RS232  // USER 19200 8 ODD 1 CTS 
                            .Usart[RS232_2] = {1,6, 0, 0, 0, 0,0},
                            .Usart[RS485] =   {1,6, 0, 0, 0, 0,0},
                            .UsartProt[RS232_1]=
                            {
                                .FrameStartInfo = 0,//FrameStartEn|byte_1,  //本配置为0时则为无协议，所有数据均为有效数据，当字节间时间超过配置时间后认为是完整一帧
                                .FrameStart = {0x02},
                                .FrameEndInfo = 0,//FrameStartEn|byte_2,//同帧头配置类似，前边的宏使能才表示有帧尾，后边一个为字节数量
                                .FrameEnd = {0x0d,0x0a,},
                                .checksum = CheckSum_None//当前是0,根据实际需要可能要更改
                            },
                            .UsartProt[RS232_2]=
                            {
                                .FrameStartInfo = 0,//FrameStartEn|byte_1,
                                .FrameStart = {0x02},
                                .FrameEndInfo = 0,//FrameStartEn|byte_2,
                                .FrameEnd = {0x0d,0x0a,},
                                .checksum = CheckSum_None
                            },
                            .UsartProt[RS485]=
                            {
                                .FrameStartInfo = 0,//FrameStartEn|byte_1,
                                .FrameStart = {0x02},
                                .FrameEndInfo = 0,//FrameStartEn|byte_2,
                                .FrameEnd = {0x0d,0x0a,},
                                .checksum = CheckSum_None
                            },
                            .can = {1,3,0x700,0x701,1,0},
                            .datalen=50,//串口或CAN的每个通道传输数据的字长度
                            .tout = 12,  //串口帧传输过程中字节间时间的最大允许间隔
                          };

                          
const u32 RS232_baud[12] = {25600,12800,115200,57600,56000,38400,19200,14400,9600,4800,2400,1200};//串口波特率配置
const u16 RS232_lenth[2] = {USART_WordLength_8b,USART_WordLength_9b};//数据位配置
const u16 RS232_parity[3] = {USART_Parity_No,USART_Parity_Odd,USART_Parity_Even};//串口校验位配置
const u16 RS232_stop[2]  ={USART_StopBits_1,USART_StopBits_2};//停止位配置
const u16 RS232_FlowCntl[4] = {USART_HardwareFlowControl_None,USART_HardwareFlowControl_RTS_CTS,};//流控配置
const u16 CAN_baud[14] ={0,1,2,3,4,5,6,7,8,9,10,11,12,13};//{1000,800,666,500,400,250,200,125,100,80,50,40,20,10};//CAN波特率配置

struct USARTCAN_Recv_t USARTCAN_Recv[NUM_UARTCAN];//串口或CAN的接收备份缓冲区，识别到有效帧后存取，可直接拷贝至以太网缓冲区


char DataType[t_typemax][20]=//数据类型，主要用于MQTT的JSON串格式的数据
{
    {"t_HEX"},
    {"t_ASCII"},
    {"t_BOOL"},
};



u16 g_u16_TCPIPsendlen;           //tcpip报文发送长度

u16 cpu_sr;                        //cpu中断状态

u8 Check_XOR(u8 *data,u8 lenth)//异或校验
{
    u8 i,temp;
    temp=0;
    for(i=0;i<lenth;i++)
    {
        temp ^= data[i];
    }
    return temp;
}

unsigned char AscToHex(unsigned char aChar)//ASCII转hex，正常输出范围0x00-0x0F，输入范围为0x30-0x39,0x41-0x46，0x61-0x66
{
    unsigned char ret;
    if((aChar >= 0x30)&&(aChar <= 0x39))
    {
        ret = aChar - 0x30;
    }
    else if((aChar >= 0x41)&&(aChar <= 0x46))
    {
        ret = aChar - 0x37;
    }
    else if((aChar >= 0x61)&&(aChar <= 0x66))
    {
       ret = aChar - 0x57;
    }
    else
    {        
       ret = 0xFF;
    }
    return ret;
}

unsigned char HexToAsc(unsigned char aHex)//HEX转ASCII，正常输出范围0x30-0x39,0x41-0x46,输入范围为0x00-0x0F
{
    unsigned char ret;
    if(aHex <=9 )
    {
        ret = aHex + 0x30;
    }
    else if((aHex >= 10)&&(aHex <= 15)) 
    {
        ret =  aHex + 0x37;
    }
    else
    {
        ret = 0xFF;
    }
        return ret;
}





#endif









