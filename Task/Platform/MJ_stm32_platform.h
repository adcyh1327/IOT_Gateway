#ifndef MJ_STM32_PLATFORM_H
#define MJ_STM32_PLATFORM_H

#include "stm32f10x.h"
#include "MJ_stm32_platform_cfg.h" 
#include "m_flash.h" 
#include "mes.h" 
#include <string.h>
#include <stddef.h>

#define APP_CRC_ADDR                                          0x8024000
#define FLASH_PAGESIZE                                            0x800    //     
#define MODBUS_CONFIG_BASEADDR                              0x803A000ul    //存储配置文件的起始地址
#define MODBUS_READREG_BASEADDR   MODBUS_CONFIG_BASEADDR+FLASH_PAGESIZE    //
#define CONFIG_PAGENUMBER                                            8U    //配置数据区大小
#define PROTOCAL_WRITE                              MAXNUM_READPROTOCAL    //写命令索引号
#define CFGAREA_UNIT                                                 64    //每个配置所占字节大小
#define REG_ADDRSIZE                                                 34    //配置帧里地址域字节数量

typedef void (*FUNC) (u8,u8);
#define USER_DEFINED_VAL(n)   g_u8_EthRespData[NODE_USER_DEFINED][n]
#define DI_STATUS(in,out)    (out= in>=4 ? 0:1)

#define GREEN_LED_OFF 		GPIO_SetBits(GPIOB , GPIO_Pin_0);
#define GREEN_LED_ON 		  GPIO_ResetBits(GPIOB , GPIO_Pin_0);

#define YELLOW_LED_OFF 		GPIO_SetBits(GPIOB , GPIO_Pin_1);
#define YELLOW_LED_ON 		GPIO_ResetBits(GPIOB , GPIO_Pin_1);

#define ON                                        1U      //
#define OFF                                       0U      //
#define READ                                      0U  
#define WRITE                                     1U

#define TYPE_BIT                                  1U
#define TYPE_BYTE                                 8U
#define TYPE_WORD                                16U
#define TYPE_DWORD                               32U

#define FUNC_RESP_POS                          0x55U      //肯定响应
#define FUNC_RESP_NEG                          0x80U      //否定响应
#define FUNC_RD_COILSTATUS                     0x01U      //读线圈状态
#define FUNC_RD_INPUTSTATUS                    0x02U      //读输入状态
#define FUNC_RD_HOLDREG                        0x03U      //读保持寄存器
#define FUNC_WR_SGCOIL                         0x05U      //写单个线圈
#define FUNC_WR_SGREG                          0x06U      //写单个寄存器
#define FUNC_WR_MULCOIL                        0x0FU      //
#define FUNC_WR_MULREG                         0x10U      //写多个寄存器

enum MB_TCP_ErrorStatusTyp{
  Sta_OK=0,
  Err_FunCode,
  Err_DataAddr,
  Err_DataValue,
  Err_MBcmd,
  Err_Busy,
  Err_CfgUnmatch=0x0A, 
  Err_FlashWrFail, 
};

extern enum MB_TCP_ErrorStatusTyp MB_TCP_ErrSta;

enum CommType{
  Comm_RS232,
  Comm_RS485,
  Comm_CAN,
  Comm_ETH
};
extern enum CommType CommType; //协议类型

typedef union {
  u32 DWord;
  struct {
		u8  bit0             :1;
		u8  bit1             :1;
		u8  bit2             :1;
		u8  bit3             :1;
		u8  bit4             :1;
		u8  bit5             :1;
		u8  bit6             :1;
		u8  bit7             :1;
		u8  bit8             :1;
		u8  bit9             :1;
		u8  bit10            :1;
		u8  bit11            :1;
		u8  bit12            :1;
		u8  bit13            :1;
		u8  bit14            :1;
		u8  bit15            :1;
		u8  bit16            :1;
		u8  bit17            :1;
		u8  bit18            :2;
		u8  bit20            :4;
		u8  bit24            :8;
	} Bits;
}Tdef_DWord;
extern  volatile  Tdef_DWord               _SystemFlag;
#define SystemFlag                         _SystemFlag.DWord
#define GW_CfgComplete                     _SystemFlag.Bits.bit0    //
#define MB_ReadVersion                     _SystemFlag.Bits.bit3    //读版本信息
#define ReadMulRegdata                     _SystemFlag.Bits.bit4    //0-读单个配置帧信息，1-一次读多个信息
#define MB_ResptoPC                        _SystemFlag.Bits.bit7    //板卡向一体机发送响应的请求标志
#define SPEC_RESPONDSE                     _SystemFlag.Bits.bit8    //表明需要进行肯定响应或否定响应，回复帧格式不同
#define MB_EXECUTESTATUS                   _SystemFlag.Bits.bit9    //肯定或否定响应状态
#define Upload_data                        _SystemFlag.Bits.bit18   //

