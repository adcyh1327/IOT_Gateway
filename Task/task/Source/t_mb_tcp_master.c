#include "main.h"
#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"	 
#include "m_spi_mb_tcp_master.h"	  
#include "m_spi_mb_tcp_master_cfg.h"
#include "tcp_master_mb.h"
#include "m_w5500.h"

#define MBTCP_SEND_LENGTH            120          //���ͻ�������С
#define MBTCP_RECE_LENGTH            120         //���ܻ�������С

static u32 mb_tcp_num_head=0;
static u8 MBsendbufer[MBTCP_SEND_LENGTH];
static u8 MBRecbufer[MBTCP_RECE_LENGTH];
static u8 Modbus_TCP=0xff;;
static u8 Modbustcp_Enable=OFF;

void ModbusTCp_Init(u8 nodeID)
{
  mb_tcp_num_head=0;
  Modbus_TCP=nodeID;
  Modbustcp_Enable=ON;
}

void md_tcp_master_Read(u8 func_code,u16 addr, u8 len)
{
	mb_tcp_num_head++;
	func_code=(func_code==0)? FUNC_RD_HOLDREG : func_code ;
	MBsendbufer[0] = 	mb_tcp_num_head/256;
	MBsendbufer[1] = 	mb_tcp_num_head%256;
	MBsendbufer[2] = 	0x00;
	MBsendbufer[3] = 	0x00;
	MBsendbufer[4] = 	0x00;	   //���� ���ֽ�
	MBsendbufer[5] = 	0x06;	   //���� ���ֽ�
	MBsendbufer[6] = 	RegisterCfgBuff[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]].plcaddrstation; 				//ID��
	MBsendbufer[7] = 	func_code; 				//������
	MBsendbufer[8] = 	addr/256;				//��ʼ��ַ���ֽ�
	MBsendbufer[9] = 	addr%256;				//��ʼ��ַ���ֽ�
	MBsendbufer[10] = 0x00;																				 //�����ݳ��ȸ��ֽ�
	MBsendbufer[11] = len;				 //�����ݳ��ȵ��ֽ�
	DAQ_EthSend(0,MBsendbufer, 12);
}

void TIM_1ms_MBTCPMaster(void)
{
  if(Modbustcp_Enable==ON)
  {
  	g_u16_SwitchTimer[Modbus_TCP]++;
  }
  if((ON==MB_NeedWrite(Modbus_TCP))&&(READ_IDLE==GW_ReadStatus(Modbus_TCP)))
	{ 	
    if(GW_WriteStatus(Modbus_TCP)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[Modbus_TCP]>1000)
	     {
	     		g_u16_SwitchTimer[Modbus_TCP]=0;
					GW_WriteStatus(Modbus_TCP)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(Modbus_TCP)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[Modbus_TCP]>WriteTime))
	     {	
		   		GW_WriteStatus(Modbus_TCP)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[Modbus_TCP]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(Modbus_TCP)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(Modbus_TCP)=WRITE_DELAY;
      g_u16_SwitchTimer[Modbus_TCP]=0;
	  }
	  else if(g_u16_SwitchTimer[Modbus_TCP]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[Modbus_TCP]=0;
      GW_WriteStatus(Modbus_TCP)=WRITE_IDLE;
	    MB_NeedWrite(Modbus_TCP)=OFF;
	    g_u8_threadIdx[Modbus_TCP]++;
		  ThreadNew(Modbus_TCP)=ON;
	  }
	  else{

	  }
	}
	else
	{
		if((g_u16_SwitchTimer[Modbus_TCP]>UpdateCycle[Modbus_TCP])&&
		    (g_u8_ProtocalNum[Modbus_TCP][READ]!=0))
		{	  
		  g_u16_SwitchTimer[Modbus_TCP]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(Modbus_TCP))
		  {
		    GW_ReadStatus(Modbus_TCP)=READ_IDLE;
		    g_u16_TimeoutCnt[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]]=0;
		    if(OFF==MB_NeedWrite(Modbus_TCP))
		    {
          g_u8_threadIdx[Modbus_TCP]++;
          while(RegisterCfgBuff[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[Modbus_TCP]++;
            if(g_u8_ProtocalNum[Modbus_TCP][READ]<=g_u8_threadIdx[Modbus_TCP])
      			{			  
      	      g_u8_threadIdx[Modbus_TCP]=0;
      			}
          }
		      ThreadNew(Modbus_TCP)=ON;
		    }
		    else
		    {
          
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]]++;
        if(g_u16_TimeoutCnt[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]]>
          (THRES_TIMOUTCNT/UpdateCycle[Modbus_TCP]))
        {
          g_u16_RecvTransLen[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]]=0;
          g_u16_TimeoutCnt[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]]=0;
          if(OFF==MB_NeedWrite(Modbus_TCP))
  		    {
            g_u8_threadIdx[Modbus_TCP]++;
            while(RegisterCfgBuff[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[Modbus_TCP]++;
              if(g_u8_ProtocalNum[Modbus_TCP][READ]<=g_u8_threadIdx[Modbus_TCP])
        			{			  
        	      g_u8_threadIdx[Modbus_TCP]=0;
        			}
            }
  		      ThreadNew(Modbus_TCP)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(Modbus_TCP)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[Modbus_TCP][READ]<=g_u8_threadIdx[Modbus_TCP])
	{			  
		g_u8_threadIdx[Modbus_TCP]=0;
	}
}


