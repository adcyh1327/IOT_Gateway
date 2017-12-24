#include "main.h"
//#include "config.h"

#define MITSU_A_SEND_LENGTH            120          //发送缓冲区大小
#define MITSU_A_RECE_LENGTH            120         //接受缓冲区大小

static u8 MitsubishiA_SendBuf[MITSU_A_SEND_LENGTH];
static u8 MitsubishiAReceData[MITSU_A_RECE_LENGTH];          //板卡接受设备返回数据的缓冲区
static u8 MitsubishiA=0xff;;
static u8 MitsubishiA_Enable=OFF;
static u8 Bit_Read_Req[13]={0x00,0xF1,0x00,0x00,0xFF,0x03,0x0B,0x00,0x1E,0x01,0x00,0x00,0xFF};
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

void Get_PLCType(void)
{
  u8 buf[2];
	RTS_DISABLE;
  Delay1ms(50);
  buf[0]=0xAA;
  DAQ_UartSend(buf,1,CHN_UART_CFG);
  Delay1ms(50);
  
  RTS_ENABLE;
  Delay1ms(50);
	buf[0]=0xB0;
  DAQ_UartSend(buf,1,CHN_UART_CFG);
  Feach_type=ON;
  wr_index=0;
  Delay1ms(200);
  Feach_type=OFF;
}
	
void MitsubishiA_Init(u8 nodeID)
{
  u8 i,buf[2];
  for(i=0;i<MITSU_A_RECE_LENGTH;i++)
  {
    MitsubishiAReceData[i]=0;
  }
  for(i=0;i<MITSU_A_SEND_LENGTH;i++)
  {
    MitsubishiA_SendBuf[i]=0;
  }
  MitsubishiA = nodeID;
  Feach_type=OFF;

}


//======-------------------------------------------------------------------------------------------------------
void MitsubishiA_Calc_Check(unsigned char *l_u8ArrCheckData, unsigned char l_u8CheckDataLen, unsigned char *pl_u8CheckH, unsigned char *pl_u8CheckL)
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



void MitsubishiA_Send(u8 *sendbuf,u8 len)
{
  u8 buf[2];
  u32 delay;

  RTS_DISABLE;
  Delay1ms(2);
  buf[0]=PLC_type;
  DAQ_UartSend(buf,1,CHN_UART_CFG);
  delay=5000;
  
  RTS_ENABLE;
  Delay1ms(2);
  DAQ_UartSend(sendbuf,len,CHN_UART_CFG);
  delay=50000;
  
}


void MitsubishiA_RecDataHandle(u8 regtype,u8 *sourcebuf,u8 *targetbuf,u8 datalen)
{
  u8 i,temp;
  if((RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].regtype=='X')||
     (RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].regtype=='Y')||
     (RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].regtype=='M'))
  {
    for(i=0;i<datalen;i++)
    {
      temp=(RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].regaddr[6]<<8) +
            RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].regaddr[7]+i;
      temp=temp%16;
      targetbuf[i*2]=0x00;
      targetbuf[i*2+1]=((sourcebuf[i*2]*256+sourcebuf[i*2+1])>>(temp));
    }
    g_u16_RecvTransLen[MitsubishiA][g_u8_threadIdx[MitsubishiA]]= 
       RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].datalenth*2;
  }
  else if((RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].regtype == 'D') ||
          (RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].regtype == 'T'))
  {
    for(i=0;i<datalen*2;i++)
    {
      targetbuf[2*i]  =sourcebuf[2*i+1];
      targetbuf[2*i+1]  =sourcebuf[2*i];
    }
    g_u16_RecvTransLen[MitsubishiA][g_u8_threadIdx[MitsubishiA]]=
          RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].datalenth * 2;
  }
  else
  {

  }
}

