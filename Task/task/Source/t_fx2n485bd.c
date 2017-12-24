//#ifdef FX2N485BD_H

#include "main.h"

#define FX2N485BD_SEND_LENGTH            220
#define FX2N485BD_RECE_LENGTH            220

static u8 Fx2n485bd_SendBuffer[FX2N485BD_SEND_LENGTH];
static u8 Fx2n485bdReceData[FX2N485BD_RECE_LENGTH];          //板卡接受设备返回数据的缓冲区
static u8 headRecvd;
static u8 frameLen;
static u8 frameLenWait;
static u8 writeResp;	
static u8 Fx2n485bd=0xff;
static u8 Fx2n485bd_Enable=OFF;



void Fx2n485bd_Init(u8 nodeID)
{
  u8 i;
  headRecvd=0;
  frameLen=0;
  writeResp=0;
  for(i=0;i<FX2N485BD_SEND_LENGTH;i++)
  {
    Fx2n485bd_SendBuffer[i]=0;
  }
  for(i=0;i<FX2N485BD_RECE_LENGTH;i++)
  {
    Fx2n485bdReceData[i]=0;
  }
  Fx2n485bd=nodeID;
  Fx2n485bd_Enable=ON;
}

void Fx2n485bd_Send(u8 *sendbuf, u8 len)
{
  /*u8 i;
  for(i = 0;i < len;i ++)
  {  
   	if(0!=(Count_bit1(sendbuf[i]) % 2)) 
  	{
  	  sendbuf[i] =0x80|(sendbuf[i]);
  	}
  	else
  	{	
  	  sendbuf[i] = (sendbuf[i]);
  	} 						  	
  }*/
  DAQ_UartSend(sendbuf,len,CHN_UART_CFG);
}

void TIM_1ms_Fx2n485bd(void)
{
  if(Fx2n485bd_Enable==ON)
  {
  	g_u16_SwitchTimer[Fx2n485bd]++; 
  }
  if((ON==MB_NeedWrite(Fx2n485bd))&&(READ_IDLE==GW_ReadStatus(Fx2n485bd)))
	{
    g_u16_SwitchTimer[Fx2n485bd]++; 	
    if(GW_WriteStatus(Fx2n485bd)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[Fx2n485bd]>1000)
	     {
	     		g_u16_SwitchTimer[Fx2n485bd]=0;
					GW_WriteStatus(Fx2n485bd)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(Fx2n485bd)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[Fx2n485bd]>WriteTime))
	     {	
		   		GW_WriteStatus(Fx2n485bd)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[Fx2n485bd]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(Fx2n485bd)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(Fx2n485bd)=WRITE_DELAY;
      g_u16_SwitchTimer[Fx2n485bd]=0;
	  }
	  else if(g_u16_SwitchTimer[Fx2n485bd]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[Fx2n485bd]=0;
      GW_WriteStatus(Fx2n485bd)=WRITE_IDLE;
	    MB_NeedWrite(Fx2n485bd)=OFF;
	    g_u8_threadIdx[Fx2n485bd]++;
		  ThreadNew(Fx2n485bd)=ON;
	  }
	  else{

	  }
	}
	else
	{	
		if((g_u16_SwitchTimer[Fx2n485bd]>UpdateCycle[Fx2n485bd])&&
		    (g_u8_ProtocalNum[Fx2n485bd][READ]!=0))
		{	  
		  g_u16_SwitchTimer[Fx2n485bd]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(Fx2n485bd))
		  {
		    GW_ReadStatus(Fx2n485bd)=READ_IDLE;
		    g_u16_TimeoutCnt[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]]=0;
		    if(OFF==MB_NeedWrite(Fx2n485bd))
		    {
          g_u8_threadIdx[Fx2n485bd]++;
          while(RegisterCfgBuff[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[Fx2n485bd]++;
            if(g_u8_ProtocalNum[Fx2n485bd][READ]<=g_u8_threadIdx[Fx2n485bd])
      			{			  
      	      g_u8_threadIdx[Fx2n485bd]=0;
      			}
          }
		      ThreadNew(Fx2n485bd)=ON;
		    }
		    else
		    {
          
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]]++;
        if(g_u16_TimeoutCnt[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]]>
          (THRES_TIMOUTCNT/UpdateCycle[Fx2n485bd]))
        {
          g_u16_RecvTransLen[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]]=0;
          g_u16_TimeoutCnt[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]]=0;
          if(OFF==MB_NeedWrite(Fx2n485bd))
  		    {
            g_u8_threadIdx[Fx2n485bd]++;
            while(RegisterCfgBuff[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[Fx2n485bd]++;
              if(g_u8_ProtocalNum[Fx2n485bd][READ]<=g_u8_threadIdx[Fx2n485bd])
        			{			  
        	      g_u8_threadIdx[Fx2n485bd]=0;
        			}
            }
  		      ThreadNew(Fx2n485bd)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(Fx2n485bd)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[Fx2n485bd][READ]<=g_u8_threadIdx[Fx2n485bd])
	{			  
		g_u8_threadIdx[Fx2n485bd]=0;
	}
}