void mb_tcp_slave_write(u8 func_code,u16 addr,u8 lenth)
{
	u8 index=0,i,temp,lenstd=0;
	mb_tcp_num_head++;
	func_code=(func_code==0)? FUNC_WR_MULREG : func_code ;
	MBsendbufer[index++] = 	mb_tcp_num_head/256;
	MBsendbufer[index++] = 	mb_tcp_num_head%256;
	MBsendbufer[index++] = 0x00;
	MBsendbufer[index++] = 0x00;
    if(func_code == FUNC_WR_SGCOIL)
    {
        MBsendbufer[index++] = 0;
        MBsendbufer[index++] = 0x06;
    }
    else
    {
        MBsendbufer[index++] = (7+lenth*2)/256;						  //��׼��modbus ���ͳ��ȸ��ֽ� 
        MBsendbufer[index++] = (7+lenth*2)%256;						  //��׼��modbus ���ͳ��ȵ��ֽ� 
	}
    MBsendbufer[index++] = RegisterCfgBuff[Modbus_TCP][g_u8_RespondID].plcaddrstation;	//��׼��modbus վ�� ID
	MBsendbufer[index++] = func_code;								  //��׼��modbus ������ д 0x10
	MBsendbufer[index++] = addr/256;						  //��׼��modbus Ҫд�ĵ�ַ���ֽ� 
	MBsendbufer[index++] = addr%256;						  //��׼��modbus Ҫд�ĵ�ַ���ֽ� 
	if(func_code == FUNC_WR_SGCOIL)
  {
    temp=2;
    lenstd = 10;
    g_u8_Writedata[0]=(g_u8_Writedata[1]==0x01)?0xff:0x00;
    g_u8_Writedata[1]=0x00;
  }
	else if(func_code == FUNC_WR_MULCOIL)
  {
    temp = (lenth/8)+((lenth%8)==0?0:1);
    lenstd=13;
  }
  else if(func_code == FUNC_WR_MULREG)
  {
    MBsendbufer[index++] =  lenth/256;
    MBsendbufer[index++] =  lenth%256;
    temp = lenth * 2;
    MBsendbufer[index++] = temp;
    lenstd=13;
  }
  else
  {
    temp = lenth * 2;
  }
  for(i=0;i<temp;i++)
	{
    MBsendbufer[index++] =  g_u8_Writedata[i];
	}
	
  DAQ_EthSend(0,MBsendbufer,lenth*2+lenstd);
}

