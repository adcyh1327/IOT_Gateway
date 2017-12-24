#include "main.h"
#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"	 
#include "m_spi_mb_tcp_master.h"	  
#include "m_spi_mb_tcp_master_cfg.h"
#include "tcp_master_mb.h"
#include "m_w5500.h"
#include "t_MitsubishiQ_tcp.h"
//#include "s7200_tcp.h" 

#define MITSUBISHIQTCP_SEND_LENGTH             450         //发送缓冲区大小
#define MITSUBISHIQTCP_RECE_LENGTH             450         //接受缓冲区大小

static u8 MitsubishiQ_tcp=0xff;
static u8 MitsubishiQ_tcp_Enable=OFF;;

static u8 g_u8MitsubishiQtcpSend[MITSUBISHIQTCP_SEND_LENGTH]; 
static u8 g_u8MitsubishiQReceData[MITSUBISHIQTCP_RECE_LENGTH];

void MitsubishiQtcp_Init(u8 nodeID)
{
  MitsubishiQ_tcp=nodeID;
  MitsubishiQ_tcp_Enable=ON;
  memset(g_u8MitsubishiQtcpSend,'0',MITSUBISHIQTCP_SEND_LENGTH);
  memset(g_u8MitsubishiQReceData,'0',MITSUBISHIQTCP_RECE_LENGTH);
}

/********************************************************************************************/	 
/********************************************************************************************/
void TIM_1ms_MitsubishiQtcp(void)
{
  if(MitsubishiQ_tcp_Enable==ON)
  {
    g_u16_SwitchTimer[MitsubishiQ_tcp]++;
  }
  if((ON==MB_NeedWrite(MitsubishiQ_tcp))&&(READ_IDLE==GW_ReadStatus(MitsubishiQ_tcp)))
	{ 	
    if(GW_WriteStatus(MitsubishiQ_tcp)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[MitsubishiQ_tcp]>1000)
	     {
	     		g_u16_SwitchTimer[MitsubishiQ_tcp]=0;
					GW_WriteStatus(MitsubishiQ_tcp)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(MitsubishiQ_tcp)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[MitsubishiQ_tcp]>WriteTime))
	     {	
		   		GW_WriteStatus(MitsubishiQ_tcp)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[MitsubishiQ_tcp]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(MitsubishiQ_tcp)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(MitsubishiQ_tcp)=WRITE_DELAY;
      g_u16_SwitchTimer[MitsubishiQ_tcp]=0;
	  }
	  else if(g_u16_SwitchTimer[MitsubishiQ_tcp]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[MitsubishiQ_tcp]=0;
      GW_WriteStatus(MitsubishiQ_tcp)=WRITE_IDLE;
	    MB_NeedWrite(MitsubishiQ_tcp)=OFF;
	    g_u8_threadIdx[MitsubishiQ_tcp]++;
		  ThreadNew(MitsubishiQ_tcp)=ON;
	  }
	  else{

	  }
	}
	else
	{
		if((g_u16_SwitchTimer[MitsubishiQ_tcp]>UpdateCycle[MitsubishiQ_tcp])&&
		    (g_u8_ProtocalNum[MitsubishiQ_tcp][READ]!=0))
		{	  
		  g_u16_SwitchTimer[MitsubishiQ_tcp]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(MitsubishiQ_tcp))
		  {
		    GW_ReadStatus(MitsubishiQ_tcp)=READ_WAITRESPOND;
		    g_u16_TimeoutCnt[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]]=0;
		    if(OFF==MB_NeedWrite(MitsubishiQ_tcp))
		    {
          g_u8_threadIdx[MitsubishiQ_tcp]++;
          while(RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[MitsubishiQ_tcp]++;
            if(g_u8_ProtocalNum[MitsubishiQ_tcp][READ]<=g_u8_threadIdx[MitsubishiQ_tcp])
      			{			  
      	      g_u8_threadIdx[MitsubishiQ_tcp]=0;
      			}
          }
		      ThreadNew(MitsubishiQ_tcp)=ON;
		    }
		    else
		    {
          GW_ReadStatus(MitsubishiQ_tcp)=READ_IDLE;
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]]++;
        if(g_u16_TimeoutCnt[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]]>
          (THRES_TIMOUTCNT/UpdateCycle[MitsubishiQ_tcp]))
        {
          g_u16_RecvTransLen[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]]=0;
          g_u16_TimeoutCnt[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]]=0;
          if(OFF==MB_NeedWrite(MitsubishiQ_tcp))
  		    {
            g_u8_threadIdx[MitsubishiQ_tcp]++;
            while(RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[MitsubishiQ_tcp]++;
              if(g_u8_ProtocalNum[MitsubishiQ_tcp][READ]<=g_u8_threadIdx[MitsubishiQ_tcp])
        			{			  
        	      g_u8_threadIdx[MitsubishiQ_tcp]=0;
        			}
            }
  		      ThreadNew(MitsubishiQ_tcp)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(MitsubishiQ_tcp)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[MitsubishiQ_tcp][READ]<=g_u8_threadIdx[MitsubishiQ_tcp])
	{			  
		g_u8_threadIdx[MitsubishiQ_tcp]=0;
	}
}

