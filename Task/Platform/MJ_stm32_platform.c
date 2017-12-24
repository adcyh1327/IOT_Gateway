
#include "stdlib.h"
#include "stdio.h"
//#include<malloc.h>  
#include "main.h"
#include "m_w5500.h"

/* flash */	   			
const  u32   APPL_CRC __attribute__((at(APP_CRC_ADDR)))={0xA1A2A3A4}; 

volatile  Tdef_DWord                     _SystemFlag;
volatile  Tdef_Word                     _NodeFlag[MAXNUM_NODE];
enum MB_TCP_ErrorStatusTyp              MB_TCP_ErrSta;
enum CommType CommType; //协议类型
enum ProtocalIndexTyp ProtocalIndex;
struct RegisterCfgType                  RegisterCfgBuff[MAXNUM_NODE][MAXNUM_READPROTOCAL];

u16 g_u16_ETH_Timeout;
u8  CommPort;
u8  IO_AD_Node;
u8  g_u8_ProtocalNum[MAXNUM_NODE][2];                                 //设备配置的协议数目
u8  g_u8_threadIdx[MAXNUM_NODE];
u8  g_u8_RespondID;                                         //板卡响应的ID
u8  g_u8_RespondSocket;
u8  g_u8_RespondNode;
u8  g_u8_ReqCfgNum;//单次读取配置帧的数量
u8  g_u8_EthRecvData[MAXNUM_ETHRECVDATA];
u8  g_u8_EthTransData[MAXNUM_BOARDSENDDATA];
u8  g_u8_EthTransLen;
u8  g_u8_Writedata[256];
u16 g_u16_TimeoutCnt[MAXNUM_NODE][MAXNUM_READPROTOCAL];
u8  g_u8_EthRespData[MAXNUM_NODE][MAXNUMM_REGCFG];
u16 g_u16_RecvTransLen[MAXNUM_NODE][MAXNUM_READPROTOCAL];
u8  g_u8_FunCode;                    //当前一体机发送的功能码
u8  g_u8_CfgCounter;                 //配置帧计数
u16 g_u16_SwitchTimer[MAXNUM_NODE]; 	
u16 g_u16_StartAddr[MAXNUM_NODE][MAXNUM_READPROTOCAL];
u16 g_u16_ReadAddr;  //读地址
u16 g_u16_ReadLen;  //读长度
u8  g_u8_WriteAddr[REG_ADDRSIZE];//写地址
u8  g_u8_WriteAddrOffset;//写地址偏移
u16 g_u16_WriteLen;  //写长度
u8  GatewayStationAddr;                 //板卡站地址
u16 UpdateCycle[MAXNUM_NODE];
u16 WriteTime;
u8  TCPcount[2];
u8  g_u8_HardwareVer;
u8  g_u8_DialSwitch;
u8  UpdateTime;
u8  SampleIndex;
u16 Input_Result[6];
u8  Input_Status[6];
u16 AD_Result[6];
u8  AD_Status[6];
u8  CodeVal = 0x00;  //拨码开关值
u16 g_TimeCount;	//时间计时
u8  g_u8_ProtocalID[MAXNUM_NODE];
u8 Write_FuncCode;
u16 DetectTime;

struct CommParmCfgType CommParmCfg =
{
  Comm_RS232,
  USART_WordLength_8b,
  USART_Parity_No,
  USART_StopBits_1,
  USART_HardwareFlowControl_None,
  19200
};            //通讯相关参数

u8 DEC2ASCII(u8 val)
{
  if(val>9)
  {
    return (val+55);
  }
  else
  {
    return (val+48);    
  }
}

u8 ASCII2BYTE(u8 high,u8 low)
{
  u8 tst;
  if(high>'9')
  {
    tst=(high-55)*16;
  }
  else
  {	 
    tst=(high-48)*16;  
  }	  
  if(low>'9')
  {
    tst+=(low-55);
  }
  else
  {	 
    tst+=(low-48);  
  }
  return tst;
}

u8 HEXtoASCII(u8 l_u8HEX)
{    
    //传入参数0~9、A~F
    if (l_u8HEX <=9)//无符号型
    {//0~9-0x30~0x39
        return(0x30+l_u8HEX);
    }
    else if ((l_u8HEX>=0x0A) && (l_u8HEX <=0x0F))
    {//A~F-0x41~0x46
        return(0x41+l_u8HEX-0x0A);
    }
    else
    {
        return 0xFF;
    }
}

u8 ASCIItoHEX(unsigned char l_u8ASCII)
{    
    //传入参数0~9、A~F
    if ((l_u8ASCII>=0x30) && (l_u8ASCII<=0x39))
    {//0~9-0x30~0x39
        return(l_u8ASCII-0x30);
    }
    else if ((l_u8ASCII>=0x41) && (l_u8ASCII<=0x46))
    {//A~F-0x41~0x46
        return(l_u8ASCII-0x41+10);
    }
    else
    {
        return 0xFF;
    }
}

u8 Count_bit1(u8 data)
{
    u8 count=0;        //置0计数
	u8 date=data;
    while(date)
    {
       count+=date&1;   //检查Num最后一位是否为1
       date>>=1;        //位移一位
    }
    return count;
}

u8 BCC_CalcCheck(u8 *data,u8 len)
{
  u16 sum;
  u8 i;
  sum=0;
  for(i=0;i<len;i++)
  {
    sum += data[i];
  }
  sum = 256-(u8)sum;
  return (u8)sum;
}

