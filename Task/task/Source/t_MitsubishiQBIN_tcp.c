#include "main.h"
#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"	 
#include "m_spi_mb_tcp_master.h"	  
#include "m_spi_mb_tcp_master_cfg.h"
#include "tcp_master_mb.h"
#include "m_w5500.h"
#include "t_MitsubishiQBIN_tcp.h"
//#include "s7200_tcp.h" 

#define MITSUBISHIQBINTCP_SEND_LENGTH             220         //发送缓冲区大小
#define MITSUBISHIQBINTCP_RECE_LENGTH             220         //接受缓冲区大小

static u8 MitsubishiQBIN_tcp=0xff;
static u8 MitsubishiQBIN_tcp_Enable=OFF;;

static u8 g_u8MitsubishiQBINtcpSend[MITSUBISHIQBINTCP_SEND_LENGTH]; 
//static u8 g_u8MitsubishiQBINtcpSendw[20];
static u8 g_u8MitsubishiQBINReceData[MITSUBISHIQBINTCP_RECE_LENGTH];

void MitsubishiQBINtcp_Init(u8 nodeID)
{
  MitsubishiQBIN_tcp=nodeID;
  MitsubishiQBIN_tcp_Enable=ON;
  memset(g_u8MitsubishiQBINtcpSend,'0',MITSUBISHIQBINTCP_SEND_LENGTH);
  memset(g_u8MitsubishiQBINReceData,'0',MITSUBISHIQBINTCP_RECE_LENGTH);
}

/********************************************************************************************/	 
/********************************************************************************************/
void TIM_1ms_MitsubishiQBINtcp(void)
{
  if(MitsubishiQBIN_tcp_Enable==ON)
  {
    g_u16_SwitchTimer[MitsubishiQBIN_tcp]++;
  }
  if((ON==MB_NeedWrite(MitsubishiQBIN_tcp))&&(READ_IDLE==GW_ReadStatus(MitsubishiQBIN_tcp)))
	{ 	
    if(GW_WriteStatus(MitsubishiQBIN_tcp)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[MitsubishiQBIN_tcp]>1000)
	     {
	     		g_u16_SwitchTimer[MitsubishiQBIN_tcp]=0;
					GW_WriteStatus(MitsubishiQBIN_tcp)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(MitsubishiQBIN_tcp)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[MitsubishiQBIN_tcp]>WriteTime))
	     {	
		   		GW_WriteStatus(MitsubishiQBIN_tcp)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[MitsubishiQBIN_tcp]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(MitsubishiQBIN_tcp)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(MitsubishiQBIN_tcp)=WRITE_DELAY;
      g_u16_SwitchTimer[MitsubishiQBIN_tcp]=0;
	  }
	  else if(g_u16_SwitchTimer[MitsubishiQBIN_tcp]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[MitsubishiQBIN_tcp]=0;
      GW_WriteStatus(MitsubishiQBIN_tcp)=WRITE_IDLE;
	    MB_NeedWrite(MitsubishiQBIN_tcp)=OFF;
	    g_u8_threadIdx[MitsubishiQBIN_tcp]++;
		  ThreadNew(MitsubishiQBIN_tcp)=ON;
	  }
	  else{

	  }
	}
	else
	{
		if((g_u16_SwitchTimer[MitsubishiQBIN_tcp]>UpdateCycle[MitsubishiQBIN_tcp])&&
		    (g_u8_ProtocalNum[MitsubishiQBIN_tcp][READ]!=0))
		{	  
		  g_u16_SwitchTimer[MitsubishiQBIN_tcp]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(MitsubishiQBIN_tcp))
		  {
		    GW_ReadStatus(MitsubishiQBIN_tcp)=READ_WAITRESPOND;
		    g_u16_TimeoutCnt[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]]=0;
		    if(OFF==MB_NeedWrite(MitsubishiQBIN_tcp))
		    {
          g_u8_threadIdx[MitsubishiQBIN_tcp]++;
          while(RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[MitsubishiQBIN_tcp]++;
            if(g_u8_ProtocalNum[MitsubishiQBIN_tcp][READ]<=g_u8_threadIdx[MitsubishiQBIN_tcp])
      			{			  
      	      g_u8_threadIdx[MitsubishiQBIN_tcp]=0;
      			}
          }
		      ThreadNew(MitsubishiQBIN_tcp)=ON;
		    }
		    else
		    {
          GW_ReadStatus(MitsubishiQBIN_tcp)=READ_IDLE;
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]]++;
        if(g_u16_TimeoutCnt[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]]>
          (THRES_TIMOUTCNT/UpdateCycle[MitsubishiQBIN_tcp]))
        {
          g_u16_RecvTransLen[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]]=0;
          g_u16_TimeoutCnt[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]]=0;
          if(OFF==MB_NeedWrite(MitsubishiQBIN_tcp))
  		    {
            g_u8_threadIdx[MitsubishiQBIN_tcp]++;
            while(RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[MitsubishiQBIN_tcp]++;
              if(g_u8_ProtocalNum[MitsubishiQBIN_tcp][READ]<=g_u8_threadIdx[MitsubishiQBIN_tcp])
        			{			  
        	      g_u8_threadIdx[MitsubishiQBIN_tcp]=0;
        			}
            }
  		      ThreadNew(MitsubishiQBIN_tcp)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(MitsubishiQBIN_tcp)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[MitsubishiQBIN_tcp][READ]<=g_u8_threadIdx[MitsubishiQBIN_tcp])
	{			  
		g_u8_threadIdx[MitsubishiQBIN_tcp]=0;
	}
}