void MitsubishiQ_Write(unsigned char LeiXing, unsigned int  Adderadd, unsigned char l_u8Len)
{
	 u8 sum = 0,len=0;
	 u8 i = 0;
	 u32 addr;
   memset(g_u8MitsubishiQtcpSend,'0',MITSUBISHIQTCP_SEND_LENGTH);
	
	g_u8MitsubishiQtcpSend[0]  = '5'; 
	g_u8MitsubishiQtcpSend[1]  = '0'; 
	g_u8MitsubishiQtcpSend[2]  = '0'; 
	g_u8MitsubishiQtcpSend[3]  = '0'; 
	g_u8MitsubishiQtcpSend[4]  = '0'; 
	g_u8MitsubishiQtcpSend[5]  = '0'; 
	g_u8MitsubishiQtcpSend[6]  = 'F'; 
	g_u8MitsubishiQtcpSend[7]  = 'F'; 
  g_u8MitsubishiQtcpSend[8]  = '0'; 
	g_u8MitsubishiQtcpSend[9]  = '3'; 
	g_u8MitsubishiQtcpSend[10] = 'F'; 
	g_u8MitsubishiQtcpSend[11] = 'F'; 
	g_u8MitsubishiQtcpSend[12] = '0'; 
	g_u8MitsubishiQtcpSend[13] = '0'; 

  g_u8MitsubishiQtcpSend[18] = '0';  
	g_u8MitsubishiQtcpSend[19] = '0'; 
	g_u8MitsubishiQtcpSend[20] = '1'; 
	g_u8MitsubishiQtcpSend[21] = '0'; 
	g_u8MitsubishiQtcpSend[22] = '1'; 	
	g_u8MitsubishiQtcpSend[23] = '4'; 
	g_u8MitsubishiQtcpSend[24] = '0'; 
	g_u8MitsubishiQtcpSend[25] = '1';
	g_u8MitsubishiQtcpSend[26] = '0'; 
	g_u8MitsubishiQtcpSend[27] = '0';
	
	g_u8MitsubishiQtcpSend[38] = HEXtoASCII((l_u8Len>>12)&0x0F); 
	g_u8MitsubishiQtcpSend[39] = HEXtoASCII((l_u8Len>>8)&0x0F);  
	g_u8MitsubishiQtcpSend[40] = HEXtoASCII((l_u8Len>>4)&0x0F);
	g_u8MitsubishiQtcpSend[41] = HEXtoASCII(l_u8Len&0x0F);	

	if((LeiXing == 'y')||(LeiXing == 'Y')) //y_W
	{
		len = 0x18+l_u8Len;
		g_u8MitsubishiQtcpSend[14] =  HEXtoASCII((len>>12)&0x0F); 
		g_u8MitsubishiQtcpSend[15] =  HEXtoASCII((len>>8)&0x0F); 
		g_u8MitsubishiQtcpSend[16]  = HEXtoASCII((len>>4)&0x0F);	
		g_u8MitsubishiQtcpSend[17]  = HEXtoASCII(len&0x0F);	
    g_u8MitsubishiQtcpSend[28] =  '0';
		g_u8MitsubishiQtcpSend[29] =  '1';
		g_u8MitsubishiQtcpSend[30] =  'Y'; 
		g_u8MitsubishiQtcpSend[31] =  '*'; 		
		
 		for(i=0;i<l_u8Len;i++)
    {
      g_u8MitsubishiQtcpSend[42+i]=HEXtoASCII((g_u8_Writedata[i*2+1])&0x01);
    }
	}
	else if((LeiXing == 'm')||(LeiXing == 'M'))  //M_W
	{
	 	len = 0x18+l_u8Len;
		g_u8MitsubishiQtcpSend[14] =  HEXtoASCII((len>>12)&0x0F); 
		g_u8MitsubishiQtcpSend[15] =  HEXtoASCII((len>>8)&0x0F); 
		g_u8MitsubishiQtcpSend[16]  = HEXtoASCII((len>>4)&0x0F);	
		g_u8MitsubishiQtcpSend[17]  = HEXtoASCII(len&0x0F);	
		
    g_u8MitsubishiQtcpSend[28] =  '0';
		g_u8MitsubishiQtcpSend[29] =  '1';
		g_u8MitsubishiQtcpSend[30] =  'M'; 
		g_u8MitsubishiQtcpSend[31] =  '*'; 		
		
    for(i=0;i<l_u8Len;i++)
    {
      g_u8MitsubishiQtcpSend[42+i]=HEXtoASCII((g_u8_Writedata[i*2+1])&0x01);
    }

	}
	else if((LeiXing == 'd')||(LeiXing == 'D'))  //d_W
	{
		len = 0x18+l_u8Len*4;
		g_u8MitsubishiQtcpSend[14] =  HEXtoASCII((len>>12)&0x0F); 
		g_u8MitsubishiQtcpSend[15] =  HEXtoASCII((len>>8)&0x0F); 
		g_u8MitsubishiQtcpSend[16]  = HEXtoASCII((len>>4)&0x0F);	
		g_u8MitsubishiQtcpSend[17]  = HEXtoASCII(len&0x0F);	

    g_u8MitsubishiQtcpSend[28] =  '0';
		g_u8MitsubishiQtcpSend[29] =  '0';
		g_u8MitsubishiQtcpSend[30] =  'D'; 
		g_u8MitsubishiQtcpSend[31] =  '*'; 		
		
    for(i=0;i<l_u8Len;i++)
    {
      g_u8MitsubishiQtcpSend[42+4*i]=  HEXtoASCII((g_u8_Writedata[i*2+0]>>4)&0x0F);
      g_u8MitsubishiQtcpSend[42+4*i+1]=HEXtoASCII((g_u8_Writedata[i*2+0])&0x0F);
      g_u8MitsubishiQtcpSend[42+4*i+2]=HEXtoASCII((g_u8_Writedata[i*2+1]>>4)&0x0F);
      g_u8MitsubishiQtcpSend[42+4*i+3]=HEXtoASCII((g_u8_Writedata[i*2+1])&0x0F);
    }
	}
	else if((LeiXing == 't')||(LeiXing == 'T'))  //t_W
	{
		len = 0x18+l_u8Len;
		g_u8MitsubishiQtcpSend[14] =  HEXtoASCII((len>>12)&0x0F); 
		g_u8MitsubishiQtcpSend[15] =  HEXtoASCII((len>>8)&0x0F); 
		g_u8MitsubishiQtcpSend[16]  = HEXtoASCII((len>>4)&0x0F);	
		g_u8MitsubishiQtcpSend[17]  = HEXtoASCII(len&0x0F);	
		
    g_u8MitsubishiQtcpSend[28] =  '0';
		g_u8MitsubishiQtcpSend[29] =  '0';
		g_u8MitsubishiQtcpSend[30] =  'T'; 
		g_u8MitsubishiQtcpSend[31] =  'N'; 		
		
		for(i=0;i<l_u8Len;i++)
    {
			g_u8MitsubishiQtcpSend[42+4*i]=  HEXtoASCII((g_u8_Writedata[i*2+0]>>4)&0x0F);
      g_u8MitsubishiQtcpSend[42+4*i+1]=HEXtoASCII((g_u8_Writedata[i*2+0])&0x0F);
      g_u8MitsubishiQtcpSend[42+4*i+2]=HEXtoASCII((g_u8_Writedata[i*2+1]>>4)&0x0F);
      g_u8MitsubishiQtcpSend[42+4*i+3]=HEXtoASCII((g_u8_Writedata[i*2+1])&0x0F);
    }
	}
	else if((LeiXing == 'c')||(LeiXing == 'C'))  //C_W
	{
		len = 0x18+l_u8Len;
		g_u8MitsubishiQtcpSend[14] =  HEXtoASCII((len>>12)&0x0F); 
		g_u8MitsubishiQtcpSend[15] =  HEXtoASCII((len>>8)&0x0F); 
		g_u8MitsubishiQtcpSend[16]  = HEXtoASCII((len>>4)&0x0F);	
		g_u8MitsubishiQtcpSend[17]  = HEXtoASCII(len&0x0F);	

    g_u8MitsubishiQtcpSend[28] =  '0';
		g_u8MitsubishiQtcpSend[29] =  '0';
		g_u8MitsubishiQtcpSend[30] =  'C'; 
		g_u8MitsubishiQtcpSend[31] =  'N'; 		
		
    for(i=0;i<l_u8Len;i++)
    {
			g_u8MitsubishiQtcpSend[42+4*i]=  HEXtoASCII((g_u8_Writedata[i*2+0]>>4)&0x0F);
      g_u8MitsubishiQtcpSend[42+4*i+1]=HEXtoASCII((g_u8_Writedata[i*2+0])&0x0F);
      g_u8MitsubishiQtcpSend[42+4*i+2]=HEXtoASCII((g_u8_Writedata[i*2+1]>>4)&0x0F);
      g_u8MitsubishiQtcpSend[42+4*i+3]=HEXtoASCII((g_u8_Writedata[i*2+1])&0x0F);
    }
	}
	
	else
	{

	}  
	g_u8MitsubishiQtcpSend[32] =  HEXtoASCII(((Adderadd%1000000)/100000)&0x0F );	
	g_u8MitsubishiQtcpSend[33] =  HEXtoASCII(((Adderadd%100000)/10000)&0x0F );	
	g_u8MitsubishiQtcpSend[34] =  HEXtoASCII(((Adderadd%10000)/1000)&0x0F );	
	g_u8MitsubishiQtcpSend[35] =  HEXtoASCII(((Adderadd%1000)/100)&0x0F );	
	g_u8MitsubishiQtcpSend[36] =  HEXtoASCII(((Adderadd%100)/10)&0x0F );	
	g_u8MitsubishiQtcpSend[37] =  HEXtoASCII(Adderadd%10);
  DAQ_EthSend(0,g_u8MitsubishiQtcpSend,len+18);
}




