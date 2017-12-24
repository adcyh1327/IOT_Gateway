#include "main.h"
#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"	 
#include "m_spi_mb_tcp_master.h"	  
#include "m_spi_mb_tcp_master_cfg.h"
#include "tcp_master_mb.h"
#include "m_w5500.h"
#include "t_Fx3uEnet_adp_Binary.h" 


static u8 Fx3uEnet_adp_Binary=0xff;
static u8 Fx3uEnet_adp2_Enable=OFF;

static u8 g_u8Fx3uEnet_adp2Send[FX3UENETADP2_SEND_LENTH]; 
static u8 g_u8Fx3uEnet_adp2_ReceData[FX3UENETADP2_RECE_LENGTH];

void Fx3uEnet_adp2_Init(u8 nodeID)
{
  Fx3uEnet_adp_Binary=nodeID;
  Fx3uEnet_adp2_Enable=ON;
  memset(g_u8Fx3uEnet_adp2Send,0,FX3UENETADP2_SEND_LENTH);
  memset(g_u8Fx3uEnet_adp2_ReceData,0,FX3UENETADP2_RECE_LENGTH);
}

/********************************************************************************************/	 
/********************************************************************************************/
void TIM_1ms_Fx3uEnet_adp2(void)
{
  if(Fx3uEnet_adp2_Enable==ON)
  {
    g_u16_SwitchTimer[Fx3uEnet_adp_Binary]++;
  }
  if((ON==MB_NeedWrite(Fx3uEnet_adp_Binary))&&(READ_IDLE==GW_ReadStatus(Fx3uEnet_adp_Binary)))
	{ 	
    if(GW_WriteStatus(Fx3uEnet_adp_Binary)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[Fx3uEnet_adp_Binary]>1000)
	     {
	     		g_u16_SwitchTimer[Fx3uEnet_adp_Binary]=0;
					GW_WriteStatus(Fx3uEnet_adp_Binary)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(Fx3uEnet_adp_Binary)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[Fx3uEnet_adp_Binary]>WriteTime))
	     {	
		   		GW_WriteStatus(Fx3uEnet_adp_Binary)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[Fx3uEnet_adp_Binary]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(Fx3uEnet_adp_Binary)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(Fx3uEnet_adp_Binary)=WRITE_DELAY;
      g_u16_SwitchTimer[Fx3uEnet_adp_Binary]=0;
	  }
	  else if(g_u16_SwitchTimer[Fx3uEnet_adp_Binary]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[Fx3uEnet_adp_Binary]=0;
      GW_WriteStatus(Fx3uEnet_adp_Binary)=WRITE_IDLE;
	    MB_NeedWrite(Fx3uEnet_adp_Binary)=OFF;
	    g_u8_threadIdx[Fx3uEnet_adp_Binary]++;
		  ThreadNew(Fx3uEnet_adp_Binary)=ON;
	  }
	  else{

	  }
	}
	else
	{
		if((g_u16_SwitchTimer[Fx3uEnet_adp_Binary]>UpdateCycle[Fx3uEnet_adp_Binary])&&
		    (g_u8_ProtocalNum[Fx3uEnet_adp_Binary][READ]!=0))
		{	  
		  g_u16_SwitchTimer[Fx3uEnet_adp_Binary]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(Fx3uEnet_adp_Binary))
		  {
		    GW_ReadStatus(Fx3uEnet_adp_Binary)=READ_WAITRESPOND;
		    g_u16_TimeoutCnt[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]]=0;
		    if(OFF==MB_NeedWrite(Fx3uEnet_adp_Binary))
		    {
          g_u8_threadIdx[Fx3uEnet_adp_Binary]++;
          while(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[Fx3uEnet_adp_Binary]++;
            if(g_u8_ProtocalNum[Fx3uEnet_adp_Binary][READ]<=g_u8_threadIdx[Fx3uEnet_adp_Binary])
      			{			  
      	      g_u8_threadIdx[Fx3uEnet_adp_Binary]=0;
      			}
          }
		      ThreadNew(Fx3uEnet_adp_Binary)=ON;
		    }
		    else
		    {
          GW_ReadStatus(Fx3uEnet_adp_Binary)=READ_IDLE;
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]]++;
        if(g_u16_TimeoutCnt[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]]>
          (THRES_TIMOUTCNT/UpdateCycle[Fx3uEnet_adp_Binary]))
        {
          g_u16_RecvTransLen[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]]=0;
          g_u16_TimeoutCnt[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]]=0;
          if(OFF==MB_NeedWrite(Fx3uEnet_adp_Binary))
  		    {
            g_u8_threadIdx[Fx3uEnet_adp_Binary]++;
            while(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[Fx3uEnet_adp_Binary]++;
              if(g_u8_ProtocalNum[Fx3uEnet_adp_Binary][READ]<=g_u8_threadIdx[Fx3uEnet_adp_Binary])
        			{			  
        	      g_u8_threadIdx[Fx3uEnet_adp_Binary]=0;
        			}
            }
  		      ThreadNew(Fx3uEnet_adp_Binary)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(Fx3uEnet_adp_Binary)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[Fx3uEnet_adp_Binary][READ]<=g_u8_threadIdx[Fx3uEnet_adp_Binary])
	{			  
		g_u8_threadIdx[Fx3uEnet_adp_Binary]=0;
	}
}