#define UP_IDLE                            0u
#define UP_SUCCESS                         1u
#define UP_FAIL                            2u


typedef union {
  u16 T_word;
  struct {
		u8  bit0             :1;
		u8  bit1             :2;
		u8  bit3             :1;
		u8  bit4             :1;
		u8  bit5             :3;
		u8  bit8             :1;
		u8  bit9             :1;
		u8  bit10            :1;
		u8  bit11            :1;
		u8  bit12            :1;
		u8  bit13            :1;
		u8  bit14            :1;
		u8  bit15            :1;
	} Bits;
}Tdef_Word;

extern  volatile  Tdef_Word                _NodeFlag[MAXNUM_NODE];
#define NodeFlag(n)                        _NodeFlag[n].T_word
#define ThreadNew(n)                       _NodeFlag[n].Bits.bit0    //周期调度读寄存器标志
#define GW_ReadStatus(n)                   _NodeFlag[n].Bits.bit1    //板卡成功接收到设备的响应
#define MB_ReadRegVal(n)                   _NodeFlag[n].Bits.bit3    //一体机发送读寄存器值命令标志
#define MB_NeedWrite(n)                    _NodeFlag[n].Bits.bit4    //一体机发送写命令请求标志
#define GW_WriteStatus(n)                  _NodeFlag[n].Bits.bit5    //板卡写命令执行状态
#define RecvFrameStart(n)                  _NodeFlag[n].Bits.bit8    //接收帧起始标志

#define READ_IDLE                          0U 
#define READ_WAITRESPOND                   1U
#define READ_RECVSUCCESS                   2U

#define WRITE_IDLE                         0U                       //GW_SendStatus状态位,空闲
#define WRITE_PRESEND                      1U                       //GW_SendStatus状态位,开始启动发送
#define WRITE_WAITRESPOND                  2U                       //GW_SendStatus状态位,数据发送完成
#define WRITE_RECVSUCCESS                  3U                       //GW_SendStatus状态位,收到设备响应
#define WRITE_DELAY                        4U                       //GW_SendStatus状态位,延迟结束
#define PLCRESPONSE                        5U                       //PPI使用，PLC允许响应

struct RegisterCfgType
{
  union 
  {
    u8 bytetype;
    struct {
  		u8  fbrd             :1;
  		u8  bit1             :1;
  		u8  bit2             :1;
  		u8  bit3             :1;
  		u8  nodenum          :4;
  	}Bits;
  }cmdstatus;
  u8 regtype;
  u8 operatesize;
  u8 datalenth;
  u8 plcaddrstation;
  u8 specfuncode;
  u8 regaddr[REG_ADDRSIZE];
};

extern struct RegisterCfgType       RegisterCfgBuff[MAXNUM_NODE][MAXNUM_READPROTOCAL];//读状态的存储缓冲区
#define ReadRegForbid(m)(n)         RegisterCfgBuff[m][n].cmdstatus.Bits.fbrd
#define ReadNode(m)(n)              RegisterCfgBuff[m][n].cmdstatus.Bits.nodenum
#define RegOpSize(m)(n)             RegisterCfgBuff[m][n].operatesize
#define RegisterType(m)(n)          RegisterCfgBuff[m][n].regtype
#define ReadDataLen(m)(n)           RegisterCfgBuff[m][n].datalenth
#define RegisterAddr(m)(n)          RegisterCfgBuff[m][n].regaddr