void MitsubishiQBIN_Write(unsigned char LeiXing, unsigned int  Adderadd, unsigned char l_u8Len)
{
  	u8 len=0;
	  u8 i = 0;

    memset(g_u8MitsubishiQBINtcpSend,'0',MITSUBISHIQBINTCP_SEND_LENGTH);
    g_u8MitsubishiQBINtcpSend[1]  = 0xff;
    g_u8MitsubishiQBINtcpSend[2]  = 0x0A;
    g_u8MitsubishiQBINtcpSend[3]  = 0x00;
    g_u8MitsubishiQBINtcpSend[4]  = Adderadd&0xff;
    g_u8MitsubishiQBINtcpSend[5]  = (Adderadd>>8)&0xff;
    g_u8MitsubishiQBINtcpSend[6]  = (Adderadd>>16)&0xff;
    g_u8MitsubishiQBINtcpSend[7]  = (Adderadd>>24)&0xff;

	if((LeiXing == 'x')||(LeiXing == 'X')) //x_R
	{
		g_u8MitsubishiQBINtcpSend[0]  = 0x02;
		g_u8MitsubishiQBINtcpSend[8] =  0x20;
		g_u8MitsubishiQBINtcpSend[9] =  'X';
		len=(l_u8Len/2)+(l_u8Len%2);
		for(i=0;i<l_u8Len;i++)
		{
			if(i == 0)
			{
				g_u8MitsubishiQBINtcpSend[12] = (g_u8_Writedata[i*2+1]<<4)&0xf0;
			}
			else
      {
				if((i % 2) != 0)
				{
					g_u8MitsubishiQBINtcpSend[12+i-1] = g_u8_Writedata[i*2+1] & 0x0f;
				}
				else
        {
					g_u8MitsubishiQBINtcpSend[12+i-1] = (g_u8_Writedata[i*2+1]<<4)&0xf0;
				}
			}
		}
	}
	else if((LeiXing == 'y')||(LeiXing == 'Y'))  //y_R
	{
		g_u8MitsubishiQBINtcpSend[0]  = 0x02;
		g_u8MitsubishiQBINtcpSend[8] =  0x20;
		g_u8MitsubishiQBINtcpSend[9] =  'Y';
		len=(l_u8Len/2)+(l_u8Len%2);
		for(i=0;i<l_u8Len;i++)
		{
			//tmp = read_shm(Writeshm_base,writecfg->mem_biase_write,i);
			if(i == 0)
			{
				g_u8MitsubishiQBINtcpSend[12] = (g_u8_Writedata[i*2+1]<<4)&0xf0;
			}
			else{
				if((i % 2) != 0)
				{
					g_u8MitsubishiQBINtcpSend[12+i-1] = g_u8_Writedata[i*2+1] & 0x0f;
				}
				else{
					g_u8MitsubishiQBINtcpSend[12+i-1] = (g_u8_Writedata[i*2+1]<<4)&0xf0;
				}
			}
		}
	}
	else if((LeiXing == 'm')||(LeiXing == 'M'))  //M
	{
		g_u8MitsubishiQBINtcpSend[0]  = 0x02;
		g_u8MitsubishiQBINtcpSend[8] =  0x20;
		g_u8MitsubishiQBINtcpSend[9] =  'M';
		len=(l_u8Len/2)+(l_u8Len%2);
		for(i=0;i<l_u8Len;i++)
		{
			//tmp = read_shm(Writeshm_base,writecfg->mem_biase_write,i);
			if(i == 0)
			{
				g_u8MitsubishiQBINtcpSend[12] = (g_u8_Writedata[i*2+1]<<4)&0xf0;
			}
			else{
				if((i % 2) != 0)
				{
					g_u8MitsubishiQBINtcpSend[12+i-1] = g_u8_Writedata[i*2+1] & 0x0f;
				}
				else{
					g_u8MitsubishiQBINtcpSend[12+i-1] = (g_u8_Writedata[i*2+1]<<4)&0xf0;
				}
			}
		}
	}
	else if((LeiXing == 's')||(LeiXing == 'S'))  //S
	{
		g_u8MitsubishiQBINtcpSend[0]  = 0x02;
		g_u8MitsubishiQBINtcpSend[8] =  0x20;
		g_u8MitsubishiQBINtcpSend[9] =  'S';
		len=(l_u8Len/2)+(l_u8Len%2);
		for(i=0;i<l_u8Len;i++)
		{
			//tmp = read_shm(Writeshm_base,writecfg->mem_biase_write,i);
			if(i == 0)
			{
				g_u8MitsubishiQBINtcpSend[12] = (g_u8_Writedata[i*2+1]<<4)&0xf0;
			}
			else{
				if((i % 2) != 0)
				{
					g_u8MitsubishiQBINtcpSend[12+i-1] = g_u8_Writedata[i*2+1] & 0x0f;
				}
				else{
					g_u8MitsubishiQBINtcpSend[12+i-1] = (g_u8_Writedata[i*2+1]<<4)&0xf0;
				}
			}
		}
	}
	else if((LeiXing == 'd')||(LeiXing == 'D'))  //D_R
	{
		len=l_u8Len*2;
		g_u8MitsubishiQBINtcpSend[0]  = 0x03;
		g_u8MitsubishiQBINtcpSend[8] =  0x20;
		g_u8MitsubishiQBINtcpSend[9] =  'D';
		for(i=0;i<l_u8Len;i++)
		{
			//tmp = read_shm(Writeshm_base,writecfg->mem_biase_write,i);
			g_u8MitsubishiQBINtcpSend[12+2*i+0] =  g_u8_Writedata[i*2+1]&0xff;
			g_u8MitsubishiQBINtcpSend[12+2*i+1] =  g_u8_Writedata[i*2+0]&0xff;
		}
	}
  else if((LeiXing == 't')||(LeiXing == 'T'))  //T_N
  {
	  len=l_u8Len*2;
	  g_u8MitsubishiQBINtcpSend[0]  = 0x03;
	  g_u8MitsubishiQBINtcpSend[8] =  'N';
	  g_u8MitsubishiQBINtcpSend[9] =  'T';
		for(i=0;i<l_u8Len;i++)
		{
			//tmp = read_shm(Writeshm_base,writecfg->mem_biase_write,i);
			g_u8MitsubishiQBINtcpSend[12+2*i+0] =  g_u8_Writedata[i*2+1]&0xff;
			g_u8MitsubishiQBINtcpSend[12+2*i+1] =  g_u8_Writedata[i*2+0]&0xff;
		}
	}
	else if((LeiXing == 'c')||(LeiXing == 'C'))  //C_N
	{
		len=l_u8Len*2;
		g_u8MitsubishiQBINtcpSend[0]  = 0x03;
		g_u8MitsubishiQBINtcpSend[8] =  'C';
		g_u8MitsubishiQBINtcpSend[9] =  'N';
		for(i=0;i<l_u8Len;i++)
		{
			//tmp = read_shm(Writeshm_base,writecfg->mem_biase_write,i);
			g_u8MitsubishiQBINtcpSend[12+2*i+0] =  g_u8_Writedata[i*2+1]&0xff;
			g_u8MitsubishiQBINtcpSend[12+2*i+1] =  g_u8_Writedata[i*2+0]&0xff;
		}
	}
	else
	{

	}
	g_u8MitsubishiQBINtcpSend[10]=l_u8Len;
	g_u8MitsubishiQBINtcpSend[11]=0x00;
	len = len + 12;
	DAQ_EthSend(0,g_u8MitsubishiQBINtcpSend,len);
}