void MitsubishiQ_read(u8 LeiXing, u32 Adderadd,u8 send_len )
{
	u8 sum = 0,len=0;
	u8 i = 0;
	u32 addr;

  memset(g_u8MitsubishiQtcpSend,'0',MITSUBISHIQTCP_SEND_LENGTH);	
  g_u8MitsubishiQtcpSend[0]  = '5'; 
	g_u8MitsubishiQtcpSend[1]  = '0'; 
	g_u8MitsubishiQtcpSend[2]  = '0'; 
	g_u8MitsubishiQtcpSend[3]  = '0'; 
	g_u8MitsubishiQtcpSend[4]  = '0'; 
	g_u8MitsubishiQtcpSend[5]  = '0'; 
	g_u8MitsubishiQtcpSend[6]  = 'F'; 
	g_u8MitsubishiQtcpSend[7]  = 'F'; 
	g_u8MitsubishiQtcpSend[8]  = '0';	
  g_u8MitsubishiQtcpSend[9]  = '3'; 
	g_u8MitsubishiQtcpSend[10] = 'F'; 
	g_u8MitsubishiQtcpSend[11] = 'F'; 	
	g_u8MitsubishiQtcpSend[12] = '0'; 
	g_u8MitsubishiQtcpSend[13] = '0'; 
	g_u8MitsubishiQtcpSend[14] = '0';
	g_u8MitsubishiQtcpSend[15] = '0'; 
	g_u8MitsubishiQtcpSend[16] = '1'; 
	g_u8MitsubishiQtcpSend[17] = '8'; 
	g_u8MitsubishiQtcpSend[18] = '0'; 
	g_u8MitsubishiQtcpSend[19] = '0'; 
	g_u8MitsubishiQtcpSend[20] = '1'; 
	g_u8MitsubishiQtcpSend[21] = '0'; 
	g_u8MitsubishiQtcpSend[22] = '0'; 
	g_u8MitsubishiQtcpSend[23] = '4';	
  g_u8MitsubishiQtcpSend[24] = '0'; 
	g_u8MitsubishiQtcpSend[25] = '1'; 
	g_u8MitsubishiQtcpSend[26] = '0'; 	
	g_u8MitsubishiQtcpSend[27] = '0'; 

	if((LeiXing == 'x')||(LeiXing == 'X')) //x_R
	{
    g_u8MitsubishiQtcpSend[28] =  '0';
    g_u8MitsubishiQtcpSend[29] =  '1';
		g_u8MitsubishiQtcpSend[30] =  'X'; 
		g_u8MitsubishiQtcpSend[31] =  '*'; 
	}
	else if((LeiXing == 'y')||(LeiXing == 'Y'))  //y_R
	{
    g_u8MitsubishiQtcpSend[28] =  '0';
    g_u8MitsubishiQtcpSend[29] =  '1';
		g_u8MitsubishiQtcpSend[30] =  'Y'; 
		g_u8MitsubishiQtcpSend[31] =  '*'; 
	}
	else if((LeiXing == 'm')||(LeiXing == 'M'))  //M_R
	{
    g_u8MitsubishiQtcpSend[28] =  '0';
    g_u8MitsubishiQtcpSend[29] =  '1';
		g_u8MitsubishiQtcpSend[30] =  'M'; 
		g_u8MitsubishiQtcpSend[31] =  '*'; 
	}
	else if((LeiXing == 'd')||(LeiXing == 'D'))  //D_R
	{
	
    g_u8MitsubishiQtcpSend[28] =  '0';
    g_u8MitsubishiQtcpSend[29] =  '0';
		g_u8MitsubishiQtcpSend[30] =  'D'; 
		g_u8MitsubishiQtcpSend[31] =  '*'; 
	  
	}
  else if((LeiXing == 't')||(LeiXing == 'T'))  //T_R
	{
    g_u8MitsubishiQtcpSend[28] =  '0';
    g_u8MitsubishiQtcpSend[29] =  '0';
		g_u8MitsubishiQtcpSend[30] =  'T'; 
		g_u8MitsubishiQtcpSend[31] =  'N'; 
	}
	else if((LeiXing == 'c')||(LeiXing == 'C'))  //C_R
	{
	  g_u8MitsubishiQtcpSend[28] =  '0';
    g_u8MitsubishiQtcpSend[29] =  '0';
		g_u8MitsubishiQtcpSend[30] =  'C'; 
		g_u8MitsubishiQtcpSend[31] =  'N'; 
	}
	else
	{

	}
	g_u8MitsubishiQtcpSend[32] =  HEXtoASCII(((Adderadd%1000000)/100000)&0x0F );	
	g_u8MitsubishiQtcpSend[33] =  HEXtoASCII(((Adderadd%100000)/10000)&0x0F );	
	g_u8MitsubishiQtcpSend[34] =  HEXtoASCII(((Adderadd%10000)/1000)&0x0F );	
	g_u8MitsubishiQtcpSend[35] =  HEXtoASCII(((Adderadd%1000)/100)&0x0F );	
	g_u8MitsubishiQtcpSend[36] =  HEXtoASCII(((Adderadd%100)/10)&0x0F );	
	g_u8MitsubishiQtcpSend[37] =  HEXtoASCII(Adderadd%10);
	g_u8MitsubishiQtcpSend[38] =  HEXtoASCII((send_len>>16)&0x0F);
	g_u8MitsubishiQtcpSend[39] =  HEXtoASCII((send_len>>8)&0x0F);
  g_u8MitsubishiQtcpSend[40] =  HEXtoASCII((send_len>>4)&0x0F);
  g_u8MitsubishiQtcpSend[41] =  HEXtoASCII(send_len&0x0F);
	len =42;
	DAQ_EthSend(0,g_u8MitsubishiQtcpSend,len);
}