void Fx3uEnet_adp2_Write(unsigned char regtype, unsigned int  Adderadd, unsigned char l_u8Len)
{
	u8 sum = 0,len=0;
	u8 i = 0;
	u32 addr;
  memset(g_u8Fx3uEnet_adp2Send,0,FX3UENETADP2_SEND_LENTH);
	g_u8Fx3uEnet_adp2Send[1]  = 0xff; 
	g_u8Fx3uEnet_adp2Send[2]  = 0x0A; 
	g_u8Fx3uEnet_adp2Send[3]  = 0x00; 
	g_u8Fx3uEnet_adp2Send[4]  = Adderadd&0xff; 
	g_u8Fx3uEnet_adp2Send[5]  = (Adderadd>>8)&0xff; 
	g_u8Fx3uEnet_adp2Send[6]  = (Adderadd>>16)&0xff;
	g_u8Fx3uEnet_adp2Send[7]  = (Adderadd>>24)&0xff; 

	if((regtype == 'x')||(regtype == 'X')) //x_R
	{
    g_u8Fx3uEnet_adp2Send[0]  = 0x02;
		g_u8Fx3uEnet_adp2Send[8] =  0x20;
    g_u8Fx3uEnet_adp2Send[9] =  'X';
		len=RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].datalenth/2+
				RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].datalenth%2;
		for(i=0;i<len;i++)
		{
			g_u8Fx3uEnet_adp2Send[12+i] =  ((g_u8_Writedata[2*i+1])<<4)&0xf0;
			g_u8Fx3uEnet_adp2Send[12+i] |=  g_u8_Writedata[2*i+3]&0x0f;
		}
	}
	else if((regtype == 'y')||(regtype == 'Y'))  //y_R
	{
    g_u8Fx3uEnet_adp2Send[0]  = 0x02;
		g_u8Fx3uEnet_adp2Send[8] =  0x20;
    g_u8Fx3uEnet_adp2Send[9] =  'Y'; 
		len=RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].datalenth/2+
				RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].datalenth%2;
		for(i=0;i<len;i++)
		{
			g_u8Fx3uEnet_adp2Send[12+i] =  ((g_u8_Writedata[2*i+1])<<4)&0xf0;
			g_u8Fx3uEnet_adp2Send[12+i] |=  g_u8_Writedata[2*i+3]&0x0f;
		}
	}
	else if((regtype == 'm')||(regtype == 'M'))  //M
	{
    g_u8Fx3uEnet_adp2Send[0]  = 0x02;
		g_u8Fx3uEnet_adp2Send[8] =  0x20;
    g_u8Fx3uEnet_adp2Send[9] =  'M'; 
		len=(l_u8Len/2)+(l_u8Len%2);
		for(i=0;i<len;i++)
		{
			g_u8Fx3uEnet_adp2Send[12+i] =  ((g_u8_Writedata[2*i+1])<<4)&0xf0;
			g_u8Fx3uEnet_adp2Send[12+i] |=  g_u8_Writedata[2*i+3]&0x0f;
		}
	}
	else if((regtype == 's')||(regtype == 'S'))  //S
	{
    g_u8Fx3uEnet_adp2Send[0]  = 0x02;
		g_u8Fx3uEnet_adp2Send[8] =  0x20;
    g_u8Fx3uEnet_adp2Send[9] =  'S'; 
		len=RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].datalenth/2+
				RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].datalenth%2;
		for(i=0;i<len;i++)
		{
			g_u8Fx3uEnet_adp2Send[12+i] =  ((g_u8_Writedata[2*i+1])<<4)&0xf0;
			g_u8Fx3uEnet_adp2Send[12+i] |=  g_u8_Writedata[2*i+3]&0x0f;
		}
	}
	else if((regtype == 'd')||(regtype == 'D'))  //D_R
	{
		len=l_u8Len*2;
		g_u8Fx3uEnet_adp2Send[0]  = 0x03;
    g_u8Fx3uEnet_adp2Send[8] =  0x20;
    g_u8Fx3uEnet_adp2Send[9] =  'D';
		for(i=0;i<l_u8Len;i++)
		{
			g_u8Fx3uEnet_adp2Send[12+2*i+0] =  g_u8_Writedata[2*i+1];
			g_u8Fx3uEnet_adp2Send[12+2*i+1] =  g_u8_Writedata[2*i+0];
		}
	}
  else if((regtype == 't')||(regtype == 'T'))  //T_N
	{
    len=l_u8Len*2;
		g_u8Fx3uEnet_adp2Send[0]  = 0x03;
		g_u8Fx3uEnet_adp2Send[8] =  'N';
    g_u8Fx3uEnet_adp2Send[9] =  'T';
		for(i=0;i<l_u8Len;i++)
		{
			g_u8Fx3uEnet_adp2Send[12+2*i+0] =  g_u8_Writedata[2*i+1];
			g_u8Fx3uEnet_adp2Send[12+2*i+1] =  g_u8_Writedata[2*i+0];
		}
	}
	else if((regtype == 'c')||(regtype == 'C'))  //C_N
	{
		len=l_u8Len*2;
		g_u8Fx3uEnet_adp2Send[0]  = 0x03;
		g_u8Fx3uEnet_adp2Send[8] =  'C'; 
		g_u8Fx3uEnet_adp2Send[9] =  'N'; 
		for(i=0;i<l_u8Len;i++)
		{
			g_u8Fx3uEnet_adp2Send[12+2*i+0] =  g_u8_Writedata[2*i+1];
			g_u8Fx3uEnet_adp2Send[12+2*i+1] =  g_u8_Writedata[2*i+0];
		}
	}
	else
	{

	}
	g_u8Fx3uEnet_adp2Send[10]=l_u8Len;
	g_u8Fx3uEnet_adp2Send[11]=0x00;
  DAQ_EthSend(0,g_u8Fx3uEnet_adp2Send,12+len);
}