//======-----------------------------------------------------------------------------------------------
//函数名：uart5_rs232_rece
//功能：  串口数据接受及处理
//参数：  串口数据寄存器数据
//使用：  函数放入串口接受中断中，传入接受的字节数据。
//        数据完成标志g_u8FlagRece,数据(纯寄存器)长度g_u8LengthArrEditData,数据g_u8ArrEditData[]
void MitsubishiA_Rece(unsigned char  l_u8ReceData)
{//串口接收中断
  static unsigned char sl_u8FlagGetData   = FALSE;
  static unsigned char sl_u8FlagEndData   = FALSE;
  static unsigned char sl_u8CountGetData  = FALSE;
  static unsigned char sl_u8CountCheck    = NONE;
  static unsigned char sl_u8DataCheckH    = NONE;
  static unsigned char sl_u8DataCheckL    = NONE;
  unsigned char l_u8FlagEndRece           = FALSE;
  unsigned char l_u8DataCheck             = NONE;
  unsigned char i = NONE;
  unsigned int  j;
  u8 n = 0;
  u8 high;
  u8 low;

  //接受帧头
  if (((0x10+(RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].datalenth*2)) == 
  	l_u8ReceData) && (FALSE == sl_u8FlagGetData))
  {//当前没有在接受帧内数据
    sl_u8FlagGetData    = TRUE;//正在接受数据
    sl_u8CountGetData   = NONE;
    //Clear_ArrReceData();
    MitsubishiAReceData[sl_u8CountGetData] = l_u8ReceData;
		return;
  }
  //接受数据
  if (TRUE == sl_u8FlagGetData)
  {
    sl_u8CountGetData++;
    MitsubishiAReceData[sl_u8CountGetData] = l_u8ReceData;
    if(sl_u8CountGetData>=(MitsubishiAReceData[0]+1))
    {
			sl_u8FlagGetData =FALSE;
			l_u8DataCheck=0;
		  for(i=0;i<sl_u8CountGetData;i++)
		  {
		    l_u8DataCheck+=MitsubishiAReceData[i];
		  }
		  if(l_u8DataCheck == MitsubishiAReceData[sl_u8CountGetData])
		  {
				GW_ReadStatus(MitsubishiA)= READ_RECVSUCCESS;
	      MitsubishiA_RecDataHandle(RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].regtype,
	          &MitsubishiAReceData[17],
						&g_u8_EthRespData[MitsubishiA][g_u16_StartAddr[MitsubishiA][g_u8_threadIdx[MitsubishiA]]*2],
	          RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].datalenth);	 
	    }
	    else
	    {
				NegativeResponse(Err_MBcmd);
	    }
			sl_u8CountGetData=0;
    }
  }
}