void MitsubishiQtcp_RecDataHandle(u8 regtype,u8 *sourcebuf,u8 *targetbuf,u8 datalen)
{
  u16 i;
  if((RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regtype=='X')||
			(RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regtype=='Y')||
			(RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regtype=='M'))
	{
    for(i=0;i<RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].datalenth ;i++)
    {
      targetbuf[2*i+0]=0x00;
      targetbuf[2*i+1]=ASCIItoHEX(sourcebuf[i]);
    }
  }
  else if((RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regtype=='D')||
			(RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regtype=='T')||
			(RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regtype=='C'))
  {
    for(i=0;i<datalen / 2;i++)
    {
      targetbuf[i]=ASCII2BYTE(sourcebuf[2*i+0],sourcebuf[2*i+1]);
    }
  }
  else
  {
    
  }
}




void f_MitsubishiQtcp_task(void)
{
	u16 size=0, i = 0;
	u32 dataaddr;
	
	
	mb_tcp_master_W5500_Socket_Set(0);//检查连接
  mb_tcp_master_W5500_Interrupt_Process(0);//W5500中断处理程序框架

	memset(g_u8MitsubishiQReceData,'0',MITSUBISHIQTCP_RECE_LENGTH);
  size=MasterRead_SOCK_Data_Buffer(0,g_u8MitsubishiQReceData);

	if(size)
	{		
		size = (ASCII2BYTE(g_u8MitsubishiQReceData[14],g_u8MitsubishiQReceData[15])*256) + 
					 ASCII2BYTE(g_u8MitsubishiQReceData[16],g_u8MitsubishiQReceData[17]);
		if(GW_WriteStatus(MitsubishiQ_tcp)==WRITE_WAITRESPOND)
		{
					GW_WriteStatus(MitsubishiQ_tcp)=WRITE_RECVSUCCESS;	
          memcpy(&g_u8_EthRespData[MitsubishiQ_tcp][(g_u16_StartAddr[MitsubishiQ_tcp][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
		}
	  else if(GW_ReadStatus(MitsubishiQ_tcp)==READ_WAITRESPOND)
		{
			if((size == RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].datalenth+4)||
	  	 (size == RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].datalenth*4+4))
			{
				GW_ReadStatus(MitsubishiQ_tcp)=READ_RECVSUCCESS;
				if((RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regtype=='X')||
						(RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regtype=='Y')||
						(RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regtype=='M'))
				{
					size=RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].datalenth;
				}
				else{
					size=RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].datalenth*4;
				}
        MitsubishiQtcp_RecDataHandle(RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regtype,
          &g_u8MitsubishiQReceData[22],
			    &g_u8_EthRespData[MitsubishiQ_tcp][g_u16_StartAddr[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]]*2],size);
				  g_u16_RecvTransLen[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]] = RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].datalenth * 2 ; 	
			}
		}
		else { //0x16 == g_u8ArrReceData_plc[8]
		
			 //GW_SendStatus(MitsubishiQ_tcp)=SENDRESPONDSE;
			 NegativeResponse(Err_MBcmd);
 			 memset(g_u8MitsubishiQReceData,'0',MITSUBISHIQTCP_RECE_LENGTH);
		}		  
	}

	if(ThreadNew(MitsubishiQ_tcp) == ON)
	{
		ThreadNew(MitsubishiQ_tcp) = OFF;
		GW_ReadStatus(MitsubishiQ_tcp)=READ_WAITRESPOND;
    dataaddr=(RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regaddr[5]<<16)
			+(RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regaddr[6]<<8)
      + RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regaddr[7] ;
    MitsubishiQ_read(RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].regtype,dataaddr,
      RegisterCfgBuff[MitsubishiQ_tcp][g_u8_threadIdx[MitsubishiQ_tcp]].datalenth);
	}	
  else if(WRITE_PRESEND==GW_WriteStatus(MitsubishiQ_tcp))
  {
    GW_WriteStatus(MitsubishiQ_tcp)=WRITE_WAITRESPOND;
    dataaddr=(g_u8_WriteAddr[5]<<16) + (g_u8_WriteAddr[6]<<8) + g_u8_WriteAddr[7];
    MitsubishiQ_Write(RegisterCfgBuff[MitsubishiQ_tcp][g_u8_RespondID].regtype,
      dataaddr,g_u16_WriteLen);	
  }
  else
  {
        ;
  }
}
