#include "main.h"
#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"	 
#include "m_spi_mb_tcp_master.h"	  
#include "m_spi_mb_tcp_master_cfg.h"
#include "tcp_master_mb.h"
#include "m_w5500.h"
#include "t_AB_Compact_tcp.h" 

#define ABCOMPACT_TCP_SEND_LENGTH             300          //���ͻ�������С
#define ABCOMPACT_TCP_RECE_LENGTH             300         //���ܻ�������С

#define Off_PLCName                       107
#define LABLE_BOOL                          1
#define LABLE_BYTE                          2
#define LABLE_SHORT                         3
#define LABLE_LONG                          4
#define LABLE_FLOAT                         5


enum AB_Compact_StatusTyp{
  PLC_Idle,
  Login_Request,
  Open_Request,
  Read_PLCName,
  NormalMode
};
enum AB_Compact_StatusTyp AB_Compact_Status;

static u16 ConnectTime;
static u8 AB_Compact_tcp=0xff;
static u8 Retry_Flag;
static u8 PLC_Respond;
static u8 g_u8AB1769tcpSend[ABCOMPACT_TCP_SEND_LENGTH]; 
static u8 g_u8AB1769ReceData[ABCOMPACT_TCP_RECE_LENGTH];
static u8 Default_SessionHandle[4]={0x00,0x00,0x00,0x00};
static u8 Register_SessionHandle[4]={0x00,0x00,0x00,0x00};
static u8 Open_ReqTrace[4]={0x20,0x06,0x24,0x01};
static u8 Operate_ReqTrace[4];
static u8 O_T_NetworkConnectID[4]={0x00,0x00,0x00,0x00};
static u16 FrameCount;
static u8 PLC_ProgramName[REG_ADDRSIZE];
static u8 PLC_ProgNameLenth;
static u8 AB_Compact_Enable=OFF;

void AB_Compact_tcp_Init(u8 nodeID)
{
  AB_Compact_tcp=nodeID;
  AB_Compact_Status=PLC_Idle;
  Retry_Flag=OFF;
  FrameCount=1;
  PLC_Respond =ON;//�ϵ���һ�η���ע������
  memset(g_u8AB1769tcpSend,0,ABCOMPACT_TCP_SEND_LENGTH);
  memset(g_u8AB1769ReceData,0,ABCOMPACT_TCP_RECE_LENGTH);
  AB_Compact_Enable=ON;
}