//======-----------------和校验------------------------------------------------------------------------------
void AndCalc_Check(u8 *l_u8ArrCheckData, u8 l_u8CheckDataLen, u8 *pl_u8CheckH, u8 *pl_u8CheckL)
{
    unsigned char i = 0;
    unsigned char l_u8CheckSum = 0;

    for (i=1; i<l_u8CheckDataLen; i++)//计算从CMD至ETX的累加和校验
    {
        l_u8CheckSum = l_u8CheckSum + l_u8ArrCheckData[i];
    }

    *pl_u8CheckH = HEXtoASCII((l_u8CheckSum>>4)&0x0F);
    *pl_u8CheckL = HEXtoASCII(l_u8CheckSum&0x0F);

}

void PositiveResponse(void)
{
  u8 i=0;
	u16 addrtemp;
  MB_EXECUTESTATUS = ON;
  SPEC_RESPONDSE=ON;
  if(g_u8_FunCode == 'm')
  {
    g_u8_EthTransData[(LENTH_BEFORE_DATA-2)+i++]=g_u8_FunCode;
    g_u8_EthTransData[(LENTH_BEFORE_DATA-2)+i++]=FUNC_RESP_POS;
    g_u8_EthTransLen=i-2;
  }
  else
  {
    MB_ResptoPC=ON;
    g_u8_EthTransData[(LENTH_BEFORE_DATA-2)+i++]=g_u8_FunCode;
    addrtemp=g_u8_WriteAddrOffset+g_u16_StartAddr[g_u8_RespondNode][g_u8_RespondID];
    g_u8_EthTransData[(LENTH_BEFORE_DATA-2)+i++]=(g_u8_RespondNode<<4)+addrtemp/256;
    g_u8_EthTransData[(LENTH_BEFORE_DATA-2)+i++]=addrtemp%256;
    if((Write_FuncCode==FUNC_WR_SGREG)||(Write_FuncCode==FUNC_WR_SGCOIL))
    {
      g_u8_EthTransData[(LENTH_BEFORE_DATA-2)+i++]=g_u8_Writedata[0];
      g_u8_EthTransData[(LENTH_BEFORE_DATA-2)+i++]=g_u8_Writedata[1];
    }
    else if((Write_FuncCode==FUNC_WR_MULREG)||(Write_FuncCode==FUNC_WR_MULCOIL))
    {
      g_u8_EthTransData[(LENTH_BEFORE_DATA-2)+i++]=g_u16_WriteLen>>8;
      g_u8_EthTransData[(LENTH_BEFORE_DATA-2)+i++]=g_u16_WriteLen;
    }
    else
    {

    }
    g_u8_EthTransLen=i-2;
  }
  Upload_data = UP_SUCCESS;
}

void NegativeResponse(u8 errortype)
{
  u8 i=0;
  MB_EXECUTESTATUS = ON;
  MB_TCP_ErrSta = errortype;
  MB_ReadRegVal(g_u8_RespondNode) = OFF;
  MB_NeedWrite(g_u8_RespondNode) = OFF;
  GW_ReadStatus(g_u8_RespondNode) = READ_IDLE;
  GW_WriteStatus(g_u8_RespondNode) = WRITE_IDLE;
  g_u8_EthTransData[(LENTH_BEFORE_DATA-2)+i++]=FUNC_RESP_NEG + g_u8_FunCode;
  g_u8_EthTransData[(LENTH_BEFORE_DATA-2)+i++]=errortype;
  g_u8_EthTransLen = i-2;  //不需打包功能码
  SPEC_RESPONDSE=ON;
  Upload_data = UP_FAIL;
}

void Node2_Init(void)
{
  u8 i;
  g_u8_ProtocalNum[NODE_USER_DEFINED][READ] = 16;
	g_u8_ProtocalNum[NODE_USER_DEFINED][WRITE] = 16;
	for(i=0;i<16;i++)
	{
  	g_u16_StartAddr[NODE_USER_DEFINED][i] = (256/2)*i;
  	RegisterCfgBuff[NODE_USER_DEFINED][i].datalenth = 256/2;
  	g_u16_RecvTransLen[NODE_USER_DEFINED][i]=256;
  }
}

void Node3_Init(void)
{
  u8 i;
  g_u8_ProtocalNum[IO_AD_Node][READ] = 0x05;
	g_u8_ProtocalNum[IO_AD_Node][WRITE] = 0x05;
	g_u16_StartAddr[IO_AD_Node][0] = 0x00;//6个输入+8个输出+5个AD+6个字的版本号信息
	g_u16_StartAddr[IO_AD_Node][1] = 0x19;//配置信息
  for(i=0;i<CFGAREA_UNIT;i++)
  {
    g_u16_StartAddr[IO_AD_Node][2+i] = 0x29+(CFGAREA_UNIT/2*i);//配置帧内容
  }
	RegisterCfgBuff[IO_AD_Node][0].datalenth = 6+8+5+6;
	RegisterCfgBuff[IO_AD_Node][1].datalenth = CFGAREA_UNIT/2;//结束帧内容
	RegisterCfgBuff[IO_AD_Node][2].datalenth = 1000;//
  for(i=0;i<CFGAREA_UNIT;i++)
  {
    RegisterCfgBuff[IO_AD_Node][4+i].datalenth = CFGAREA_UNIT/2;//配置帧内容
  }
	RegisterCfgBuff[IO_AD_Node][1].regaddr[6] = 0x00;
	RegisterCfgBuff[IO_AD_Node][1].regaddr[7] = 0x06;//配置输出继电器的地址

	memcpy(&g_u8_EthRespData[IO_AD_Node][38],SW_VERSION,12);
  //g_u8_EthRespData[IO_AD_Node][g_u16_StartAddr[IO_AD_Node][3]*2]=0x00;
  //g_u8_EthRespData[IO_AD_Node][g_u16_StartAddr[IO_AD_Node][3]*2+1]=g_u8_CfgCounter;
  for(i=0;i<3;i++)
	{
		g_u16_RecvTransLen[IO_AD_Node][i]=RegisterCfgBuff[IO_AD_Node][i].datalenth * 2;
	}
	memcpy(&g_u8_EthRespData[IO_AD_Node][g_u16_StartAddr[IO_AD_Node][1]*2],
	      (void *)MODBUS_CONFIG_BASEADDR,CFGAREA_UNIT);
  memcpy(&g_u8_EthRespData[IO_AD_Node][g_u16_StartAddr[IO_AD_Node][2]*2],
        (void *)(MODBUS_READREG_BASEADDR),2000);//剩余空间只能存放59个配置帧
}

