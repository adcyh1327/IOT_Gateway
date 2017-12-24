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
#define MODBUS_CONFIG_BASEADDR                              0x803A000ul    //�洢�����ļ�����ʼ��ַ
#define MODBUS_READREG_BASEADDR   MODBUS_CONFIG_BASEADDR+FLASH_PAGESIZE    //
#define CONFIG_PAGENUMBER                                            8U    //������������С
#define PROTOCAL_WRITE                              MAXNUM_READPROTOCAL    //д����������
#define CFGAREA_UNIT                                                 64    //ÿ��������ռ�ֽڴ�С
#define REG_ADDRSIZE                                                 34    //����֡���ַ���ֽ�����

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

#define FUNC_RESP_POS                          0x55U      //�϶���Ӧ
#define FUNC_RESP_NEG                          0x80U      //����Ӧ
#define FUNC_RD_COILSTATUS                     0x01U      //����Ȧ״̬
#define FUNC_RD_INPUTSTATUS                    0x02U      //������״̬
#define FUNC_RD_HOLDREG                        0x03U      //�����ּĴ���
#define FUNC_WR_SGCOIL                         0x05U      //д������Ȧ
#define FUNC_WR_SGREG                          0x06U      //д�����Ĵ���
#define FUNC_WR_MULCOIL                        0x0FU      //
#define FUNC_WR_MULREG                         0x10U      //д����Ĵ���

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
extern enum CommType CommType; //Э������

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
#define MB_ReadVersion                     _SystemFlag.Bits.bit3    //���汾��Ϣ
#define ReadMulRegdata                     _SystemFlag.Bits.bit4    //0-����������֡��Ϣ��1-һ�ζ������Ϣ
#define MB_ResptoPC                        _SystemFlag.Bits.bit7    //�忨��һ���������Ӧ�������־
#define SPEC_RESPONDSE                     _SystemFlag.Bits.bit8    //������Ҫ���п϶���Ӧ�����Ӧ���ظ�֡��ʽ��ͬ
#define MB_EXECUTESTATUS                   _SystemFlag.Bits.bit9    //�϶������Ӧ״̬
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
#define ThreadNew(n)                       _NodeFlag[n].Bits.bit0    //���ڵ��ȶ��Ĵ�����־
#define GW_ReadStatus(n)                   _NodeFlag[n].Bits.bit1    //�忨�ɹ����յ��豸����Ӧ
#define MB_ReadRegVal(n)                   _NodeFlag[n].Bits.bit3    //һ������Ͷ��Ĵ���ֵ�����־
#define MB_NeedWrite(n)                    _NodeFlag[n].Bits.bit4    //һ�������д���������־
#define GW_WriteStatus(n)                  _NodeFlag[n].Bits.bit5    //�忨д����ִ��״̬
#define RecvFrameStart(n)                  _NodeFlag[n].Bits.bit8    //����֡��ʼ��־

#define READ_IDLE                          0U 
#define READ_WAITRESPOND                   1U
#define READ_RECVSUCCESS                   2U

#define WRITE_IDLE                         0U                       //GW_SendStatus״̬λ,����
#define WRITE_PRESEND                      1U                       //GW_SendStatus״̬λ,��ʼ��������
#define WRITE_WAITRESPOND                  2U                       //GW_SendStatus״̬λ,���ݷ������
#define WRITE_RECVSUCCESS                  3U                       //GW_SendStatus״̬λ,�յ��豸��Ӧ
#define WRITE_DELAY                        4U                       //GW_SendStatus״̬λ,�ӳٽ���
#define PLCRESPONSE                        5U                       //PPIʹ�ã�PLC������Ӧ

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

extern struct RegisterCfgType       RegisterCfgBuff[MAXNUM_NODE][MAXNUM_READPROTOCAL];//��״̬�Ĵ洢������
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
extern struct CommParmCfgType CommParmCfg;                       //ͨѶ��ز���
#define ProtocalType          CommParmCfg.ProtType               //Э�����ͣ�0-232,1-485,2-CAN��3-ETH
#define Uart_DataBits         CommParmCfg.Bits.databit           //����ͨѶ����λ����
#define Uart_ParityChk        CommParmCfg.Bits.Paritybit         //����ͨѶ��żУ�飬0-��,1-��,2-ż
#define Uart_Stopbit          CommParmCfg.Bits.stopbit           //����ͨѶֹͣλ��1-1λ��2-2λ

extern u16 g_u16_ETH_Timeout;
extern u16 DetectTime;
extern u8  g_u8_ProtocalNum[MAXNUM_NODE][2];                                 //�豸���õĶ�Э����Ŀ
extern u8  g_u8_threadIdx[MAXNUM_NODE];
extern u16 AD_Result[6];
extern u8  CommPort;
extern u8  g_u8_RespondID;                                        //�忨��Ӧ��ID
extern u8  g_u8_RespondNode;
extern u8  g_u8_RespondSocket;
extern u8  g_u8_ReqCfgNum;//���ζ�ȡ����֡������
extern u8  g_u8_Writedata[256];
extern u16 g_u16_TimeoutCnt[MAXNUM_NODE][MAXNUM_READPROTOCAL];
extern u8  g_u8_EthRespData[MAXNUM_NODE][MAXNUMM_REGCFG];
extern u8  g_u8_EthRecvData[MAXNUM_ETHRECVDATA];	 
extern u8  g_u8_EthTransData[MAXNUM_BOARDSENDDATA];
extern u8  g_u8_EthTransLen;
extern u16 g_u16_RecvTransLen[MAXNUM_NODE][MAXNUM_READPROTOCAL];
extern u8  g_u8_FunCode;                    //��ǰһ������͵Ĺ�����
extern u8  g_u8_CfgCounter;                 //����֡����
extern u16 g_u16_SwitchTimer[MAXNUM_NODE];
extern u16 g_u16_StartAddr[MAXNUM_NODE][MAXNUM_READPROTOCAL];
extern u16 g_u16_ReadLen;  //������
extern u16 g_u16_ReadAddr;  //����ַ
extern u8  g_u8_WriteAddr[REG_ADDRSIZE];//д��ַ
extern u8  g_u8_WriteAddrOffset;//д��ַƫ��
extern u16 g_u16_WriteLen;  //д����
extern u8  GatewayStationAddr;                 //�忨վ��ַ
extern u16 UpdateCycle[MAXNUM_NODE];
extern u16 WriteTime;
extern u8  TCPcount[2];
extern u8  g_u8_HardwareVer;
extern u8  g_u8_DialSwitch;
extern u8  UpdateTime;    //ģ�������������Ĳɼ�ʱ���ʱ
extern u8  CodeVal;  //���뿪��ֵ
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