//fx2n485bd配置成了 9600,7,1,偶校验
void fx2n485bd_read(u8 regtype,u16 addr,u16 lenth)
{
  u8 i;
  u8 sum;
  Fx2n485bd_SendBuffer[0]=5;	 
  Fx2n485bd_SendBuffer[1]=DEC2ASCII(
    RegisterCfgBuff[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]].plcaddrstation/16);
  Fx2n485bd_SendBuffer[2]=DEC2ASCII(
    RegisterCfgBuff[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]].plcaddrstation%16);
  Fx2n485bd_SendBuffer[3]='F';
  Fx2n485bd_SendBuffer[4]='F';	
	if((regtype=='X')||(regtype=='Y')||(regtype=='M'))
	{
		Fx2n485bd_SendBuffer[5]='B';	
	}
	else if((regtype=='D')||(regtype=='T')||(regtype=='C'))
	{
		Fx2n485bd_SendBuffer[5]='W';	
	}
	else
	{
		
	}
  Fx2n485bd_SendBuffer[6]='R'; 
  Fx2n485bd_SendBuffer[7]='A'; 
  Fx2n485bd_SendBuffer[8]=regtype;

  //地址
  if((regtype=='C')||(regtype=='T'))
  {
    Fx2n485bd_SendBuffer[ 9]='N';
  }
  else
  {
    Fx2n485bd_SendBuffer[ 9]=DEC2ASCII( addr / 1000);
  }
  Fx2n485bd_SendBuffer[10]=DEC2ASCII((addr % 1000)/100);
  Fx2n485bd_SendBuffer[11]=DEC2ASCII((addr % 100)/10);
  Fx2n485bd_SendBuffer[12]=DEC2ASCII( addr % 10);
  //长度
  Fx2n485bd_SendBuffer[13]=DEC2ASCII( lenth  /16);
  Fx2n485bd_SendBuffer[14]=DEC2ASCII( lenth  %16);
  //校验
  sum=0;
  for(i=1;i<15;i++)
  {
    sum+=Fx2n485bd_SendBuffer[i];
  }
  Fx2n485bd_SendBuffer[15]=DEC2ASCII(sum  /16);  
  Fx2n485bd_SendBuffer[16]=DEC2ASCII(sum  %16);
  Fx2n485bd_Send(Fx2n485bd_SendBuffer, 17);
}