void Platform_Init(u8 stationaddr,u16 wtcyc)
{
  u8 i,j;
  IO_AD_Node=NODE_DIG_ANG;
  if(CodeVal==0)
  {
    GatewayStationAddr=0x18;
  }
  else
  {
    GatewayStationAddr=stationaddr;
  }
  for(i=0;i<MAXNUM_NODE;i++)
  {
    g_u8_ProtocalID[i] = 0xff;
  }
  ReadAllCongfigData();
	DetectTime = 5000;
  MB_TCP_ErrSta=Sta_OK;
  g_u16_ETH_Timeout=0;
  SystemFlag=0x00;
  UpdateCycle[0]=(UpdateCycle[0]==0) ? 1  : UpdateCycle[0]*10;
  UpdateCycle[1]=(UpdateCycle[1]==0) ? 1  : UpdateCycle[1]*10;
  UpdateCycle[2]=(UpdateCycle[2]==0) ? 1  : UpdateCycle[2]*10;
  UpdateCycle[3]=(UpdateCycle[3]==0) ? 10 : UpdateCycle[3]*10;
  WriteTime=wtcyc;
  /*for(i=0;i<;i++)
  {
    g_u8_CfgCounter=g_u8_Read_ProtocalNum;
  }*/
	Node2_Init();
	Node3_Init();
}

void ServerSta_Detect(void)
{
		u8 i,j;
		
    for(i=0;i<4;i++)
    {
        if(g_u8TcpSlaveLclIP[i] != SlaveRead_1_Byte(SIPR+i))
        {
            break;
        }
    }
    for(j=0;j<6;j++)
    {
//         if(g_u8TcpSlaveLclMac[j] != SlaveRead_1_Byte(SHAR+j))
//         {
//             break;
//         }
    }
    DetectTime = 200;
    if((i<4)||(j<6))
    {
        MB_TCP_Slave_W5500_Init();
        MB_TCP_Slave_Socket_Init(0,502,MR_TCP); 
        MB_TCP_Slave_Socket_Init(6,60171,MR_TCP); 
        MB_TCP_Slave_Socket_Init(7,60172,MR_UDP); 
        DetectTime = 5000;
    }
}

void ClientSta_Detect(void)
{
    u8 i,j,temp[4];
    for(i=0;i<4;i++)
    {
        temp[i]=MasterRead_1_Byte(SIPR+i);
        if(g_u8TcpMasterLclIP[i] != MasterRead_1_Byte(SIPR+i))
        {
            break;
        }
    }
    for(j=0;j<6;j++)
    {
        //if(Config_TcpSlave_MACAddr[j] != SlaveRead_1_Byte(SHAR+j))
        //{
        //    break;
        //}
    }
    DetectTime = 200;
    if((i<4)||(j<6))
    {
        mb_tcp_master_W5500_Init();
        mb_tcp_master_Socket_Init(0);		//指定Socket(0~7)初始化
        DetectTime = 5000;
    }
}

