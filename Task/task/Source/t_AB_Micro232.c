#include "main.h"
//#include "config.h"

#define AB_MICRO_SEND_LENGTH            220          //发送缓冲区大小
#define AB_MICRO_RECE_LENGTH            220         //接受缓冲区大小

#define FILE_INDEX     RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regaddr[4]   
#define ELEMENT_IDX    RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regaddr[6] 
#define ELEMENT_SUBIDX RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regaddr[7]


static u8 AB_Micro232_SendBuf[AB_MICRO_SEND_LENGTH];
static u8 AB_Micro232_RecBuf[AB_MICRO_RECE_LENGTH];          //板卡接受设备返回数据的缓冲区
static u8 AB_Micro232=0xff;;
static u8 AB_Micro232_Enable=OFF;
static u8 FrameStart=OFF;
static u8 FrameEnd=OFF;
static u8 AppendNum=0;
static u8 AppendFlag=OFF;
static u8 ReqHeader[6]={0x10,0x02,0x01,0x00,0x0F,0x00};
static u8 N_ResHeader[6]={0x10,0x02,0x00,0x01,0x4F,0x00};
static u8 I_ResHeader[7]={0x10,0x02,0x00,0x01,0x4F,0x10,0x10};
static u8 Bit_Read_Res[15]={0x00,0xF1,0x00,0x00,0xFE,0x03,0x0A,0x00,0x9E,0x01,0x00,0x00,0x00,0x00,0xFF};
static u8 Bit_Set_Req[16]= {0x00,0xF1,0x00,0x00,0xFF,0x03,0x0D,0x00,0x1E,0x01,0x00,0x00,0xFF,0x06,0x01,0x00};
static u8 Bit_Set_Res[18]= {0x10,0x00,0xF1,0x00,0x00,0xFE,0x03,0x08,0x00,0x9E,0x01,0x00,0x00,0x00,0x00,0xFF,0x06,0xAE};
static u8 Word_Read_Req1[21]= {0x00,0xF1,0x00,0x00,0xFF,0x03,0x7F,0x00,0x1E,0x01,0x00,0x00,0xFF,0x04,0x3E,0x13,0x00,0x74,0x00,0x02,0x00};
static u8 Word_Read_Req2[15]= {0x0E,0x00,0xF1,0x00,0x00,0xFF,0x03,0x06,0x00,0x1E,0x01,0x00,0x00,0xFF,0x05};
static u8 Word_Read_Res[16] = {0x00,0xF1,0x00,0x00,0xFE,0x03,0x0A,0x00,0x9E,0x01,0x00,0x00,0x00,0x00,0xFF,0x05};
static u8 Word_Set_Req [17] = {0x00,0xF1,0x00,0x00,0xFF,0x03,0x12,0x00,0x1E,0x01,0x00,0x00,0xFF,0x06,0x02,0x00,0x02};
static u8 Word_Set_Res [17] = {0x10,0x00,0xF1,0x00,0x00,0xFE,0x03,0x08,0x00,0x9E,0x01,0x00,0x00,0x00,0x00,0xFF,0x06};
static u8 wr_buf[50];
static u8 wr_index;
static u8 PLC_type;
static u8 Feach_type;
static u16 Timecouner;
static u16 Framecount;


void AB_Micro232_Init(u8 nodeID)
{
  memset(AB_Micro232_SendBuf,0,AB_MICRO_SEND_LENGTH);
  memset(AB_Micro232_RecBuf,0,AB_MICRO_RECE_LENGTH);
  AB_Micro232 = nodeID;
  Feach_type=OFF;
  Framecount = 0x01;
  AB_Micro232_Enable=ON;
}

u8 AppendData(u8 *in_array,u8 in_num,u8 *out_array)
{
  u8 i,out_num,temp[120];
  memcpy(temp,in_array,in_num);
  out_num=0;
  for(i=0;i<in_num;i++)
  {
    out_array[out_num++]=temp[i];
    if(temp[i] == 0x10)
    {
      out_array[out_num++]=0x10;
    }
  }
  return out_num;
}

//======-------------------------------------------------------------------------------------------------------
void AB_Micro232_Calc_Check(unsigned char *l_u8ArrCheckData, unsigned char l_u8CheckDataLen, unsigned char *pl_u8CheckH, unsigned char *pl_u8CheckL)
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



void AB_Micro232_Send(u8 *sendbuf,u8 len)
{
  Framecount++;
  DAQ_UartSend(sendbuf,len,CHN_UART_CFG);  
}