void fx2n485bd_write(u8 regtype,u16 addr,u16 lenth)
{
  u8 i;
  u8 sum;
  u8 lenTemp;
  Fx2n485bd_SendBuffer[0]=5;	 
  Fx2n485bd_SendBuffer[1]=DEC2ASCII(RegisterCfgBuff[Fx2n485bd][g_u8_RespondID].plcaddrstation/16);
  Fx2n485bd_SendBuffer[2]=DEC2ASCII(RegisterCfgBuff[Fx2n485bd][g_u8_RespondID].plcaddrstation%16);
  Fx2n485bd_SendBuffer[3]='F';
  Fx2n485bd_SendBuffer[4]='F';			  
  if((regtype=='X')||(regtype=='Y')||(regtype=='M'))
	{
		Fx2n485bd_SendBuffer[5]='B';	
	}
	else if((regtype=='D')||(regtype=='T'))
	{
		Fx2n485bd_SendBuffer[5]='W';	
	}
	else
	{
		
	}				   
  Fx2n485bd_SendBuffer[6]='W'; 
  Fx2n485bd_SendBuffer[7]='A'; 
  Fx2n485bd_SendBuffer[8]=regtype;	  

  //地址
  if((regtype=='C')||(regtype=='T'))
  {
    Fx2n485bd_SendBuffer[ 9]='N';
  }
  else
  {
    Fx2n485bd_SendBuffer[ 9]=DEC2ASCII( addr / 1000);
  }
  Fx2n485bd_SendBuffer[10]=DEC2ASCII((addr % 1000)/100);
  Fx2n485bd_SendBuffer[11]=DEC2ASCII((addr % 100)/10);
  Fx2n485bd_SendBuffer[12]=DEC2ASCII( addr % 10);
  //长度
  Fx2n485bd_SendBuffer[13]=DEC2ASCII( lenth  /16);	  
  Fx2n485bd_SendBuffer[14]=DEC2ASCII( lenth  %16);

  if((regtype=='X')||(regtype=='Y')||(regtype=='M'))
  {
    Fx2n485bd_SendBuffer[15]=DEC2ASCII(g_u8_Writedata[1]&0x0f);
    lenTemp=lenth+15;			
  }
  else if((regtype=='D')||(regtype=='T'))
  {
    lenTemp=lenth*2;
    for(i=0;i<lenTemp;i++)
  	{
  	  Fx2n485bd_SendBuffer[15+2*i  ]=DEC2ASCII(g_u8_Writedata[i]  /16);
  	  Fx2n485bd_SendBuffer[15+2*i+1]=DEC2ASCII(g_u8_Writedata[i] %16);
  	}
  	lenTemp=4*lenth+15;
  }
  else
  {
  }
  //校验
  sum=0;
  
  for(i=1;i<lenTemp;i++)
  {
    sum+=Fx2n485bd_SendBuffer[i];
  }
  Fx2n485bd_SendBuffer[lenTemp]=DEC2ASCII(sum/16);  
  Fx2n485bd_SendBuffer[lenTemp+1]=DEC2ASCII(sum  %16);
  Fx2n485bd_Send(Fx2n485bd_SendBuffer, lenTemp+2);
}

void fx2n485bd_RecDataHandle(u8 regtype,u8 *sourcebuf,u8 *targetbuf,u8 datalen)
{
  u8 i;
  if((regtype=='X')||(regtype=='Y')||(regtype=='M'))
  {
    for(i=0;i<(datalen-7);i++)
    {
      targetbuf[2*i]=ASCII2BYTE('0','0');
      targetbuf[2*i+1]=ASCII2BYTE('0',sourcebuf[4+i]);
    }
    g_u16_RecvTransLen[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]]= 
       RegisterCfgBuff[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]].datalenth*2;
  }
  else if((regtype=='D')||(regtype=='T')||(regtype=='C'))
  {
    for(i=0;i<(datalen-7)/4;i++)
    {
      targetbuf[2*i]=ASCII2BYTE(sourcebuf[4+4*i+0],sourcebuf[4+4*i+1]);
      targetbuf[2*i+1]=ASCII2BYTE(sourcebuf[4+4*i+2],sourcebuf[4+4*i+3]);
    }
    g_u16_RecvTransLen[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]]=
        RegisterCfgBuff[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]].datalenth * 2;
  }
  else
  {

  }
}