void f_Acquisition_Task(void)
{
    u8 i;
    if(DetectTime==0)
    {
		ServerSta_Detect();
		ClientSta_Detect();
	}
	
  if(UpdateTime>=10)
	{
		UpdateTime=0;
		Input_Result[0] += GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_13);
		Input_Result[1] += GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_12);
		Input_Result[2] += GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_15);
		Input_Result[3] += GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_14);
		Input_Result[4] += GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11);
		Input_Result[5] += GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10);
		SampleIndex++;
		if(SampleIndex>=8)
		{
			SampleIndex=0;
			//g_u16_RecvTransLen[IO_AD_Node][0]=2;
			for(i=0;i<6;i++)
			{
				DI_STATUS(Input_Result[i],Input_Status[i]);
				Input_Result[i]=0;
				g_u8_EthRespData[IO_AD_Node][2*i+0]=0x00;
				g_u8_EthRespData[IO_AD_Node][2*i+1]=Input_Status[i];
			}
			ReadADCAverageValue();
      
			for(i=0;i<5;i++)
			{
        g_u8_EthRespData[IO_AD_Node][28+2*i]=AD_Result[i]/256;
        g_u8_EthRespData[IO_AD_Node][28+2*i+1]=AD_Result[i]%256;
			}
		}
	}

  if(ON==MB_NeedWrite(NODE_USER_DEFINED))
  {
    memcpy(&g_u8_EthRespData[NODE_USER_DEFINED][(g_u16_StartAddr[NODE_USER_DEFINED][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
    MB_NeedWrite(NODE_USER_DEFINED)=OFF;
    PositiveResponse();
  }

  if(ON==MB_NeedWrite(IO_AD_Node))
  {
    if(g_u8_WriteAddr[7]==0x06)
    {
      if(g_u8_Writedata[1] == ON)
      {
        RELAY1_ON;
        g_u8_EthRespData[IO_AD_Node][12]=0x00;
        g_u8_EthRespData[IO_AD_Node][13]=0x01;
      }
      else
      {
        RELAY1_OFF;
        g_u8_EthRespData[IO_AD_Node][12]=0x00;
        g_u8_EthRespData[IO_AD_Node][13]=0x00;
      }
    }
    if(g_u8_WriteAddr[7]==0x07)
    {
      if(g_u8_Writedata[1] == ON)
      {
        RELAY2_ON;
        g_u8_EthRespData[IO_AD_Node][14]=0x00;
        g_u8_EthRespData[IO_AD_Node][15]=0x01;
      }
      else
      {
        RELAY2_OFF;
        g_u8_EthRespData[IO_AD_Node][14]=0x00;
        g_u8_EthRespData[IO_AD_Node][15]=0x00;
      }
    }
    if(g_u8_WriteAddr[7]==0x08)
    {
      if(g_u8_Writedata[1] == ON)
      {
        RELAY3_ON;
        g_u8_EthRespData[IO_AD_Node][16]=0x00;
        g_u8_EthRespData[IO_AD_Node][17]=0x01;
      }
      else
      {
        RELAY3_OFF;
        g_u8_EthRespData[IO_AD_Node][16]=0x00;
        g_u8_EthRespData[IO_AD_Node][17]=0x00;
      }
    }
    if(g_u8_WriteAddr[7]==0x09)
    {
      if(g_u8_Writedata[1] == ON)
      {
        RELAY4_ON;
        g_u8_EthRespData[IO_AD_Node][18]=0x00;
        g_u8_EthRespData[IO_AD_Node][19]=0x01;
      }
      else
      {
        RELAY4_OFF;
        g_u8_EthRespData[IO_AD_Node][18]=0x00;
        g_u8_EthRespData[IO_AD_Node][19]=0x00;
      }
    }
	  if(g_u8_WriteAddr[7]==0x0A)
    {
      if(g_u8_Writedata[1] == ON)
      {
        RELAY5_ON;
        g_u8_EthRespData[IO_AD_Node][20]=0x00;
        g_u8_EthRespData[IO_AD_Node][21]=0x01;
      }
      else
      {
        RELAY5_OFF;
        g_u8_EthRespData[IO_AD_Node][20]=0x00;
        g_u8_EthRespData[IO_AD_Node][21]=0x00;
      }
    }
	  if(g_u8_WriteAddr[7]==0x0B)
    {
      if(g_u8_Writedata[1] == ON)
      {
        RELAY6_ON;
        g_u8_EthRespData[IO_AD_Node][22]=0x00;
        g_u8_EthRespData[IO_AD_Node][23]=0x01;
      }
      else
      {
        RELAY6_OFF;
        g_u8_EthRespData[IO_AD_Node][22]=0x00;
        g_u8_EthRespData[IO_AD_Node][23]=0x00;
      }
    }
    if(g_u8_WriteAddr[7]==0x0C)
    {
      if(g_u8_Writedata[1] == ON)
      {
        TOUT1_ON;
        g_u8_EthRespData[IO_AD_Node][24]=0x00;
        g_u8_EthRespData[IO_AD_Node][25]=0x01;
      }
      else
      {
        TOUT1_OFF;
        g_u8_EthRespData[IO_AD_Node][24]=0x00;
        g_u8_EthRespData[IO_AD_Node][25]=0x00;
      }
    }
    if(g_u8_WriteAddr[7]==0x0D)
    {
      if(g_u8_Writedata[1] == ON)
      {
        TOUT2_ON;
        g_u8_EthRespData[IO_AD_Node][26]=0x00;
        g_u8_EthRespData[IO_AD_Node][27]=0x01;
      }
      else
      {
        TOUT2_OFF;
        g_u8_EthRespData[IO_AD_Node][26]=0x00;
        g_u8_EthRespData[IO_AD_Node][27]=0x00;
      }
    }
    MB_NeedWrite(IO_AD_Node)=OFF;
    PositiveResponse();
  }
	
}

void f_GenSoftwareReset(void)
{
  __set_FAULTMASK(1);
	SCB->AIRCR = AIRCR_VECTKEY_MASK | (u32)0x04;
}

void SystemReset(void)
{
  __set_FAULTMASK(1);
  NVIC_SystemReset();
}


void f_MBTCP_Transmit(u8 *tcpsn)
{
  u8 needsend,datalen,j;
	u16 i;
  if(MB_ResptoPC == ON)
  {
    MB_ResptoPC=OFF;
    if(MB_EXECUTESTATUS == ON)
    {
      MB_TCP_ErrSta = Sta_OK;
      MB_EXECUTESTATUS = OFF;
    }
    else if(ON == MB_ReadRegVal(g_u8_RespondNode))
    {
      MB_ReadRegVal(g_u8_RespondNode) = OFF;
      for(i=0;i<MAXNUM_BOARDSENDDATA;i++)
      {
        g_u8_EthTransData[i]=0;
      }
      if(g_u16_RecvTransLen[g_u8_RespondNode][g_u8_RespondID]==0)
      {
        NegativeResponse(Err_MBcmd);
        MB_TCP_ErrSta = Sta_OK;
      }
      else
      {
        g_u8_EthTransLen=0;
        Upload_data = UP_SUCCESS;
        g_u8_EthTransLen=g_u16_ReadLen*2;
        memcpy(&g_u8_EthTransData[LENTH_BEFORE_DATA],&g_u8_EthRespData[g_u8_RespondNode][g_u16_ReadAddr*2],g_u8_EthTransLen);
      }
      MB_EXECUTESTATUS = OFF;
    }
    else if(ON == MB_ReadVersion)
    {
      MB_ReadVersion = OFF;
    }
    else
    {
      return;
    }
    datalen = g_u8_EthTransLen+LENTH_NONDATA;

    g_u8_EthTransData[0]=tcpsn[0];
    g_u8_EthTransData[1]=tcpsn[1];
    g_u8_EthTransData[2]=0x00;
    g_u8_EthTransData[3]=0x00;
    g_u8_EthTransData[4]=(datalen-6)>>8;
    g_u8_EthTransData[5]=(datalen-6)%256;
    g_u8_EthTransData[6]=GatewayStationAddr;
    if(SPEC_RESPONDSE==OFF)
    {
      g_u8_EthTransData[7]=g_u8_FunCode;
      g_u8_EthTransData[8]=g_u8_EthTransLen;
    }
    else
    {
      SPEC_RESPONDSE=OFF;
    }
    SlaveWrite_SOCK_Data_Buffer(g_u8_RespondSocket,g_u8_EthTransData,datalen);
  }
}

u8 ProgramCongfigData(u8 lastcfg,u8 *data)
{
  u8 i,ret;
  u32 temp[CFGAREA_UNIT/4];
  ret=0;
  for(i=0;i<CFGAREA_UNIT/4;i++)
  {
    temp[i]=0;
  }
  if(lastcfg==1)
  {
    data[0]=g_u8_CfgCounter;
    for(i=0;i<CFGAREA_UNIT/4;i++)
    {
      temp[i]=(u32)(data[4*i+3]<<24) + (u32)(data[4*i+2]<<16) + (u32)(data[4*i+1]<<8) + (u32)data[4*i+0];
    }
    ret=Flash_Write(MODBUS_CONFIG_BASEADDR,temp,CFGAREA_UNIT);
  }
  else
  {
    for(i=0;i<CFGAREA_UNIT/4;i++)
    {
      temp[i]=(u32)(data[4*i+3]<<24) + (u32)(data[4*i+2]<<16) + (u32)(data[4*i+1]<<8) + (u32)data[4*i+0];
    }
    ret=Flash_Write(MODBUS_READREG_BASEADDR+g_u8_CfgCounter*CFGAREA_UNIT,temp,CFGAREA_UNIT);
  }
  return ret;
}

void ReadAllCongfigData(void)
{
  u8 i,j,node,rwtype,index,temp,cfgcnt[MAXNUM_NODE];
  s8 cnt[MAXNUM_READPROTOCAL][2];
  //GW_CfgComplete        = ((u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR)))&0x01;
  g_u8_CfgCounter=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR));

  if(g_u8_CfgCounter==0xff) return;

  CommType=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR+4));
  if(((CommType&0x0F)==Comm_RS232)||((CommType&0x0F)==Comm_RS485))
  {
		CommPort=((CommType>>4)-1)&0x0F;
		temp=(u8)((*(u16 *)(MODBUS_CONFIG_BASEADDR+4))>>8);
      
    if(((temp>>6)&0x03) == 0x00)
    {
      CommParmCfg.flowctrl = USART_HardwareFlowControl_None;
    }
    else if(((temp>>6)&0x03) == 0x01)
    {
      CommParmCfg.flowctrl = USART_HardwareFlowControl_RTS;
    }
    else if(((temp>>6)&0x03) == 0x02)
    {
      CommParmCfg.flowctrl = USART_HardwareFlowControl_CTS;
    }
    else
    {
      CommParmCfg.flowctrl = USART_HardwareFlowControl_RTS_CTS;
    }
    if(((temp>>4)&0x03) == 0x00)
    {
      CommParmCfg.stopbit= USART_StopBits_0_5;
    }
    else if(((temp>>4)&0x03) == 0x01)
    {
      CommParmCfg.stopbit = USART_StopBits_1;
    }
    else if(((temp>>4)&0x03) == 0x02)
    {
      CommParmCfg.stopbit = USART_StopBits_1_5;
    }
    else
    {
      CommParmCfg.stopbit = USART_StopBits_2;
    }
		if(((temp>>2)&0x03) == 0x00)
    {
      CommParmCfg.paritybit= USART_Parity_No;
    }
    else if(((temp>>2)&0x03) == 0x01)
    {
      CommParmCfg.paritybit = USART_Parity_Odd;
    }
    else if(((temp>>2)&0x03) == 0x02)
    {
      CommParmCfg.paritybit = USART_Parity_Even;
    }
    else
    {
      CommParmCfg.paritybit = USART_Parity_No;
    }
		if((((temp&0x03) == 0x00)&&(CommParmCfg.paritybit!=USART_Parity_No))||
		   (((temp&0x03) == 0x01)&&(CommParmCfg.paritybit==USART_Parity_No)))
    {
      CommParmCfg.databit = USART_WordLength_8b;
    }
    else if(((temp&0x03) == 0x01)&&(CommParmCfg.paritybit!=USART_Parity_No))
    {
      CommParmCfg.databit = USART_WordLength_9b;
    }
    else
    {
      CommParmCfg.databit = USART_WordLength_8b;
    }  
    CommParmCfg.baudrate=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR+6))*256 + 
      (u8)((*(u16 *)(MODBUS_CONFIG_BASEADDR+6))>>8);
    CommParmCfg.baudrate=CommParmCfg.baudrate*10;
  }
  else if((CommType&0x0F)==Comm_CAN)
  {
    CommParmCfg.baudrate=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR+6))*256 + 
        (u8)((*(u16 *)(MODBUS_CONFIG_BASEADDR+6))>>8);
  }
  else
  {

  }
  
  //if(CodeVal != 0x00)
  {
    g_u8TcpSlaveLclIP[0]=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR+8));
    g_u8TcpSlaveLclIP[1]=(u8)((*(u16 *)(MODBUS_CONFIG_BASEADDR+8))>>8);
    g_u8TcpSlaveLclIP[2]=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR+10));
    g_u8TcpSlaveLclIP[3]=(u8)((*(u16 *)(MODBUS_CONFIG_BASEADDR+10))>>8);
  }
  g_u8TcpMasterLclIP[0]=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR+12));
  g_u8TcpMasterLclIP[1]=(u8)((*(u16 *)(MODBUS_CONFIG_BASEADDR+12))>>8);
  g_u8TcpMasterLclIP[2]=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR+14));
  g_u8TcpMasterLclIP[3]=(u8)((*(u16 *)(MODBUS_CONFIG_BASEADDR+14))>>8);
  g_u16TcpMasterRmtPort[0]=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR+16))*256+(u8)((*(u16 *)(MODBUS_CONFIG_BASEADDR+16))>>8);
  g_u16TcpMasterLclPort[0]=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR+18))*256+(u8)((*(u16 *)(MODBUS_CONFIG_BASEADDR+18))>>8);
  g_u8TcpMasterRmtIP[0]=g_u8TcpMasterLclIP[0];
  g_u8TcpMasterRmtIP[1]=g_u8TcpMasterLclIP[1];
  g_u8TcpMasterRmtIP[2]=g_u8TcpMasterLclIP[2];
  g_u8TcpMasterRmtIP[3]=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR+20));
  UpdateCycle[0]=(u8)((*(u16 *)(MODBUS_CONFIG_BASEADDR+20))>>8);
  UpdateCycle[1]=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR+22));
  UpdateCycle[2]=(u8)((*(u16 *)(MODBUS_CONFIG_BASEADDR+22))>>8);
	g_u8_ProtocalID[0]=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR+24));
  g_u8_ProtocalID[1]=(u8)((*(u16 *)(MODBUS_CONFIG_BASEADDR+24))>>8);
	g_u8_ProtocalID[2]=(u8)(*(u16 *)(MODBUS_CONFIG_BASEADDR+26));
	if(g_u8_ProtocalID[1]==g_u8_ProtocalID[0])  g_u8_ProtocalID[1]=0xff;
	if(g_u8_ProtocalID[2]==g_u8_ProtocalID[0])  g_u8_ProtocalID[2]=0xff;
	if(g_u8_ProtocalID[2]==g_u8_ProtocalID[1])  g_u8_ProtocalID[2]=0xff;
  
  for(i=0;i<MAXNUM_NODE;i++)
  {
    cnt[i][0]=-1;
    cnt[i][1]=-1;
    cfgcnt[i]=0x00;
    for(j=0;j<MAXNUM_READPROTOCAL;j++)
    {
      g_u16_StartAddr[i][j]=0x00;
    }
  }
  
  for(i=0;i<g_u8_CfgCounter;i++)
  {
    node = (((u8)((*(u16 *)(MODBUS_READREG_BASEADDR+i*CFGAREA_UNIT)))>>4))&0x0f;
    if(node>=MAXNUM_NODE)
    {
      node = 0;
    }
    rwtype = 0;//((u8)((*(u16 *)(MODBUS_READREG_BASEADDR+i*32))))&0x01;
    for(j=0;j<MAXNUM_NODE;j++)
    {
      if(node==j)
      {
        cnt[j][rwtype]++;
        index=cnt[j][rwtype];
      }
    }
    g_u8_ProtocalNum[node][READ]++;
    g_u8_ProtocalNum[node][WRITE]++;
    RegisterCfgBuff[node][index].cmdstatus.Bits.fbrd   = ((u8)((*(u16 *)(MODBUS_READREG_BASEADDR+i*CFGAREA_UNIT))))&0x01;
    RegisterCfgBuff[node][index].cmdstatus.Bits.nodenum   = node;
    RegisterCfgBuff[node][index].regtype        = (u8)(*(u16 *)(MODBUS_READREG_BASEADDR+i*CFGAREA_UNIT)>>8);
    RegisterCfgBuff[node][index].operatesize    = (u8)(*(u16 *)(MODBUS_READREG_BASEADDR+i*CFGAREA_UNIT+2));
    RegisterCfgBuff[node][index].datalenth      = (u8)((*(u16 *)(MODBUS_READREG_BASEADDR+i*CFGAREA_UNIT+2))>>8);
    RegisterCfgBuff[node][index].plcaddrstation = (u8)(*(u16 *)(MODBUS_READREG_BASEADDR+i*CFGAREA_UNIT+4));
    RegisterCfgBuff[node][index].specfuncode    = (u8)((*(u16 *)(MODBUS_READREG_BASEADDR+i*CFGAREA_UNIT+4))>>8);
    for(j=0;j<REG_ADDRSIZE;j++)
    {
      if(j%2==0)
      {
        RegisterCfgBuff[node][index].regaddr[j] = (u8)(*(u16 *)(MODBUS_READREG_BASEADDR+i*CFGAREA_UNIT+j+6));
      }
      else
      {
        RegisterCfgBuff[node][index].regaddr[j] = (u8)((*(u16 *)(MODBUS_READREG_BASEADDR+i*CFGAREA_UNIT+(j-1)+6))>>8);
      }                            
    }
    g_u16_StartAddr[node][cfgcnt[node]+1]=g_u16_StartAddr[node][cfgcnt[node]]+RegisterCfgBuff[node][index].datalenth;
    cfgcnt[node]++;
  }
}