void Fx3uEnet_adp2_read(u8 regtype, u32 Adderadd,u8 send_len )
{
	u8 sum = 0,len=0;
	u8 i = 0;
	u32 addr;

  memset(g_u8Fx3uEnet_adp2Send,0,FX3UENETADP2_SEND_LENTH);
	g_u8Fx3uEnet_adp2Send[1]  = 0xff; 
	g_u8Fx3uEnet_adp2Send[2]  = 0x0A; 
	g_u8Fx3uEnet_adp2Send[3]  = 0x00; 
	g_u8Fx3uEnet_adp2Send[4]  = Adderadd&0xff; 
	g_u8Fx3uEnet_adp2Send[5]  = (Adderadd>>8)&0xff; 
	g_u8Fx3uEnet_adp2Send[6]  = (Adderadd>>16)&0xff;
	g_u8Fx3uEnet_adp2Send[7]  = (Adderadd>>24)&0xff; 

	if((regtype == 'x')||(regtype == 'X')) //x_R
	{
    g_u8Fx3uEnet_adp2Send[0]  = 0x00;
		g_u8Fx3uEnet_adp2Send[8] =  0x20;
    g_u8Fx3uEnet_adp2Send[9] =  'X';
	}
	else if((regtype == 'y')||(regtype == 'Y'))  //y_R
	{
    g_u8Fx3uEnet_adp2Send[0]  = 0x00;
		g_u8Fx3uEnet_adp2Send[8] =  0x20;
    g_u8Fx3uEnet_adp2Send[9] =  'Y'; 
	}
	else if((regtype == 'm')||(regtype == 'M'))  //M
	{
    g_u8Fx3uEnet_adp2Send[0]  = 0x00;
		g_u8Fx3uEnet_adp2Send[8] =  0x20;
    g_u8Fx3uEnet_adp2Send[9] =  'M'; 
	}
	else if((regtype == 'm')||(regtype == 'S'))  //S
	{
    g_u8Fx3uEnet_adp2Send[0]  = 0x00;
		g_u8Fx3uEnet_adp2Send[8] =  0x20;
    g_u8Fx3uEnet_adp2Send[9] =  'S'; 
	}
	else if((regtype == 'd')||(regtype == 'D'))  //D_R
	{
		g_u8Fx3uEnet_adp2Send[0]  = 0x01;
    g_u8Fx3uEnet_adp2Send[8] =  0x20;
    g_u8Fx3uEnet_adp2Send[9] =  'D';
	  
	}
  else if((regtype == 't')||(regtype == 'T'))  //T_N
	{
    g_u8Fx3uEnet_adp2Send[0]  = 0x01;
		g_u8Fx3uEnet_adp2Send[8] =  'N';
    g_u8Fx3uEnet_adp2Send[9] =  'T';
	}
	else if((regtype == 'c')||(regtype == 'C'))  //C_N
	{
		g_u8Fx3uEnet_adp2Send[0]  = 0x01;
		g_u8Fx3uEnet_adp2Send[8] =  'C'; 
		g_u8Fx3uEnet_adp2Send[9] =  'N'; 
	}
	else
	{

	}
	g_u8Fx3uEnet_adp2Send[10]  = send_len;
	g_u8Fx3uEnet_adp2Send[11]  = 0x00;
	len =12;
	DAQ_EthSend(0,g_u8Fx3uEnet_adp2Send,len);
}