void UART_Fx2n485bd_Recv(u8 data)
{
  u8 i;			  
  static u8 chksum;
	//data = data & 0x7f;
	if(2==data)
  {
    headRecvd=1;
		frameLen=0;
  }
  else if((6==data)||(1==writeResp) )
  {
    if(6==data)
		{  
	    frameLen=0;
		  writeResp=1;
		}
		else if(1==writeResp) 
		{
		  frameLen++;
		  if((1==frameLen)&&(data==DEC2ASCII(RegisterCfgBuff[Fx2n485bd][g_u8_RespondID].plcaddrstation/16)))
		  {
		  }
		  else if((2==frameLen)&&(data==DEC2ASCII(RegisterCfgBuff[Fx2n485bd][g_u8_RespondID].plcaddrstation%16)))
		  {
		  }	
		  else if((3==frameLen)&&(data=='F'))
		  {
		  }
		  else if((4==frameLen)&&(data=='F'))
		  {
		    if(GW_WriteStatus(Fx2n485bd)==WRITE_WAITRESPOND)
        {
          GW_WriteStatus(Fx2n485bd)=WRITE_RECVSUCCESS;
          memcpy(&g_u8_EthRespData[Fx2n485bd][(g_u16_StartAddr[Fx2n485bd][g_u8_RespondID]+g_u8_WriteAddrOffset)*2],
                  &g_u8_Writedata,g_u16_WriteLen*2);
        }
			  writeResp=0;	
		  }	
		  else
		  {				 
			  writeResp=0;
			  GW_WriteStatus(Fx2n485bd)=WRITE_RECVSUCCESS;
				NegativeResponse(Err_MBcmd);
		  }
		}
  }
  else if(1==headRecvd)
  {
    Fx2n485bdReceData[frameLen]=data;
		frameLen++;	  
		if(3==data)
		{
		  //chksum test
		  chksum=0;
		  for(i=0;i<frameLen;i++)
		  {
		    chksum+=Fx2n485bdReceData[i];
		  }
		  frameLenWait=2;
		}
		else if(frameLenWait>0)
		{
		  frameLenWait--;
		  if(0==frameLenWait)
		  {
		    if((Fx2n485bdReceData[frameLen-2]==DEC2ASCII(chksum/16))&&(Fx2n485bdReceData[frameLen-1]==DEC2ASCII(chksum%16)))
  			{
          fx2n485bd_RecDataHandle(RegisterCfgBuff[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]].regtype,Fx2n485bdReceData,
		  	&g_u8_EthRespData[Fx2n485bd][g_u16_StartAddr[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]]*2],frameLen);
  			  GW_ReadStatus(Fx2n485bd)=READ_RECVSUCCESS;
  			}
		  }
		}
  }
}	

/*这个task实现周期读取
1.每100ms读取一个参数
2.如果长期得不到响应，通知上位机通信失败
idx0:M98
idx1:Y0
idx2:D106
idx3:D1072
idx4:Y032
idx5:D100 
idx6:D101 
*/
void f_Fx2n485bd_task(void)
{				
  u8 datatyppe;
  u16 dataaddr;
  if(0!=ThreadNew(Fx2n485bd))
  {
    ThreadNew(Fx2n485bd)=0;
    GW_ReadStatus(Fx2n485bd)=READ_WAITRESPOND;
    dataaddr=(RegisterCfgBuff[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]].regaddr[6]<<8)
      + RegisterCfgBuff[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]].regaddr[7];
    fx2n485bd_read(RegisterCfgBuff[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]].regtype,dataaddr,
      RegisterCfgBuff[Fx2n485bd][g_u8_threadIdx[Fx2n485bd]].datalenth);
  }
  else if(WRITE_PRESEND==GW_WriteStatus(Fx2n485bd))
  {		   
    GW_WriteStatus(Fx2n485bd)=WRITE_WAITRESPOND;
    dataaddr=(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
    fx2n485bd_write(RegisterCfgBuff[Fx2n485bd][g_u8_RespondID].regtype,dataaddr,g_u16_WriteLen);
  }
  else
  {

  }
}

//#endif