void MitsubishiQBIN_read(u8 LeiXing, u32 Adderadd,u8 send_len)
{
  u8 len = 0;
  memset(g_u8MitsubishiQBINtcpSend,'0',MITSUBISHIQBINTCP_SEND_LENGTH);
	g_u8MitsubishiQBINtcpSend[1]  = 0xff;
	g_u8MitsubishiQBINtcpSend[2]  = 0x0A;
	g_u8MitsubishiQBINtcpSend[3]  = 0x00;
	g_u8MitsubishiQBINtcpSend[4]  = Adderadd&0xff;
	g_u8MitsubishiQBINtcpSend[5]  = (Adderadd>>8)&0xff;
	g_u8MitsubishiQBINtcpSend[6]  = (Adderadd>>16)&0xff;
	g_u8MitsubishiQBINtcpSend[7]  = (Adderadd>>24)&0xff;

	if((LeiXing == 'x')||(LeiXing == 'X')) //x_R
	{
    g_u8MitsubishiQBINtcpSend[0]  = 0x00;
		g_u8MitsubishiQBINtcpSend[8] =  0x20;
    g_u8MitsubishiQBINtcpSend[9] =  'X';
	}
	else if((LeiXing == 'y')||(LeiXing == 'Y'))  //y_R
	{
    g_u8MitsubishiQBINtcpSend[0]  = 0x00;
		g_u8MitsubishiQBINtcpSend[8] =  0x20;
    g_u8MitsubishiQBINtcpSend[9] =  'Y';
	}
	else if((LeiXing == 'm')||(LeiXing == 'M'))  //M
	{
    g_u8MitsubishiQBINtcpSend[0]  = 0x00;
		g_u8MitsubishiQBINtcpSend[8] =  0x20;
    g_u8MitsubishiQBINtcpSend[9] =  'M';
	}
	else if((LeiXing == 'm')||(LeiXing == 'S'))  //S
	{
    g_u8MitsubishiQBINtcpSend[0]  = 0x00;
		g_u8MitsubishiQBINtcpSend[8] =  0x20;
    g_u8MitsubishiQBINtcpSend[9] =  'S';
	}
	else if((LeiXing == 'd')||(LeiXing == 'D'))  //D_R
	{
		g_u8MitsubishiQBINtcpSend[0]  = 0x01;
    g_u8MitsubishiQBINtcpSend[8] =  0x20;
    g_u8MitsubishiQBINtcpSend[9] =  'D';

	}
  else if((LeiXing == 't')||(LeiXing == 'T'))  //T_N
	{
    g_u8MitsubishiQBINtcpSend[0]  = 0x01;
		g_u8MitsubishiQBINtcpSend[8] =  'N';
    g_u8MitsubishiQBINtcpSend[9] =  'T';
	}
	else if((LeiXing == 'c')||(LeiXing == 'C'))  //C_N
	{
		g_u8MitsubishiQBINtcpSend[0]  = 0x01;
		g_u8MitsubishiQBINtcpSend[8] =  'C';
		g_u8MitsubishiQBINtcpSend[9] =  'N';
	}
	else
	{

	}
	g_u8MitsubishiQBINtcpSend[10]  = send_len;
	g_u8MitsubishiQBINtcpSend[11]  = 0x00;
	len =12;
  DAQ_EthSend(0,g_u8MitsubishiQBINtcpSend,len);
}