void Read_Version(u8 vercode)
{
  MB_ReadVersion=ON;
  //g_u8_RespondID=g_u8_Read_ProtocalNum;
  //g_u8_RecvTransBuf[g_u8_RespondID][0]='A';
  //g_u8_RecvTransBuf[g_u8_RespondID][1]=vercode;
  switch(vercode)
  {
    case 1:
      //g_u8_RecvTransBuf[0][g_u8_Read_ProtocalNum][2]=g_u8_Read_ProtocalNum;
      //g_u16_RecvTransLen[0][g_u8_RespondID]=3;
      break;
    case 2:

      break;
    default: break;
  }
}

void TCP_Recv_EthData(u8 socket,u8 *Ethrecdata, u8 lenth)
{
  u16 usStartAddr,usNumber,cfgcounter,node,resID,temp;
	u8 i,j,temp_offset,temp_and,flashdata[CFGAREA_UNIT];
  g_u8_FunCode = Ethrecdata[1];
  usStartAddr = (u16)(Ethrecdata[2]<<8);
  usStartAddr +=(u16)(Ethrecdata[3]);
  usNumber  = (u16)(Ethrecdata[4]<<8);
  usNumber += (u16)(Ethrecdata[5]);
  node = usStartAddr/0x1000;
  MB_ResptoPC=ON;
  if((g_u8_FunCode==FUNC_RD_HOLDREG))//如果收到读取保持寄存器命令，则启动S7-200TCP，发送读取命令
  {
    for(i=0;i<MAXNUM_NODE;i++)
    {
      if((MB_NeedWrite(i) == ON)||(SPEC_RESPONDSE == ON))
      {
        //MB_ResptoPC=ON;
				return;
      }
    }
    for(i=0;i<MAXNUM_READPROTOCAL;i++)
    {
      if(((usStartAddr&0xfff)>=g_u16_StartAddr[node][i])&&((usStartAddr&0xfff)<g_u16_StartAddr[node][i+1]))
      {
        resID = i;
        break;
      }
    }
    g_u8_RespondSocket = socket;
    if(i==MAXNUM_READPROTOCAL)
    {
      NegativeResponse(Err_CfgUnmatch);
      return;
    }
    if(resID < g_u8_ProtocalNum[node][READ])
    {
      temp=0;
      g_u8_ReqCfgNum=0;
      for(i=resID;i<MAXNUM_READPROTOCAL;i++)
      {
        temp += RegisterCfgBuff[node][i].datalenth;
        g_u8_ReqCfgNum++;
        if(temp>=usNumber)
        {
          break;
        }
      }
      if(temp>(MAXNUM_BOARDSENDDATA-LENTH_NONDATA))//超出当前支持的最大数据长度
      {
        NegativeResponse(Err_CfgUnmatch);
      }
			else if(temp<usNumber)
			{
				NegativeResponse(Err_CfgUnmatch);
			}
      else
      {
        g_u8_RespondNode = node;
        g_u8_RespondID = resID;
				g_u16_ReadAddr=usStartAddr%0x1000;
				g_u16_ReadLen=usNumber;
        MB_ReadRegVal(g_u8_RespondNode) = ON;
      }
    }
    else
    {
      NegativeResponse(Err_CfgUnmatch);
    }
	}
	else if(g_u8_FunCode==FUNC_WR_SGREG)
  {
    for(i=0;i<MAXNUM_NODE;i++)
    {
      if((MB_NeedWrite(i) == ON)||(SPEC_RESPONDSE == ON))
      {
        return;
      }
    }
    for(i=0;i<MAXNUM_READPROTOCAL;i++)
    {
      if((usStartAddr&0xfff)==g_u16_StartAddr[node][i])
      {
        resID = i;
        temp_offset=0;
        for(j=0;j<REG_ADDRSIZE;j++)
        {
          g_u8_WriteAddr[j]=RegisterCfgBuff[node][resID].regaddr[j];
        }
        break;
      }
      else if((usStartAddr&0xfff)<g_u16_StartAddr[node][i])
      {
        resID = i-1;
        for(j=0;j<REG_ADDRSIZE;j++)
        {
          g_u8_WriteAddr[j]=RegisterCfgBuff[node][resID].regaddr[j];
        }
        temp_offset = (usStartAddr&0xfff) - g_u16_StartAddr[node][resID];
        if((g_u8_ProtocalID[node]==NODE_S7200PPI)||(g_u8_ProtocalID[node]==NODE_S7200TCP)||
          (g_u8_ProtocalID[node]==NODE_S7300TCP))
        {
          temp_offset = ((usStartAddr&0xfff) - g_u16_StartAddr[node][resID])*2;
        }
        else{
          temp_offset = (usStartAddr&0xfff) - g_u16_StartAddr[node][resID];
        }
        temp_and = g_u8_WriteAddr[7]+(temp_offset%256);
        if(temp_and>0xff)
        {
          g_u8_WriteAddr[6]++;
        }
        g_u8_WriteAddr[6] += temp_offset/256;
        g_u8_WriteAddr[7] += temp_offset%256;
        break;
      }
      else
      {

      }
    }
    g_u8_RespondSocket = socket;
    if(i==MAXNUM_READPROTOCAL)
    {
      NegativeResponse(Err_CfgUnmatch);
      return;
    }

    if((g_u8_ProtocalID[node]==NODE_S7200PPI)||(g_u8_ProtocalID[node]==NODE_S7200TCP)||
          (g_u8_ProtocalID[node]==NODE_S7300TCP))
    {
      temp = temp_offset/2;
    }
    else
    {
      temp = temp_offset;
    }
    if(resID < g_u8_ProtocalNum[node][WRITE])
    {
      for(i=0;i<2;i++)
      {
        g_u8_Writedata[i]=Ethrecdata[4+i];
      }
      g_u8_RespondNode = node;
      g_u8_RespondID = resID;
			g_u16_WriteLen = 1;
			g_u8_WriteAddrOffset = temp;
			Write_FuncCode=FUNC_WR_SGREG;
      GW_WriteStatus(g_u8_RespondNode)=WRITE_IDLE;	
      MB_NeedWrite(g_u8_RespondNode) = ON;
    }
    else
    {
      NegativeResponse(Err_CfgUnmatch);
    }
	}
  else if(g_u8_FunCode==FUNC_WR_MULREG)
  {
    for(i=0;i<MAXNUM_NODE;i++)
    {
      if((MB_NeedWrite(i) == ON)||(SPEC_RESPONDSE == ON))
      {
        return;
      }
    }
    for(i=0;i<MAXNUM_READPROTOCAL;i++)
    {
      if((usStartAddr&0xfff)==g_u16_StartAddr[node][i])
      {
        resID = i;
        temp_offset=0;
        for(j=0;j<REG_ADDRSIZE;j++)
        {
          g_u8_WriteAddr[j]=RegisterCfgBuff[node][resID].regaddr[j];
        }
        break;
      }
      else if((usStartAddr&0xfff)<g_u16_StartAddr[node][i])
      {
        resID = i-1;
        for(j=0;j<REG_ADDRSIZE;j++)
        {
          g_u8_WriteAddr[j]=RegisterCfgBuff[node][resID].regaddr[j];
        }
        temp_offset = (usStartAddr&0xfff) - g_u16_StartAddr[node][resID];
        if((g_u8_ProtocalID[node]==NODE_S7200PPI)||(g_u8_ProtocalID[node]==NODE_S7200TCP)||
          (g_u8_ProtocalID[node]==NODE_S7300TCP))
        {
          temp_offset = ((usStartAddr&0xfff) - g_u16_StartAddr[node][resID])*2;
        }
        else{
          temp_offset = (usStartAddr&0xfff) - g_u16_StartAddr[node][resID];
        }
        temp_and = g_u8_WriteAddr[7]+(temp_offset%256);
        if(temp_and>0xff)
        {
          g_u8_WriteAddr[6]++;
        }
        g_u8_WriteAddr[6] += temp_offset/256;
        g_u8_WriteAddr[7] += temp_offset%256;
        break;
      }
      else
      {

      }
    }
    g_u8_RespondSocket = socket;
    if(i==MAXNUM_READPROTOCAL)
    {
      NegativeResponse(Err_CfgUnmatch);
      return;
    }

    if((g_u8_ProtocalID[node]==NODE_S7200PPI)||(g_u8_ProtocalID[node]==NODE_S7200TCP)||
          (g_u8_ProtocalID[node]==NODE_S7300TCP))
    {
      temp = temp_offset/2;
    }
    else
    {
      temp = temp_offset;
    }
    if((resID < g_u8_ProtocalNum[node][WRITE])&&
       (usNumber <= RegisterCfgBuff[node][resID].datalenth-temp))
    {
      for(i=0;i<usNumber*2;i++)
      {
        g_u8_Writedata[i]=Ethrecdata[7+i];
      }
      g_u8_RespondNode = node;
      g_u8_RespondID = resID;
			g_u16_WriteLen = usNumber;
			g_u8_WriteAddrOffset = temp;
			Write_FuncCode=FUNC_WR_MULREG;
      GW_WriteStatus(g_u8_RespondNode)=WRITE_IDLE;	
      MB_NeedWrite(g_u8_RespondNode) = ON;
    }
    else
    {
      NegativeResponse(Err_CfgUnmatch);
    }
	}
  else if(g_u8_FunCode=='m')
  {
    g_u8_RespondSocket = socket;
    temp=flashdata[0]>>4;
    
    if((Ethrecdata[2]&0x80)>>7)
    {
      for(i=0;i<lenth;i++)//
      {
        flashdata[0+i]=Ethrecdata[0+i];
      }
      Flash_Erase(MODBUS_CONFIG_BASEADDR,1);
      if(FLASH_COMPLETE == ProgramCongfigData(((Ethrecdata[2]&0x80)>>7),flashdata))
      {
        PositiveResponse();
      }
      else
      {
        NegativeResponse(Err_FlashWrFail);
      }
      f_MBTCP_Transmit(Ethrecdata);//此处仅仅为了协议，传入的参数无意义
      Delay1ms(100);
      f_GenSoftwareReset();
    }
    else
    {
      for(i=0;i<lenth;i++)//
      {
        flashdata[0+i]=Ethrecdata[4+i];
      }
      if(Ethrecdata[3]==0x00)
      {
        for(i=0;i<MAXNUM_NODE;i++)
        {
          g_u8_ProtocalNum[i][READ]=0;
          g_u8_ProtocalNum[i][WRITE]=0;
        }
        g_u8_CfgCounter=0;
        Flash_Erase(MODBUS_CONFIG_BASEADDR,CONFIG_PAGENUMBER); 
      }
      if((g_u8_CfgCounter == Ethrecdata[3])&&
        (FLASH_COMPLETE == ProgramCongfigData(((Ethrecdata[2]&0x80)>>7),flashdata)))
      {
        g_u8_CfgCounter++;
        PositiveResponse();
      }
      else
      {
        NegativeResponse(Err_FlashWrFail);
      }
    }
  }
  else if(g_u8_FunCode=='A')
  {
    Read_Version(Ethrecdata[1]);
  }
  else
  {
    NegativeResponse(Err_FunCode);
  }
}