/*******************************************************************************
* ������  : mb_tcp_master_RecData_translate
* ����    : ������յ����ݣ�����ת��Ϊ�Լ���Э���ʽ
* ����    : sourcebuf:Դ���ݻ������� targetbuf:Ŀ�����ݻ�������datalen:���ݳ���
* ���    : ��
* ����ֵ  : ��
* ˵��    : �յ����Ǵ��豸���ı�׼modbus���ݣ��п�������ɢ�ģ�ͨ�����������ϵ������ĵ�ַ�ռ䡣
*******************************************************************************/
static void mb_tcp_master_RecData_translate(u8 *sourcebuf,u8 *targetbuf,u8 datalen)
{
	u8 i,j,loc;
	loc = RegisterCfgBuff[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]].datalenth*2;
	if((RegisterCfgBuff[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]].specfuncode==FUNC_RD_COILSTATUS)||
			(RegisterCfgBuff[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]].specfuncode==FUNC_RD_INPUTSTATUS))
	{
		for(i=0;i<sourcebuf[2];i++)
		{
			for(j=0;j<8;j++)
			{
				g_u8_EthRespData[Modbus_TCP][(g_u16_StartAddr[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]]*2)+(8*2*i)+(2*j)]=0x00;
				g_u8_EthRespData[Modbus_TCP][(g_u16_StartAddr[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]]*2)+(8*2*i)+(2*j)+1]=
					(sourcebuf[3+i]>>j)&0x01;
			}
		}
	}
	else
	{
		memcpy(&g_u8_EthRespData[Modbus_TCP][g_u16_StartAddr[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]]*2],
						&sourcebuf[3],RegisterCfgBuff[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]].datalenth*2);
	}
	
	GW_ReadStatus(Modbus_TCP)=READ_RECVSUCCESS;
	g_u16_RecvTransLen[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]]= 
		RegisterCfgBuff[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]].datalenth*2;
}

void Ethernet_Recv_Process(void)
{
  unsigned short size;
  u8 i,buff[40];
  mb_tcp_master_W5500_Socket_Set(0);//�������
	mb_tcp_master_W5500_Interrupt_Process(0);//W5500�жϴ��������
	Delay1ms(10);	/* Wait for a moment */
	// ���͹���
	// ���չ���
	if((mb_tcp_master_Socket_Data[0]& S_RECEIVE) == S_RECEIVE)//���Socket0���յ�����
	{
		mb_tcp_master_Socket_Data[0]&=~S_RECEIVE;
		size=MasterRead_SOCK_Data_Buffer(0,MBRecbufer);
  	if(size==0) return;
  	if(GW_WriteStatus(Modbus_TCP)==WRITE_WAITRESPOND)
    {
      GW_WriteStatus(Modbus_TCP)=WRITE_RECVSUCCESS;
      memcpy(&g_u8_EthRespData[Modbus_TCP][(g_u16_StartAddr[Modbus_TCP][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
    }
    else
    {
      if((MBRecbufer[7]&FUNC_RESP_NEG)!=FUNC_RESP_NEG)
      {
        GW_ReadStatus(Modbus_TCP)=READ_RECVSUCCESS;
        for(i=0;i<size-6;i++)
        {
		      buff[i] = MBRecbufer[6+i];
		    }
        mb_tcp_master_RecData_translate(&MBRecbufer[6],
  			&g_u8_EthRespData[Modbus_TCP][g_u16_StartAddr[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]]*2],
  							size-6);
  		}
  		else
  		{
        NegativeResponse(MBRecbufer[8]);
  		}
    }
  	
	}
}


void f_mb_tcp_master_task(void)
{
	u16 dataaddr;
	//��ǰ���� �� û��д����
	Ethernet_Recv_Process();
	if(ThreadNew(Modbus_TCP) == ON)
	{
		ThreadNew(Modbus_TCP) = OFF;
		GW_ReadStatus(Modbus_TCP)=READ_WAITRESPOND;
    dataaddr=(RegisterCfgBuff[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]].regaddr[6]<<8)
      + RegisterCfgBuff[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]].regaddr[7];
    md_tcp_master_Read(RegisterCfgBuff[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]].specfuncode,
												dataaddr,RegisterCfgBuff[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]].datalenth);
	}	
  else if(WRITE_PRESEND==GW_WriteStatus(Modbus_TCP))
  {		   
    GW_WriteStatus(Modbus_TCP)=WRITE_WAITRESPOND;
    dataaddr=(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
    //mb_tcp_slave_write(RegisterCfgBuff[Modbus_TCP][g_u8_threadIdx[Modbus_TCP]].specfuncode,dataaddr,g_u16_WriteLen);
		switch(RegisterCfgBuff[Modbus_TCP][g_u8_RespondID].specfuncode)
    {
      case FUNC_RD_COILSTATUS:  
            mb_tcp_slave_write(FUNC_WR_SGCOIL,dataaddr,g_u16_WriteLen);
            break;
      case FUNC_RD_HOLDREG:  
            mb_tcp_slave_write(FUNC_WR_MULREG,dataaddr,g_u16_WriteLen);
            break;
      default:  
            mb_tcp_slave_write(FUNC_WR_MULREG,dataaddr,g_u16_WriteLen);
            break;
    }
  }
  else
  {

  }
}