struct CommParmCfgType
{
  u8 ProtType;
  u16 databit;
  u16 paritybit;
  u16 stopbit;
  u16 flowctrl;
  /*union 
  {
    u8 commparm;
    struct {
  		u8  databit          :2;
  		u8  Paritybit        :2;
  		u8  stopbit          :2;
  		u8  flowctrl         :1;
  	}Bits;
  }commstatus;*/
  u32 baudrate;
};
extern struct CommParmCfgType CommParmCfg;                       //通讯相关参数
#define ProtocalType          CommParmCfg.ProtType               //协议类型，0-232,1-485,2-CAN，3-ETH
#define Uart_DataBits         CommParmCfg.Bits.databit           //串口通讯数据位数量
#define Uart_ParityChk        CommParmCfg.Bits.Paritybit         //串口通讯奇偶校验，0-无,1-奇,2-偶
#define Uart_Stopbit          CommParmCfg.Bits.stopbit           //串口通讯停止位，1-1位，2-2位

extern u16 g_u16_ETH_Timeout;
extern u16 DetectTime;
extern u8  g_u8_ProtocalNum[MAXNUM_NODE][2];                                 //设备配置的读协议数目
extern u8  g_u8_threadIdx[MAXNUM_NODE];
extern u16 AD_Result[6];
extern u8  CommPort;
extern u8  g_u8_RespondID;                                        //板卡响应的ID
extern u8  g_u8_RespondNode;
extern u8  g_u8_RespondSocket;
extern u8  g_u8_ReqCfgNum;//单次读取配置帧的数量
extern u8  g_u8_Writedata[256];
extern u16 g_u16_TimeoutCnt[MAXNUM_NODE][MAXNUM_READPROTOCAL];
extern u8  g_u8_EthRespData[MAXNUM_NODE][MAXNUMM_REGCFG];
extern u8  g_u8_EthRecvData[MAXNUM_ETHRECVDATA];	 
extern u8  g_u8_EthTransData[MAXNUM_BOARDSENDDATA];
extern u8  g_u8_EthTransLen;
extern u16 g_u16_RecvTransLen[MAXNUM_NODE][MAXNUM_READPROTOCAL];
extern u8  g_u8_FunCode;                    //当前一体机发送的功能码
extern u8  g_u8_CfgCounter;                 //配置帧计数
extern u16 g_u16_SwitchTimer[MAXNUM_NODE];
extern u16 g_u16_StartAddr[MAXNUM_NODE][MAXNUM_READPROTOCAL];
extern u16 g_u16_ReadLen;  //读长度
extern u16 g_u16_ReadAddr;  //读地址
extern u8  g_u8_WriteAddr[REG_ADDRSIZE];//写地址
extern u8  g_u8_WriteAddrOffset;//写地址偏移
extern u16 g_u16_WriteLen;  //写长度
extern u8  GatewayStationAddr;                 //板卡站地址
extern u16 UpdateCycle[MAXNUM_NODE];
extern u16 WriteTime;
extern u8  TCPcount[2];
extern u8  g_u8_HardwareVer;
extern u8  g_u8_DialSwitch;
extern u8  UpdateTime;    //模拟量、数字量的采集时间计时
extern u8  CodeVal;  //拨码开关值
extern u8  g_u8_ProtocalID[MAXNUM_NODE];
extern u8 Write_FuncCode;


u8 DEC2HEX(u8 val);
u8 ASCII2BYTE(u8 high,u8 low);
u8 HEXtoASCII(u8 l_u8HEX);
u8 ASCIItoHEX(u8 l_u8ASCII);
u8 Count_bit1(u8 data);
u8 BCC_CalcCheck(u8 *data,u8 len);
void AndCalc_Check(u8 *l_u8ArrCheckData, u8 l_u8CheckDataLen, u8 *pl_u8CheckH, u8 *pl_u8CheckL);
u8 ProgramCongfigData(u8 lastcfg,u8 *data);
void Platform_Init(u8 stationaddr,u16 wtcyc);
void f_Acquisition_Task(void);
void ReadAllCongfigData(void);
void creat_list(void);
void f_GenSoftwareReset(void);
void PositiveResponse(void);
void f_MBTCP_Transmit(u8 *tcpsn);
void NegativeResponse(u8 errortype);
void Read_Version(u8 vercode);
void TCP_Recv_EthData(u8 socket,u8 *Ethrecdata, u8 lenth);
#endif