void AB_Micro232_RecDataHandle(u8 regtype,u8 *sourcebuf,u8 *targetbuf,u8 datalen)
{
  u8 i,temp;
  if((RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regtype=='I')||
     (RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regtype=='O')||
     (RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regtype=='B'))
  {
    for(i=0;i<datalen;i++)
    {
      //mp=RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regaddr[7];
      //mp=temp%16;
      targetbuf[i*2]=0x00;
      targetbuf[i*2+1]=sourcebuf[i] ;
    }
    
  }
  else if(RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regtype == 'N')
  {
    for(i=0;i<datalen*2;i++)
    {
      targetbuf[2*i]  =sourcebuf[2*i+1];
      targetbuf[2*i+1]  =sourcebuf[2*i];
    }
  }
  else
  {

  }
  g_u16_RecvTransLen[AB_Micro232][g_u8_threadIdx[AB_Micro232]]= 
       RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].datalenth*2;
}

void UART_AB_Micro232_Recv(u8 data)
{
  u8 len;
  AB_Micro232_RecBuf[wr_index]=data;
  if(FrameStart==OFF)
  {
    if(RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regtype=='I')
    {
      if(data==I_ResHeader[wr_index])
      {
        wr_buf[wr_index++]=data;
        if(wr_index==sizeof(I_ResHeader))
        {
          FrameStart=ON;
          FrameEnd=OFF;
          Timecouner=0;
        }
      }
      else
      {
        wr_index=0x00;
      }
    }
    else if(RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regtype=='N')
    {
      if(data==N_ResHeader[wr_index])
      {
        wr_buf[wr_index++]=data;
        if(wr_index==sizeof(N_ResHeader))
        {
          FrameStart=ON;
          FrameEnd=OFF;
          Timecouner=0;
        }
      }
      else
      {
        wr_index=0x00;
      }
    }
    else
    {
      wr_index=0x00;
    }
  }
  else
  {
    if((AppendFlag==ON)&&(data==0X10))
    {
      AppendFlag=OFF;
    }
    else if((AppendFlag==OFF)&&(data==0X10))
    {
      AppendFlag=ON;
      wr_buf[wr_index++]=data;
    }
    else
    {
      AppendFlag=OFF;
      wr_buf[wr_index++]=data;
    }
    
    Timecouner=0;
    switch(RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regtype)
    {
      case 'I':
      case 'O':
      case 'B':
            len=RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].datalenth;
            if(wr_index==(11+len))
            {
              FrameEnd = ON;
            }
            break;
      case 'N':
            len=RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].datalenth*2;
            if(wr_index==(11+len))
            {
              FrameEnd = ON;
            }
            break;
    }
    if(FrameEnd == ON )
    {
      FrameEnd = OFF;
      GW_ReadStatus(AB_Micro232)= READ_RECVSUCCESS;
      AB_Micro232_RecDataHandle(RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regtype,&wr_buf[8],
        &g_u8_EthRespData[AB_Micro232][g_u16_StartAddr[AB_Micro232][g_u8_threadIdx[AB_Micro232]]*2],
        RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].datalenth);
    }
  }	
}

		 																		
void AB_Micro232_Write(u8 regtype,u8 l_u8TypeReg, u16  l_u16Offset, u8 len)
{
  u8 sum = 0,index=0;
	u8 i = 0;
	index=sizeof(ReqHeader);
	memset(AB_Micro232_RecBuf,0,AB_MICRO_RECE_LENGTH);
	memset(AB_Micro232_SendBuf,0,AB_MICRO_SEND_LENGTH);
	memcpy(AB_Micro232_SendBuf,ReqHeader,index);
	AB_Micro232_SendBuf[index++]=Framecount%256;
	AB_Micro232_SendBuf[index++]=Framecount/256;
	AB_Micro232_SendBuf[index++]=0xAA;
	switch(regtype)
	{
    case 'I':
    case 'O':
    case 'B':
          AB_Micro232_SendBuf[index++]=len;
          break;
    case 'N':
          AB_Micro232_SendBuf[index++]=len*2;
          break;
    case 'C':
    case 'T':
          AB_Micro232_SendBuf[index++]=len*6;
          break;
    default:break;
	}
	AB_Micro232_SendBuf[index++]=FILE_INDEX;//文件编号
	switch(regtype)//文件类型
	{
    case 'I':
          AB_Micro232_SendBuf[index++]=0x8C;
          break;
    case 'O':
          AB_Micro232_SendBuf[index++]=0x8B;
          break;
    case 'B':
          AB_Micro232_SendBuf[index++]=0x85;
          break;
    case 'N':
          AB_Micro232_SendBuf[index++]=0x89;
          break;
    case 'C':
          AB_Micro232_SendBuf[index++]=0x86;
          break;
    case 'T':
          AB_Micro232_SendBuf[index++]=0x87;
          break;
    default:break;
	}
	AB_Micro232_SendBuf[index++]=ELEMENT_IDX;
	AB_Micro232_SendBuf[index++]=ELEMENT_SUBIDX;
	for(i=0;i<len;i++)
	{
    AB_Micro232_SendBuf[index++]=g_u8_Writedata[2*i+1];
    AB_Micro232_SendBuf[index++]=g_u8_Writedata[2*i];
	}
	index=AppendData(&AB_Micro232_SendBuf[2],index-2,&AB_Micro232_SendBuf[2])+2;
	//校验
	sum=BCC_CalcCheck(&AB_Micro232_SendBuf[2],index);
	AB_Micro232_SendBuf[index++]  = 0x10;
	AB_Micro232_SendBuf[index++]  = 0x03;
	AB_Micro232_SendBuf[index++]  = sum;
  AB_Micro232_Send(AB_Micro232_SendBuf,index);
  GW_WriteStatus(AB_Micro232)=WRITE_RECVSUCCESS;
}
//======---------------------------------------------------------------------------------
void TIM_1ms_AB_Micro232(void)
{
  if(FrameStart==ON)
  {
    if(++Timecouner>10)
    {
      FrameStart =OFF;
    }
  }

  if(AB_Micro232_Enable==ON)
  {
    g_u16_SwitchTimer[AB_Micro232]++;
  }

  if((ON==MB_NeedWrite(AB_Micro232))&&(READ_IDLE==GW_ReadStatus(AB_Micro232)))
	{ 	
    if(GW_WriteStatus(AB_Micro232)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[AB_Micro232]>1000)
	     {
	     		g_u16_SwitchTimer[AB_Micro232]=0;
					GW_WriteStatus(AB_Micro232)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(AB_Micro232)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[AB_Micro232]>WriteTime))
	     {	
		   		GW_WriteStatus(AB_Micro232)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[AB_Micro232]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(AB_Micro232)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(AB_Micro232)=WRITE_DELAY;
      g_u16_SwitchTimer[AB_Micro232]=0;
	  }
	  else if(g_u16_SwitchTimer[AB_Micro232]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[AB_Micro232]=0;
      GW_WriteStatus(AB_Micro232)=WRITE_IDLE;
	    MB_NeedWrite(AB_Micro232)=OFF;
	    g_u8_threadIdx[AB_Micro232]++;
		  ThreadNew(AB_Micro232)=ON;
	  }
	  else{

	  }
	}
	else
	{
		if((g_u16_SwitchTimer[AB_Micro232]>UpdateCycle[AB_Micro232])&&
		    (g_u8_ProtocalNum[AB_Micro232][READ]!=0))
		{	  
		  g_u16_SwitchTimer[AB_Micro232]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(AB_Micro232))
		  {
		    GW_ReadStatus(AB_Micro232)=READ_IDLE;
		    g_u16_TimeoutCnt[AB_Micro232][g_u8_threadIdx[AB_Micro232]]=0;
		    if(OFF==MB_NeedWrite(AB_Micro232))
		    {
          g_u8_threadIdx[AB_Micro232]++;
          while(RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[AB_Micro232]++;
            if(g_u8_ProtocalNum[AB_Micro232][READ]<=g_u8_threadIdx[AB_Micro232])
      			{			  
      	      g_u8_threadIdx[AB_Micro232]=0;
      			}
          }
		      ThreadNew(AB_Micro232)=ON;
		    }
		    else
		    {
          
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[AB_Micro232][g_u8_threadIdx[AB_Micro232]]++;
        if(g_u16_TimeoutCnt[AB_Micro232][g_u8_threadIdx[AB_Micro232]]>
          (THRES_TIMOUTCNT/UpdateCycle[AB_Micro232]))
        {
          g_u16_RecvTransLen[AB_Micro232][g_u8_threadIdx[AB_Micro232]]=0;
          g_u16_TimeoutCnt[AB_Micro232][g_u8_threadIdx[AB_Micro232]]=0;
          if(OFF==MB_NeedWrite(AB_Micro232))
  		    {
            g_u8_threadIdx[AB_Micro232]++;
            while(RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[AB_Micro232]++;
              if(g_u8_ProtocalNum[AB_Micro232][READ]<=g_u8_threadIdx[AB_Micro232])
        			{			  
        	      g_u8_threadIdx[AB_Micro232]=0;
        			}
            }
  		      ThreadNew(AB_Micro232)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(AB_Micro232)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[AB_Micro232][READ]<=g_u8_threadIdx[AB_Micro232])
	{			  
		g_u8_threadIdx[AB_Micro232]=0;
	}
}




//======------------------------------------------------------------------------------
void AB_Micro232_read(u8 regtype,u8 LeiXing, u16 Adderadd,u8 len )
{
	u8 sum = 0,index=0;
	u8 i = 0;
	index=sizeof(ReqHeader);
	memset(AB_Micro232_RecBuf,0,AB_MICRO_RECE_LENGTH);
	memset(AB_Micro232_SendBuf,0,AB_MICRO_SEND_LENGTH);
	memcpy(AB_Micro232_SendBuf,ReqHeader,index);
	AB_Micro232_SendBuf[index++]=Framecount%256;
	AB_Micro232_SendBuf[index++]=Framecount/256;
	AB_Micro232_SendBuf[index++]=0xA2;
	switch(regtype)
	{
    case 'I':
    case 'O':
    case 'B':
          AB_Micro232_SendBuf[index++]=len;
          break;
    case 'N':
          AB_Micro232_SendBuf[index++]=len*2;
          break;
    case 'C':
    case 'T':
          AB_Micro232_SendBuf[index++]=len*6;
          break;
    default:break;
	}
	AB_Micro232_SendBuf[index++]=FILE_INDEX;//文件编号
	switch(regtype)//文件类型
	{
    case 'I':
          AB_Micro232_SendBuf[index++]=0x8C;
          break;
    case 'O':
          AB_Micro232_SendBuf[index++]=0x8B;
          break;
    case 'B':
          AB_Micro232_SendBuf[index++]=0x85;
          break;
    case 'N':
          AB_Micro232_SendBuf[index++]=0x89;
          break;
    case 'C':
          AB_Micro232_SendBuf[index++]=0x86;
          break;
    case 'T':
          AB_Micro232_SendBuf[index++]=0x87;
          break;
    default:break;
	}
	AB_Micro232_SendBuf[index++]=ELEMENT_IDX;
	AB_Micro232_SendBuf[index++]=ELEMENT_SUBIDX;
	index=AppendData(&AB_Micro232_SendBuf[2],index-2,&AB_Micro232_SendBuf[2])+2;
	//校验
	sum=BCC_CalcCheck(&AB_Micro232_SendBuf[2],index);
	AB_Micro232_SendBuf[index++]  = 0x10;
	AB_Micro232_SendBuf[index++]  = 0x03;
	AB_Micro232_SendBuf[index++]  = sum;
  AB_Micro232_Send(AB_Micro232_SendBuf,index);
}
//===----------------------------------------------------------------------------
void f_AB_Micro232_task(void)
{
	u8 i = 0;
	u16 dataaddr;
	if(Timecouner>=500)
	{
		Timecouner=0;
		Get_PLCType();
	}
	//当前空闲 且 没有写操作
	if(ThreadNew(AB_Micro232) == ON)
	{
		ThreadNew(AB_Micro232) = OFF;
		GW_ReadStatus(AB_Micro232)=READ_WAITRESPOND;
    dataaddr=(RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regaddr[6]<<8)
      + RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regaddr[7];
    AB_Micro232_read(RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regtype,
    	RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].regtype,dataaddr,
      RegisterCfgBuff[AB_Micro232][g_u8_threadIdx[AB_Micro232]].datalenth);
	}	
  else if(WRITE_PRESEND==GW_WriteStatus(AB_Micro232))
  {		   
    GW_WriteStatus(AB_Micro232)=WRITE_WAITRESPOND;
    dataaddr=(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
    AB_Micro232_Write(RegisterCfgBuff[AB_Micro232][g_u8_RespondID].regtype,
    	RegisterCfgBuff[AB_Micro232][g_u8_RespondID].regtype,dataaddr,g_u16_WriteLen);	// 孔数
  }
  else
  {

  }
}