void MitsubishiQBINtcp_RecDataHandle(u8 regtype,u8 *sourcebuf,u8 *targetbuf,u8 datalen)
{
  
  u8 i = 0 ;
//	u32 num = 0;
	u16 tmp = 0;

//   num=(RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].regaddr[5]<<16)
//         +(RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].regaddr[6]<<8)
//         + RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].regaddr[7] ;
	if((regtype=='X')||(regtype=='Y')||(regtype=='M')||(regtype=='S'))
	{
		for(i = 0;i < datalen;i++)
		{
      if(i == 0)
      {
        tmp = (sourcebuf[0]>>4)&0x0f;
      }
      else
      {
        if((i % 2) != 0)
        {
          tmp = sourcebuf[i/2]&0x0f;
        }
        else
        {
          tmp = (sourcebuf[i/2]>>4)&0x0f;
        }
      }
      targetbuf[2*i+0]=0x00;
      targetbuf[2*i+1]=tmp;
		}
	}

	else if((regtype=='D')||(regtype=='T')||(regtype=='C'))
	{
    for(i = 0;i < datalen/2;i++)
    {
      targetbuf[2*i+0]=sourcebuf[1+i*2];
      targetbuf[2*i+1]=sourcebuf[2*i];
    }
	}
}

void f_MitsubishiQBINtcp_task(void)
{
	u16 size=0, i = 0;
	u32 dataaddr;
	
	
	mb_tcp_master_W5500_Socket_Set(0);//检查连接
  mb_tcp_master_W5500_Interrupt_Process(0);//W5500中断处理程序框架

	memset(g_u8MitsubishiQBINReceData,'0',MITSUBISHIQBINTCP_RECE_LENGTH);
  size=MasterRead_SOCK_Data_Buffer(0,g_u8MitsubishiQBINReceData);

	if(size)
	{		
		if((g_u8MitsubishiQBINReceData[0]==0x81) || (g_u8MitsubishiQBINReceData[0]==0x83) || (g_u8MitsubishiQBINReceData[0]==0x80) || (g_u8MitsubishiQBINReceData[0]==0x82))
		{

      if((GW_WriteStatus(MitsubishiQBIN_tcp)==WRITE_WAITRESPOND)&&(g_u8MitsubishiQBINReceData[1]==0x00))
      {
        GW_WriteStatus(MitsubishiQBIN_tcp)=WRITE_RECVSUCCESS;	
        memcpy(&g_u8_EthRespData[MitsubishiQBIN_tcp][(g_u16_StartAddr[MitsubishiQBIN_tcp][g_u8_RespondID]+
                  g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
      }
      else if(GW_ReadStatus(MitsubishiQBIN_tcp)==READ_WAITRESPOND)
      {
        if(g_u8MitsubishiQBINReceData[1]==0X00)
        {
          GW_ReadStatus(MitsubishiQBIN_tcp)=READ_RECVSUCCESS;
          if((RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].regtype=='X')||
              (RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].regtype=='Y')||
              (RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].regtype=='M')||
              (RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].regtype=='S'))
          {
          /*	size=RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].datalenth/2+
                RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].datalenth%2;*/
            size=RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].datalenth;
          }
          else
          {
            size=RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].datalenth*2;
          }
          MitsubishiQBINtcp_RecDataHandle(RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].regtype,
              &g_u8MitsubishiQBINReceData[2],
              &g_u8_EthRespData[MitsubishiQBIN_tcp][g_u16_StartAddr[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]]*2],
              size);
          g_u16_RecvTransLen[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]] = RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].datalenth * 2 ; 	
        }
      }
      else 
      {
         NegativeResponse(Err_MBcmd);
         memset(g_u8MitsubishiQBINReceData,0,MITSUBISHIQBINTCP_RECE_LENGTH);
      }		  
	 }
}
	if(ThreadNew(MitsubishiQBIN_tcp) == ON)
	{
		ThreadNew(MitsubishiQBIN_tcp) = OFF;
		GW_ReadStatus(MitsubishiQBIN_tcp)=READ_WAITRESPOND;
    dataaddr=(RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].regaddr[5]<<16)
			+(RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].regaddr[6]<<8)
      + RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].regaddr[7] ;
    MitsubishiQBIN_read(RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].regtype,dataaddr,
      RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_threadIdx[MitsubishiQBIN_tcp]].datalenth);
	}	
  else if(WRITE_PRESEND==GW_WriteStatus(MitsubishiQBIN_tcp))
  {
    GW_WriteStatus(MitsubishiQBIN_tcp)=WRITE_WAITRESPOND;
    dataaddr=(g_u8_WriteAddr[5]<<16) + (g_u8_WriteAddr[6]<<8) + g_u8_WriteAddr[7];
    MitsubishiQBIN_Write(RegisterCfgBuff[MitsubishiQBIN_tcp][g_u8_RespondID].regtype,
      dataaddr,g_u16_WriteLen);	
  }
  else
  {
        ;
  }
}