void UART_MitsubishiA_Recv(u8 data)
{
	if(Feach_type==ON)
	{
		wr_index++;//用于接收PLC类型
		if(wr_index==2)
		{
      PLC_type=data;
      MitsubishiA_Enable=ON;
		}
	}
  else if(WRITE_WAITRESPOND==GW_WriteStatus(MitsubishiA))
	{
		if((RegisterCfgBuff[MitsubishiA][g_u8_RespondID].regtype=='X')||
		    (RegisterCfgBuff[MitsubishiA][g_u8_RespondID].regtype=='Y')||
		    (RegisterCfgBuff[MitsubishiA][g_u8_RespondID].regtype=='M'))
		{
			if((wr_index<Bit_Set_Res[0])&&(data==Bit_Set_Res[wr_index]))
			{
				wr_buf[wr_index]=data;
				wr_index++;
				if(wr_index==Bit_Set_Res[0]-1)
				{
					GW_WriteStatus(MitsubishiA)=WRITE_RECVSUCCESS;
					memcpy(&g_u8_EthRespData[MitsubishiA][(g_u16_StartAddr[MitsubishiA][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
				}
			}
			else
			{
				NegativeResponse(Err_MBcmd);
			}
  	}
		else if((RegisterCfgBuff[MitsubishiA][g_u8_RespondID].regtype=='D')||
		    (RegisterCfgBuff[MitsubishiA][g_u8_RespondID].regtype=='T')||
		    (RegisterCfgBuff[MitsubishiA][g_u8_RespondID].regtype=='C'))
		{
			if((wr_index<Word_Set_Res[0])&&(data==Word_Set_Res[wr_index]))
			{
				wr_buf[wr_index]=data;
				wr_index++;
				if(wr_index==Word_Set_Res[0]-1)
				{
					GW_WriteStatus(MitsubishiA)=WRITE_RECVSUCCESS;
					memcpy(&g_u8_EthRespData[MitsubishiA][g_u16_StartAddr[MitsubishiA][g_u8_RespondID]*2],
                  &g_u8_Writedata,g_u16_WriteLen*2);
				}
			}
			else
			{
				NegativeResponse(Err_MBcmd);
			}
  	}
  	else
		{
			NegativeResponse(Err_MBcmd);
		}
	}
	else
	{
    MitsubishiA_Rece(data);
	}	
}

		 																		
void MitsubishiA_Write(u8 regtype,u8 l_u8TypeReg, u16  l_u16Offset, u8 l_u8Len)
{
  unsigned int  l_u16Adress   = 0;//寄存器地址
  unsigned char l_u8LenByteH  = 0;//ASCII值 字节的高4位
  unsigned char l_u8LenByteL  = 0;//ASCII值 字节的低4位
  unsigned char l_u8CheckH    = 0;//ASCII值 校验和的高4位
  unsigned char l_u8CheckL    = 0;//ASCII值 校验和的低4位
  u8 i,sum,temp,index=0;
  wr_index=0;
  memset(wr_buf,0,50);
  if((regtype=='X')||(regtype=='Y')||(regtype=='M'))
  {
    if((l_u8TypeReg == 'x')||(l_u8TypeReg == 'X'))
		{
			l_u16Adress = (l_u16Offset/8)+0x0000;
		}
		else if((l_u8TypeReg == 'y')||(l_u8TypeReg == 'Y'))
		{
			l_u16Adress = (l_u16Offset/8) + 0x800;
		}
		else if((l_u8TypeReg == 'm')||(l_u8TypeReg == 'M'))
		{
			l_u16Adress = (l_u16Offset/8) + 0xC00;
		}
    index=0;
		MitsubishiA_SendBuf[index++]=0x1A;
		memcpy(&MitsubishiA_SendBuf[1],Bit_Set_Req,sizeof(Bit_Set_Req));
		index += sizeof(Bit_Set_Req);
		if(g_u8_Writedata[1]&0x01)
  	{
			MitsubishiA_SendBuf[index++]  = 0x00;
		}
		else
  	{
			MitsubishiA_SendBuf[index++]  = 0x01;
		}
		//地址
		MitsubishiA_SendBuf[index++]  = l_u16Adress%256;
  	MitsubishiA_SendBuf[index++]  = l_u16Adress/256;
  	MitsubishiA_SendBuf[index++]  = 0x09;
  	if(g_u8_Writedata[1]&0x01)
  	{
  		MitsubishiA_SendBuf[index++]  = ((g_u8_Writedata[1]&0x01)<<(l_u16Offset%8));
  	}
  	else
  	{
			MitsubishiA_SendBuf[index++]  = 0x01;
			temp=0;
			for(i=0;i<8;i++)
			{
				temp += ((l_u16Offset%8)>>i)&0x01;
			}
			MitsubishiA_SendBuf[index++] = 0xff&(~(0x01<<temp));
  	}
	}
	else if((regtype=='D')||(regtype=='T')||(regtype=='C'))
	{
    if((l_u8TypeReg == 'd')||(l_u8TypeReg == 'D'))
		{
			l_u16Adress = 0x6800 + 2 * l_u16Offset;
		}
		else if((l_u8TypeReg == 'd')||(l_u8TypeReg == 'D'))
		{
			l_u16Adress = 0xA800 + 2 * l_u16Offset;
		}
		else
		{

		}
		index=0;
		MitsubishiA_SendBuf[index++]=0x1A;
		memcpy(&MitsubishiA_SendBuf[1],Word_Set_Req,sizeof(Word_Set_Req));
		index += sizeof(Word_Set_Req);
		//地址
		MitsubishiA_SendBuf[index++]  = l_u16Adress%256;
  	MitsubishiA_SendBuf[index++]  = l_u16Adress/256;
  	MitsubishiA_SendBuf[index++]  = 0x09;
  	MitsubishiA_SendBuf[index++]  = g_u8_Writedata[1];
  	MitsubishiA_SendBuf[index++]  = 0x02;
  	MitsubishiA_SendBuf[index++]  = (l_u16Adress+l_u8Len)%256;
  	MitsubishiA_SendBuf[index++]  = (l_u16Adress+l_u8Len)/256;
  	MitsubishiA_SendBuf[index++]  = 0x09;
  	MitsubishiA_SendBuf[index++]  = g_u8_Writedata[0];
  }
  MitsubishiA_SendBuf[0] = index-1;
  //校验
	sum=0;
	for(i=0;i<index;i++)
	{
		sum+=MitsubishiA_SendBuf[i];
	}
	MitsubishiA_SendBuf[index++]  = sum;
  MitsubishiA_Send(MitsubishiA_SendBuf,index);
}
//======---------------------------------------------------------------------------------
void TIM_1ms_MitsubishiA(void)
{
  if(MitsubishiA_Enable==ON)
  {
  	g_u16_SwitchTimer[MitsubishiA]++;
  }
  else
  {
		Timecouner++;
  }
  if((ON==MB_NeedWrite(MitsubishiA))&&(READ_IDLE==GW_ReadStatus(MitsubishiA)))
	{ 	
    if(GW_WriteStatus(MitsubishiA)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[MitsubishiA]>1000)
	     {
	     		g_u16_SwitchTimer[MitsubishiA]=0;
					GW_WriteStatus(MitsubishiA)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(MitsubishiA)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[MitsubishiA]>WriteTime))
	     {	
		   		GW_WriteStatus(MitsubishiA)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[MitsubishiA]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(MitsubishiA)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(MitsubishiA)=WRITE_DELAY;
      g_u16_SwitchTimer[MitsubishiA]=0;
	  }
	  else if(g_u16_SwitchTimer[MitsubishiA]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[MitsubishiA]=0;
      GW_WriteStatus(MitsubishiA)=WRITE_IDLE;
	    MB_NeedWrite(MitsubishiA)=OFF;
	    g_u8_threadIdx[MitsubishiA]++;
		  ThreadNew(MitsubishiA)=ON;
	  }
	  else{

	  }
	}
	else
	{
		if((g_u16_SwitchTimer[MitsubishiA]>UpdateCycle[MitsubishiA])&&
		    (g_u8_ProtocalNum[MitsubishiA][READ]!=0))
		{	  
		  g_u16_SwitchTimer[MitsubishiA]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(MitsubishiA))
		  {
		    GW_ReadStatus(MitsubishiA)=READ_IDLE;
		    g_u16_TimeoutCnt[MitsubishiA][g_u8_threadIdx[MitsubishiA]]=0;
		    if(OFF==MB_NeedWrite(MitsubishiA))
		    {
          g_u8_threadIdx[MitsubishiA]++;
          while(RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[MitsubishiA]++;
            if(g_u8_ProtocalNum[MitsubishiA][READ]<=g_u8_threadIdx[MitsubishiA])
      			{			  
      	      g_u8_threadIdx[MitsubishiA]=0;
      			}
          }
		      ThreadNew(MitsubishiA)=ON;
		    }
		    else
		    {
          
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[MitsubishiA][g_u8_threadIdx[MitsubishiA]]++;
        if(g_u16_TimeoutCnt[MitsubishiA][g_u8_threadIdx[MitsubishiA]]>
          (THRES_TIMOUTCNT/UpdateCycle[MitsubishiA]))
        {
          g_u16_RecvTransLen[MitsubishiA][g_u8_threadIdx[MitsubishiA]]=0;
          g_u16_TimeoutCnt[MitsubishiA][g_u8_threadIdx[MitsubishiA]]=0;
          if(OFF==MB_NeedWrite(MitsubishiA))
  		    {
            g_u8_threadIdx[MitsubishiA]++;
            while(RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[MitsubishiA]++;
              if(g_u8_ProtocalNum[MitsubishiA][READ]<=g_u8_threadIdx[MitsubishiA])
        			{			  
        	      g_u8_threadIdx[MitsubishiA]=0;
        			}
            }
  		      ThreadNew(MitsubishiA)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(MitsubishiA)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[MitsubishiA][READ]<=g_u8_threadIdx[MitsubishiA])
	{			  
		g_u8_threadIdx[MitsubishiA]=0;
	}
}




//======------------------------------------------------------------------------------
void MitsubishiA_read(u8 regtype,u8 LeiXing, u16 Adderadd,u8 len )
{
	u8 sum = 0,index=0;
	u8 i = 0;
	if((regtype=='X')||(regtype=='Y')||(regtype=='M'))
	{
		if((LeiXing == 'x')||(LeiXing == 'X'))
		{
			Adderadd = (Adderadd/8)+0x0000;
		}
		else if((LeiXing == 'y')||(LeiXing == 'Y'))
		{
			Adderadd = (Adderadd/8) + 0x800;
		}
		else if((LeiXing == 'm')||(LeiXing == 'M'))
		{
			Adderadd = (Adderadd/8) + 0xC00;
		}
		MitsubishiA_SendBuf[index++]=0x13;
		memcpy(&MitsubishiA_SendBuf[1],Bit_Read_Req,sizeof(Bit_Read_Req));
		index += sizeof(Bit_Read_Req);
		//地址
		MitsubishiA_SendBuf[index++]  = 0x02;
		MitsubishiA_SendBuf[index++]  = Adderadd%256;
  	MitsubishiA_SendBuf[index++]  = Adderadd/256;
  	MitsubishiA_SendBuf[index++]  = 0x09;
  	MitsubishiA_SendBuf[index++]  = 0x02;
  	MitsubishiA_SendBuf[index++]  = 0x00;
	}
	else if((regtype=='D')||(regtype=='T')||(regtype=='C'))
	{
		if((LeiXing == 'd')||(LeiXing == 'D'))
		{
			Adderadd = 0x6800 + 2 * Adderadd;
		}
		else if((LeiXing == 'd')||(LeiXing == 'D'))
		{
			Adderadd = 0xA800 + 2 * Adderadd;
		}
		else
		{

		}
		MitsubishiA_SendBuf[index++]=0x1B;
		memcpy(&MitsubishiA_SendBuf[1],Word_Read_Req1,sizeof(Word_Read_Req1));
		index += sizeof(Word_Read_Req1);
		//地址
		MitsubishiA_SendBuf[index++]  = Adderadd%256;
  	MitsubishiA_SendBuf[index++]  = Adderadd/256;
  	MitsubishiA_SendBuf[index++]  = 0x09;
  	MitsubishiA_SendBuf[index++]  = (Adderadd+len)%256;
  	MitsubishiA_SendBuf[index++]  = (Adderadd+len)/256;
  	MitsubishiA_SendBuf[index++]  = 0x09;
  	//校验
  	sum=0;
	  for(i=0;i<index;i++)
	  {
	    sum+=MitsubishiA_SendBuf[i];
	  }
	  MitsubishiA_SendBuf[index++]  = sum;
  	MitsubishiA_Send(MitsubishiA_SendBuf,index);
  	Delay1ms(10);
  	index=sizeof(Word_Read_Req2);
  	memcpy(MitsubishiA_SendBuf,Word_Read_Req2,index);
	}
	else
	{

	}
	
	//校验
	sum=0;
	for(i=0;i<index;i++)
	{
		sum+=MitsubishiA_SendBuf[i];
	}
	MitsubishiA_SendBuf[index++]  = sum;
  MitsubishiA_Send(MitsubishiA_SendBuf,index);
}
//===----------------------------------------------------------------------------
void f_MitsubishiA_task(void)
{
	u8 i = 0;
	u16 dataaddr;
	if(Timecouner>=500)
	{
		Timecouner=0;
		Get_PLCType();
	}
	//当前空闲 且 没有写操作
	if(ThreadNew(MitsubishiA) == ON)
	{
		ThreadNew(MitsubishiA) = OFF;
		GW_ReadStatus(MitsubishiA)=READ_WAITRESPOND;
    dataaddr=(RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].regaddr[6]<<8)
      + RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].regaddr[7];
    MitsubishiA_read(RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].regtype,
    	RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].regtype,dataaddr,
      RegisterCfgBuff[MitsubishiA][g_u8_threadIdx[MitsubishiA]].datalenth);
	}	
  else if(WRITE_PRESEND==GW_WriteStatus(MitsubishiA))
  {		   
    GW_WriteStatus(MitsubishiA)=WRITE_WAITRESPOND;
    dataaddr=(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
    MitsubishiA_Write(RegisterCfgBuff[MitsubishiA][g_u8_RespondID].regtype,
    	RegisterCfgBuff[MitsubishiA][g_u8_RespondID].regtype,dataaddr,g_u16_WriteLen);	// 孔数
  }
  else
  {

  }
}