void TIM_1ms_ABCompacttcp(void)
{
  if(AB_Compact_Status != NormalMode)
  {
    ConnectTime++;
    if(ConnectTime>=8000)
    {
      ConnectTime =0;
      AB_Compact_Status=PLC_Idle;
      PLC_Respond =ON;//����ע��
    }
    return;
  }

  if(AB_Compact_Enable==ON)
  {
    g_u16_SwitchTimer[AB_Compact_tcp]++;
  }
  if((ON==MB_NeedWrite(AB_Compact_tcp))&&(READ_IDLE==GW_ReadStatus(AB_Compact_tcp)))
	{ 	
    if(GW_WriteStatus(AB_Compact_tcp)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[AB_Compact_tcp]>1000)
	     {
	     		g_u16_SwitchTimer[AB_Compact_tcp]=0;
					GW_WriteStatus(AB_Compact_tcp)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(AB_Compact_tcp)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[AB_Compact_tcp]>WriteTime))
	     {	
		   		GW_WriteStatus(AB_Compact_tcp)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[AB_Compact_tcp]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(AB_Compact_tcp)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(AB_Compact_tcp)=WRITE_DELAY;
      g_u16_SwitchTimer[AB_Compact_tcp]=0;
	  }
	  else if(g_u16_SwitchTimer[AB_Compact_tcp]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[AB_Compact_tcp]=0;
      GW_WriteStatus(AB_Compact_tcp)=WRITE_IDLE;
	    MB_NeedWrite(AB_Compact_tcp)=OFF;
	    g_u8_threadIdx[AB_Compact_tcp]++;
		  ThreadNew(AB_Compact_tcp)=ON;
	  }
	  else{

	  }
	}
	else
	{
		if((g_u16_SwitchTimer[AB_Compact_tcp]>UpdateCycle[AB_Compact_tcp])&&
		    (g_u8_ProtocalNum[AB_Compact_tcp][READ]!=0))
		{	  
		  g_u16_SwitchTimer[AB_Compact_tcp]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(AB_Compact_tcp))
		  {
		    GW_ReadStatus(AB_Compact_tcp)=READ_WAITRESPOND;
		    g_u16_TimeoutCnt[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]=0;
		    if(OFF==MB_NeedWrite(AB_Compact_tcp))
		    {
          g_u8_threadIdx[AB_Compact_tcp]++;
          while(RegisterCfgBuff[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[AB_Compact_tcp]++;
            if(g_u8_ProtocalNum[AB_Compact_tcp][READ]<=g_u8_threadIdx[AB_Compact_tcp])
      			{			  
      	      g_u8_threadIdx[AB_Compact_tcp]=0;
      			}
          }
		      ThreadNew(AB_Compact_tcp)=ON;
		    }
		    else
		    {
          GW_ReadStatus(AB_Compact_tcp)=READ_IDLE;
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]++;
        if(g_u16_TimeoutCnt[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]>
          (THRES_TIMOUTCNT/UpdateCycle[AB_Compact_tcp]))
        {
          g_u16_RecvTransLen[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]=0;
          g_u16_TimeoutCnt[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]=0;
          if(OFF==MB_NeedWrite(AB_Compact_tcp))
  		    {
            g_u8_threadIdx[AB_Compact_tcp]++;
            while(RegisterCfgBuff[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[AB_Compact_tcp]++;
              if(g_u8_ProtocalNum[AB_Compact_tcp][READ]<=g_u8_threadIdx[AB_Compact_tcp])
        			{			  
        	      g_u8_threadIdx[AB_Compact_tcp]=0;
        			}
            }
  		      ThreadNew(AB_Compact_tcp)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(AB_Compact_tcp)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[AB_Compact_tcp][READ]<=g_u8_threadIdx[AB_Compact_tcp])
	{			  
		g_u8_threadIdx[AB_Compact_tcp]=0;
	}
}

void LoginRequest(u8 *sessionhandle)
{
  u8 index;
  index=0;
  memset(g_u8AB1769tcpSend,0,ABCOMPACT_TCP_SEND_LENGTH);
  g_u8AB1769tcpSend[index++]=0x65;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x04;
  g_u8AB1769tcpSend[index++]=0x00;
  memcpy(&g_u8AB1769tcpSend[index],sessionhandle,4);//�Ự���
  index =index+4;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//״̬��Ĭ��Ϊ0x00000000
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//���ͷ�����
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//ѡ�Ĭ��Ϊ0x00000000
  //����������ָ������(command specific data)
  g_u8AB1769tcpSend[index++]=0x01;
  g_u8AB1769tcpSend[index++]=0x00;//Э��汾��Ĭ��Ϊ0x0001
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//ѡ���ǣ�Ĭ��Ϊ0x0000
  DAQ_EthSend(0,g_u8AB1769tcpSend,index);
}

void OpenRequest(u8 *sessionhandle)
{
  u8 index;
  index=0;
  memset(g_u8AB1769tcpSend,0,ABCOMPACT_TCP_SEND_LENGTH);
  g_u8AB1769tcpSend[index++]=0x6F;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x40;
  g_u8AB1769tcpSend[index++]=0x00;
  memcpy(&g_u8AB1769tcpSend[index],sessionhandle,4);//�Ự���
  index =index+4;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//״̬��Ĭ��Ϊ0x00000000
  g_u8AB1769tcpSend[index++]=0x1E;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0xE8;
  g_u8AB1769tcpSend[index++]=0x21;
  g_u8AB1769tcpSend[index++]=0x28;
  g_u8AB1769tcpSend[index++]=0x02;//���ͷ�����
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//ѡ�Ĭ��Ϊ0x00000000
  //����������ָ������(command specific data)
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//�ӿھ����Ĭ��Ϊ0x00000000(CIP)
  g_u8AB1769tcpSend[index++]=0x20;
  g_u8AB1769tcpSend[index++]=0x00;//��ʱ��Ĭ��Ϊ0x0001
  g_u8AB1769tcpSend[index++]=0x02;
  g_u8AB1769tcpSend[index++]=0x00;//������Ĭ��Ϊ0x0002
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//�յ�ַ�Ĭ��Ϊ0x0000
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//���ȣ�Ĭ��Ϊ0x0000
  g_u8AB1769tcpSend[index++]=0xB2;
  g_u8AB1769tcpSend[index++]=0x00;//δ���������Ĭ��Ϊ0x00B2
  g_u8AB1769tcpSend[index++]=0x30;
  g_u8AB1769tcpSend[index++]=0x00;//������ݰ��ĳ���(48���ֽ�)
  //������CIPЭ�������
  g_u8AB1769tcpSend[index++]=0x54;//���񣬹̶�Ϊ0x54
  g_u8AB1769tcpSend[index++]=0x02;//����·����С���̶�Ϊ0x02
  memcpy(&g_u8AB1769tcpSend[index],Open_ReqTrace,4);//����·�����̶�Ϊ0x01240620
  index=index+4;
  g_u8AB1769tcpSend[index++]=0x07;//Priority/time_tick
  g_u8AB1769tcpSend[index++]=0xF9;//Time_out_ticks
  g_u8AB1769tcpSend[index++]=0x02;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x80;//O-T Network Connection ID,Ĭ��0x00000000
  g_u8AB1769tcpSend[index++]=0x01;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0xFE;
  g_u8AB1769tcpSend[index++]=0x80;//T-O Network Connection ID������������
  g_u8AB1769tcpSend[index++]=0x02;
  g_u8AB1769tcpSend[index++]=0x00;//Connection serial number,0x0000
  g_u8AB1769tcpSend[index++]=0x4D;
  g_u8AB1769tcpSend[index++]=0x00;//Vender ID,0x0101
  g_u8AB1769tcpSend[index++]=0xD2;
  g_u8AB1769tcpSend[index++]=0x5E;
  g_u8AB1769tcpSend[index++]=0x11;
  g_u8AB1769tcpSend[index++]=0x2B;//Originator Serial Number ,��T-O Network Connection ID��ͬ
  g_u8AB1769tcpSend[index++]=0x00;//���ӳ�ʱ����
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//��������
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x12;
  g_u8AB1769tcpSend[index++]=0x7A;
  g_u8AB1769tcpSend[index++]=0x00;//O-T RPI,0x004c4b40
  g_u8AB1769tcpSend[index++]=0xF4;
  g_u8AB1769tcpSend[index++]=0x43;//O-T RPI,0x43f8
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x12;
  g_u8AB1769tcpSend[index++]=0x7A;
  g_u8AB1769tcpSend[index++]=0x00;//O-T RPI,0x004c4b40
  g_u8AB1769tcpSend[index++]=0xF4;
  g_u8AB1769tcpSend[index++]=0x43;//O-T RPI,0x43f8
  g_u8AB1769tcpSend[index++]=0xA3;//��������
  g_u8AB1769tcpSend[index++]=0x03;//����·����С
  g_u8AB1769tcpSend[index++]=0x01;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x20;
  g_u8AB1769tcpSend[index++]=0x02;
  g_u8AB1769tcpSend[index++]=0x24;
  g_u8AB1769tcpSend[index++]=0x01;//����·��
  DAQ_EthSend(0,g_u8AB1769tcpSend,index);
}

void ReadPLC_Name(u8 *sessionhandle)
{
  u8 index;
  index=0;
  memset(g_u8AB1769tcpSend,0,ABCOMPACT_TCP_SEND_LENGTH);
  g_u8AB1769tcpSend[index++]=0x70;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x2a;
  g_u8AB1769tcpSend[index++]=0x00;
  memcpy(&g_u8AB1769tcpSend[index],sessionhandle,4);//�Ự���
  index =index+4;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//״̬��Ĭ��Ϊ0x00000000
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//���ͷ�����
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//ѡ�Ĭ��Ϊ0x00000000
  //����������ָ������(command specific data)
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//�ӿھ����Ĭ��Ϊ0x0000
  g_u8AB1769tcpSend[index++]=0x01;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x02;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0xA1;
  g_u8AB1769tcpSend[index++]=0x00;//
  g_u8AB1769tcpSend[index++]=0x04;
  g_u8AB1769tcpSend[index++]=0x00;//���ȣ���һ�����ӱ�ʶ���ֽڳ��ȣ�Ҫ����ĳ�
  memcpy(&g_u8AB1769tcpSend[index],O_T_NetworkConnectID,4);//���ӱ�ʶ���ʹ�Ӧ���е�O-T Network Connecton IDһ��
  index = index+4;
  g_u8AB1769tcpSend[index++]=0xB1;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x16;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=FrameCount%256;
  g_u8AB1769tcpSend[index++]=FrameCount/256;
  g_u8AB1769tcpSend[index++]=0x55;//�����ʶ��
  g_u8AB1769tcpSend[index++]=0x03;//����·����С���������3���ֳ�
  g_u8AB1769tcpSend[index++]=0x20;
  g_u8AB1769tcpSend[index++]=0x6B;//�߼���
  g_u8AB1769tcpSend[index++]=0x25;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//ʵ����
  g_u8AB1769tcpSend[index++]=0x05;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x02;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x07;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x08;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x01;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x0a;
  g_u8AB1769tcpSend[index++]=0x00;////
  DAQ_EthSend(0,g_u8AB1769tcpSend,index);
  FrameCount++;
}
/**********************************************/
/*************     ����ǩ    ******************/
/**********************************************/
void ABCompact_tcp_read(u8 *lable,u8 lable_len )
{
	u8 index,sendnum;
  index=0;
	sendnum=20+PLC_ProgNameLenth+PLC_ProgNameLenth%2+lable_len+lable_len%2+10;
  memset(g_u8AB1769tcpSend,0,ABCOMPACT_TCP_SEND_LENGTH);
  g_u8AB1769tcpSend[index++]=0x70;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=sendnum%256;
  g_u8AB1769tcpSend[index++]=sendnum/256;
  memcpy(&g_u8AB1769tcpSend[index],Register_SessionHandle,4);//�Ự���
  index =index+4;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//״̬��Ĭ��Ϊ0x00000000
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//���ͷ�����
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//ѡ�Ĭ��Ϊ0x00000000
  //����������ָ������(command specific data)
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//�ӿھ����Ĭ��Ϊ0x0000
  g_u8AB1769tcpSend[index++]=0x01;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x02;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0xA1;
  g_u8AB1769tcpSend[index++]=0x00;//
  g_u8AB1769tcpSend[index++]=0x04;
  g_u8AB1769tcpSend[index++]=0x00;//���ȣ���һ�����ӱ�ʶ���ֽڳ��ȣ�Ҫ����ĳ�
  memcpy(&g_u8AB1769tcpSend[index],O_T_NetworkConnectID,4);//���ӱ�ʶ���ʹ�Ӧ���е�O-T Network Connecton IDһ��
  index = index+4;
  g_u8AB1769tcpSend[index++]=0xB1;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=(sendnum-20)%256;
  g_u8AB1769tcpSend[index++]=(sendnum-20)/256;//������ݰ��ĳ���
  g_u8AB1769tcpSend[index++]=FrameCount%256;
  g_u8AB1769tcpSend[index++]=FrameCount/256;//��֡���
  g_u8AB1769tcpSend[index++]=0x4C;//�����ʶ��
  g_u8AB1769tcpSend[index++]=(4+PLC_ProgNameLenth+PLC_ProgNameLenth%2+lable_len+lable_len%2)/2;//����·����С���������3���ֳ�
  g_u8AB1769tcpSend[index++]=0x91;//��չ��
  g_u8AB1769tcpSend[index++]=PLC_ProgNameLenth;//PLC���������ֽڳ���
  memcpy(&g_u8AB1769tcpSend[index],PLC_ProgramName,PLC_ProgNameLenth);//PLC������
	index = index + PLC_ProgNameLenth;
	if((PLC_ProgNameLenth%2)!=0)
	{
		g_u8AB1769tcpSend[index++]=0x00;
	}
  g_u8AB1769tcpSend[index++]=0x91;//��չ��
  g_u8AB1769tcpSend[index++]=lable_len;//��ǩ������
  memcpy(&g_u8AB1769tcpSend[index],lable,lable_len);//PLC������
	index = index + lable_len;
	if((lable_len%2)!=0)
	{
		g_u8AB1769tcpSend[index++]=0x00;
	}
  g_u8AB1769tcpSend[index++]=0x01;
  g_u8AB1769tcpSend[index++]=0x00;//ָ������
  DAQ_EthSend(0,g_u8AB1769tcpSend,index);
  FrameCount++;
}

/**********************************************/
/*************     д��ǩ    ******************/
/**********************************************/
void ABCompact_tcp_Write(u8 *lable,u8 lable_len )
{
	u8 index,sendnum,datatype;
  index=0;
  switch(RegisterCfgBuff[AB_Compact_tcp][g_u8_RespondID].regtype)
	{
		case LABLE_BOOL:datatype=1;break;
		case LABLE_BYTE:datatype=1;break;
		case LABLE_SHORT:datatype=2;break;
		case LABLE_LONG:datatype=4;break;
		case LABLE_FLOAT:datatype=4;break;
		default:break;
	}
	sendnum=20+4+PLC_ProgNameLenth+PLC_ProgNameLenth%2+4+lable_len+lable_len%2+4+datatype;
  memset(g_u8AB1769tcpSend,0,ABCOMPACT_TCP_SEND_LENGTH);
  g_u8AB1769tcpSend[index++]=0x70;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=sendnum%256;
  g_u8AB1769tcpSend[index++]=sendnum/256;
  memcpy(&g_u8AB1769tcpSend[index],Register_SessionHandle,4);//�Ự���
  index =index+4;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//״̬��Ĭ��Ϊ0x00000000
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//���ͷ�����
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//ѡ�Ĭ��Ϊ0x00000000
  //����������ָ������(command specific data)
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x00;//�ӿھ����Ĭ��Ϊ0x0000
  g_u8AB1769tcpSend[index++]=0x01;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0x02;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=0xA1;
  g_u8AB1769tcpSend[index++]=0x00;//
  g_u8AB1769tcpSend[index++]=0x04;
  g_u8AB1769tcpSend[index++]=0x00;//���ȣ���һ�����ӱ�ʶ���ֽڳ��ȣ�Ҫ����ĳ�
  memcpy(&g_u8AB1769tcpSend[index],O_T_NetworkConnectID,4);//���ӱ�ʶ���ʹ�Ӧ���е�O-T Network Connecton IDһ��
  index = index+4;
  g_u8AB1769tcpSend[index++]=0xB1;
  g_u8AB1769tcpSend[index++]=0x00;
  g_u8AB1769tcpSend[index++]=(sendnum-20)%256;
  g_u8AB1769tcpSend[index++]=(sendnum-20)/256;//������ݰ��ĳ���
  g_u8AB1769tcpSend[index++]=FrameCount%256;
  g_u8AB1769tcpSend[index++]=FrameCount/256;//��֡���
  g_u8AB1769tcpSend[index++]=0x4D;//�����ʶ��
  g_u8AB1769tcpSend[index++]=(4+PLC_ProgNameLenth+PLC_ProgNameLenth%2+lable_len+lable_len%2)/2;//����·����С;//����·����С���������3���ֳ�
  g_u8AB1769tcpSend[index++]=0x91;//��չ��
  g_u8AB1769tcpSend[index++]=PLC_ProgNameLenth;//PLC���������ֽڳ���
  memcpy(&g_u8AB1769tcpSend[index],PLC_ProgramName,PLC_ProgNameLenth);//PLC������
	index = index + PLC_ProgNameLenth;
	if((PLC_ProgNameLenth%2)!=0)
	{
		g_u8AB1769tcpSend[index++]=0x00;
	}
  g_u8AB1769tcpSend[index++]=0x91;//��չ��
  g_u8AB1769tcpSend[index++]=lable_len;//��ǩ������
  memcpy(&g_u8AB1769tcpSend[index],lable,lable_len);//PLC������
	index = index + lable_len;
	if((lable_len%2)!=0)
	{
		g_u8AB1769tcpSend[index++]=0x00;
	}
	switch(RegisterCfgBuff[AB_Compact_tcp][g_u8_RespondID].regtype)
	{
		case LABLE_BOOL:datatype=0x00C1;break;
		case LABLE_BYTE:datatype=0x00C2;break;
		case LABLE_SHORT:datatype=0x00C3;break;
		case LABLE_LONG:datatype=0x00C4;break;
		case LABLE_FLOAT:datatype=0x00CA;break;
		default:break;
	}
  g_u8AB1769tcpSend[index++]=datatype%256;
  g_u8AB1769tcpSend[index++]=datatype/256;//��������
	g_u8AB1769tcpSend[index++]=0x01;
  g_u8AB1769tcpSend[index++]=0x00;//ָ������
	switch(RegisterCfgBuff[AB_Compact_tcp][g_u8_RespondID].regtype)
	{
		case LABLE_BOOL:
			g_u8AB1769tcpSend[index++]=g_u8_Writedata[1];
			break;
		case LABLE_BYTE:
			g_u8AB1769tcpSend[index++]=g_u8_Writedata[1];
			break;
		case LABLE_SHORT:
			g_u8AB1769tcpSend[index++]=g_u8_Writedata[1];
			g_u8AB1769tcpSend[index++]=g_u8_Writedata[0];
			break;
		case LABLE_LONG:
			g_u8AB1769tcpSend[index++]=g_u8_Writedata[3];
			g_u8AB1769tcpSend[index++]=g_u8_Writedata[2];
			g_u8AB1769tcpSend[index++]=g_u8_Writedata[1];
			g_u8AB1769tcpSend[index++]=g_u8_Writedata[0];
			break;
		case LABLE_FLOAT:
			g_u8AB1769tcpSend[index++]=g_u8_Writedata[2];
			g_u8AB1769tcpSend[index++]=g_u8_Writedata[3];
			g_u8AB1769tcpSend[index++]=g_u8_Writedata[0];
			g_u8AB1769tcpSend[index++]=g_u8_Writedata[1];
			break;
		default:break;
	}
  DAQ_EthSend(0,g_u8AB1769tcpSend,index);
  FrameCount++;
}

void f_AB_Compact_tcp_task(void)
{
	u16 size=0, i = 0;
	u32 dataaddr;
	u16 lable_type;
	
	
	mb_tcp_master_W5500_Socket_Set(0);//�������
  mb_tcp_master_W5500_Interrupt_Process(0);//W5500�жϴ��������

	
	memset(g_u8AB1769ReceData,0,ABCOMPACT_TCP_RECE_LENGTH);
  size=MasterRead_SOCK_Data_Buffer(0,g_u8AB1769ReceData);

	if(size)
	{		
    ConnectTime=0;
    if((AB_Compact_Status==Login_Request)&&
      (g_u8AB1769ReceData[0]==0x65)&&(g_u8AB1769ReceData[1]==0x00))
    {
      if((g_u8AB1769ReceData[9]==0x00)&&(g_u8AB1769ReceData[10]==0x00)&&
        (g_u8AB1769ReceData[11]==0x00)&&(g_u8AB1769ReceData[12]==0x00))//�ж�״̬�Ƿ�Ϊ0
      {
        memcpy(Register_SessionHandle,&g_u8AB1769ReceData[4],4);
        PLC_Respond =ON;
      }
    }
    else if((AB_Compact_Status==Open_Request)&&
            (g_u8AB1769ReceData[0]==0x6F)&&(g_u8AB1769ReceData[1]==0x00))
    {
      if((g_u8AB1769ReceData[4]==Register_SessionHandle[0])&&(g_u8AB1769ReceData[5]==Register_SessionHandle[1])&&
        (g_u8AB1769ReceData[6]==Register_SessionHandle[2])&&(g_u8AB1769ReceData[7]==Register_SessionHandle[3]))//
      {
        memcpy(O_T_NetworkConnectID,&g_u8AB1769ReceData[44],4);
        PLC_Respond = ON;
      }
		}
		else if((AB_Compact_Status==Read_PLCName)&&
            (g_u8AB1769ReceData[0]==0x70)&&(g_u8AB1769ReceData[1]==0x00))
    {
      if((g_u8AB1769ReceData[4]==Register_SessionHandle[0])&&(g_u8AB1769ReceData[5]==Register_SessionHandle[1])&&
        (g_u8AB1769ReceData[6]==Register_SessionHandle[2])&&(g_u8AB1769ReceData[7]==Register_SessionHandle[3]))//
      {
        for(i=Off_PLCName;i<(Off_PLCName+REG_ADDRSIZE);i++)
        {
          if(g_u8AB1769ReceData[i]==0x00)
          {
            break;
          }
          else if(g_u8AB1769ReceData[i]=='$')
          {
            break;
          }
          else
          {

          }
        }
				PLC_ProgNameLenth=i-Off_PLCName;
        memcpy(PLC_ProgramName,&g_u8AB1769ReceData[Off_PLCName],PLC_ProgNameLenth);
        PLC_Respond = ON;
      }
    }
    else
    {

    }
		if(GW_WriteStatus(AB_Compact_tcp)==WRITE_WAITRESPOND)
		{
					GW_WriteStatus(AB_Compact_tcp)=WRITE_RECVSUCCESS;	
          memcpy(&g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_RespondID]*2],
                  &g_u8_Writedata,g_u16_WriteLen*2);
		}
	  else if(GW_ReadStatus(AB_Compact_tcp)==READ_WAITRESPOND)
		{
			if((g_u8AB1769ReceData[4]==Register_SessionHandle[0])&&(g_u8AB1769ReceData[5]==Register_SessionHandle[1])&&
        (g_u8AB1769ReceData[6]==Register_SessionHandle[2])&&(g_u8AB1769ReceData[7]==Register_SessionHandle[3]))//
      {
				if((g_u8AB1769ReceData[48]==0x00)&&(g_u8AB1769ReceData[49]==0x00))
				{
					GW_ReadStatus(AB_Compact_tcp)=READ_RECVSUCCESS;
					lable_type = g_u8AB1769ReceData[50]+g_u8AB1769ReceData[51]*256;
					if(lable_type==0x00C1)//bool��
					{
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2]=0x00;
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2+1]=
							g_u8AB1769ReceData[52];
						g_u16_RecvTransLen[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]= 2;
					}
					else if(lable_type==0x00C2)//�ֽ���
					{
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2]=0x00;
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2+1]=
							g_u8AB1769ReceData[52];
						g_u16_RecvTransLen[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]= 2;
					}
					else if(lable_type==0x00C3)//����
					{
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2]=
							g_u8AB1769ReceData[53];
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2+1]=
							g_u8AB1769ReceData[52];
						g_u16_RecvTransLen[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]= 2;
					}
					else if(lable_type==0x00C4)//long��
					{
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2]=
							g_u8AB1769ReceData[55];
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2+1]=
							g_u8AB1769ReceData[54];
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2+2]=
							g_u8AB1769ReceData[53];
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2+3]=
							g_u8AB1769ReceData[52];
						g_u16_RecvTransLen[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]= 4;
					}
					else if(lable_type==0x00CA)//ʵ��
					{
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2]=
							g_u8AB1769ReceData[55];
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2+1]=
							g_u8AB1769ReceData[54];
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2+2]=
							g_u8AB1769ReceData[53];
						g_u8_EthRespData[AB_Compact_tcp][g_u16_StartAddr[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]*2+3]=
							g_u8AB1769ReceData[52];
						g_u16_RecvTransLen[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]]= 4;
					}
					else
					{
						
					}
				}	
			}
		}
		else { 
			 //NegativeResponse(Err_MBcmd);
		}		  
	}

	if(PLC_Respond == ON)
  {
    PLC_Respond =OFF;
    switch(AB_Compact_Status)
    {
      case PLC_Idle:
                    AB_Compact_Status=Login_Request;
                    LoginRequest(Default_SessionHandle);
                    break;
      case Login_Request:
                    AB_Compact_Status=Open_Request;
                    OpenRequest(Register_SessionHandle);
                    break;
      case Open_Request:
                    AB_Compact_Status=Read_PLCName;
                    ReadPLC_Name(Register_SessionHandle);
                    break;
      case Read_PLCName:
                    AB_Compact_Status=NormalMode;
                    break;
      case NormalMode:
                    
                    break;
      default:
                    AB_Compact_Status=Login_Request;
                    LoginRequest(Default_SessionHandle);
                    break;
    }
  }

	if(ThreadNew(AB_Compact_tcp) == ON)
	{
		ThreadNew(AB_Compact_tcp) = OFF;
		GW_ReadStatus(AB_Compact_tcp)=READ_WAITRESPOND;
    ABCompact_tcp_read(RegisterCfgBuff[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]].regaddr,
      RegisterCfgBuff[AB_Compact_tcp][g_u8_threadIdx[AB_Compact_tcp]].plcaddrstation);
	}
  else if(WRITE_PRESEND==GW_WriteStatus(AB_Compact_tcp))
  {
    GW_WriteStatus(AB_Compact_tcp)=WRITE_WAITRESPOND;
    ABCompact_tcp_Write(RegisterCfgBuff[AB_Compact_tcp][g_u8_RespondID].regaddr,
      RegisterCfgBuff[AB_Compact_tcp][g_u8_RespondID].plcaddrstation);
  }
  else
  {
        ;
  }
}