void Fx3uEnet_adp2_RecDataHandle(u8 regtype,u8 *sourcebuf,u8 *targetbuf,u8 datalen)
{
  u16 i;
  if((RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regtype=='X')||
			(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regtype=='Y')||
			(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regtype=='M')||
			(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regtype=='S'))
	{
    for(i=0;i<datalen ;i++)
    {
      targetbuf[4*i+0]=0x00;
			targetbuf[4*i+2]=0x00;
			targetbuf[4*i+1]=(sourcebuf[i]>>4)&0x0f;
			targetbuf[4*i+3]=sourcebuf[i]&0x0f;
    }
  }
  else if((RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regtype=='D')||
			(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regtype=='T')||
			(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regtype=='C'))
  {
    for(i=0;i<datalen;i++)
    {
				targetbuf[2*i+0]=sourcebuf[2*i+1];
				targetbuf[2*i+1]=sourcebuf[2*i+0];
    }
  }
  else
  {
    
  }
}




void f_Fx3uEnet_adp2_task(void)
{
	u16 size=0, i = 0;
	u32 dataaddr;
	
	mb_tcp_master_W5500_Socket_Set(0);//检查连接
  mb_tcp_master_W5500_Interrupt_Process(0);//W5500中断处理程序框架

	
	memset(g_u8Fx3uEnet_adp2_ReceData,0,FX3UENETADP2_RECE_LENGTH);
  size=MasterRead_SOCK_Data_Buffer(0,g_u8Fx3uEnet_adp2_ReceData);

	if(size)
	{		
		if(g_u8Fx3uEnet_adp2_ReceData[0]!=0x80+g_u8Fx3uEnet_adp2Send[0])
		{
			return;
		}
		if((GW_WriteStatus(Fx3uEnet_adp_Binary)==WRITE_WAITRESPOND)&&(g_u8Fx3uEnet_adp2_ReceData[1]==0x00))
		{
			GW_WriteStatus(Fx3uEnet_adp_Binary)=WRITE_RECVSUCCESS;	
			memcpy(&g_u8_EthRespData[Fx3uEnet_adp_Binary][(g_u16_StartAddr[Fx3uEnet_adp_Binary][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
		}
	  else if(GW_ReadStatus(Fx3uEnet_adp_Binary)==READ_WAITRESPOND)
		{
			if(g_u8Fx3uEnet_adp2_ReceData[1]==0X00)
			{
				GW_ReadStatus(Fx3uEnet_adp_Binary)=READ_RECVSUCCESS;
				if((RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regtype=='X')||
						(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regtype=='Y')||
						(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regtype=='M')||
						(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regtype=='S'))
				{
					size=RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].datalenth/2+
							RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].datalenth%2;
				}
				else{
					size=RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].datalenth*2;
				}
        Fx3uEnet_adp2_RecDataHandle(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regtype,
            &g_u8Fx3uEnet_adp2_ReceData[2],
            &g_u8_EthRespData[Fx3uEnet_adp_Binary][g_u16_StartAddr[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]]*2],
            size);
				g_u16_RecvTransLen[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]] = RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].datalenth * 2 ; 	
			}
		}
		else { //0x16 == g_u8ArrReceData_plc[8]
		
			 //GW_SendStatus(Fx3uEnet_adp_Binary)=SENDRESPONDSE;
			 NegativeResponse(Err_MBcmd);
 			 memset(g_u8Fx3uEnet_adp2_ReceData,0,FX3UENETADP2_RECE_LENGTH);
		}		  
	}

	if(ThreadNew(Fx3uEnet_adp_Binary) == ON)
	{
		ThreadNew(Fx3uEnet_adp_Binary) = OFF;
		GW_ReadStatus(Fx3uEnet_adp_Binary)=READ_WAITRESPOND;
    dataaddr=(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regaddr[5]<<16)
			+(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regaddr[6]<<8)
      + RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regaddr[7] ;
    Fx3uEnet_adp2_read(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].regtype,dataaddr,
      RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_threadIdx[Fx3uEnet_adp_Binary]].datalenth);
	}	
  else if(WRITE_PRESEND==GW_WriteStatus(Fx3uEnet_adp_Binary))
  {
    GW_WriteStatus(Fx3uEnet_adp_Binary)=WRITE_WAITRESPOND;
    dataaddr=(g_u8_WriteAddr[5]<<16) + (g_u8_WriteAddr[6]<<8) + g_u8_WriteAddr[7];
    Fx3uEnet_adp2_Write(RegisterCfgBuff[Fx3uEnet_adp_Binary][g_u8_RespondID].regtype,
      dataaddr,g_u16_WriteLen);	
  }
  else
  {
        ;
  }
}
