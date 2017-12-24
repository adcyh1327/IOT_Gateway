#include "main.h"
//#include "config.h"

#define FX2NPROGRAM_SEND_LENGTH             220          //发送缓冲区大小
#define FX2NPROGRAM_RECE_LENGTH             220         //接受缓冲区大小

static u8 Fx2n_SendBuffer[FX2NPROGRAM_SEND_LENGTH];
static u8 g_u8ArrReceData[FX2NPROGRAM_RECE_LENGTH];          //板卡接受设备返回数据的缓冲区

static signed char g_u8StateRece;                        //串口接受状态  //暂未发现用途
static u8 g_u8LengthArrEditData;              //纯数据长度    //暂未发现用途
static u8 g_u8LengthArrSend;              //发送的数据帧  //暂未发现用途
static u8 Fx2nProgram=0xff;
static u8 Fx2nProgram_Enable=OFF;

void Fx2nProgram_Init(u8 nodeID)
{
  u8 i;
  for(i=0;i<FX2NPROGRAM_RECE_LENGTH;i++)
  {
    g_u8ArrReceData[i]=0;
  }
  for(i=0;i<FX2NPROGRAM_SEND_LENGTH;i++)
  {
    Fx2n_SendBuffer[i]=0;
  }
  g_u8LengthArrSend=0;
  g_u8StateRece=0;
  g_u8LengthArrEditData=0;
  Fx2nProgram = nodeID;
  Fx2nProgram_Enable=ON;
}
  
void Clear_ArrSendData(void)
{
    unsigned char i = NONE;
    
    for (i=0; i<FX2NPROGRAM_SEND_LENGTH; i++)
    {
        Fx2n_SendBuffer[i]  = NONE;
    }

}


void Clear_ArrReceData(void)
{
	#if 0
    unsigned char i = NONE;
    
    for (i=0; i<UART5_RECE_LENGTH; i++)
    {
        g_u8ArrReceData[i]  = NONE;
    }
	#endif
}

void Fx2nProgram_Send(u8 *sendbuf,u8 len)
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


void Fx2nProgram_RecDataHandle(u8 regtype,u8 *sourcebuf,u8 *targetbuf,u8 datalen)
{
  u8 i,temp;
  if((RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].regtype=='X')||
     (RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].regtype=='Y')||
     (RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].regtype=='M'))
  {
    for(i=0;i<(datalen-4)/2;i++)
    {
      temp=(RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].regaddr[6]<<8) +
            RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].regaddr[7]+i;
      if(RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].regtype=='M')
      {
        temp=temp%8;
      }
      else
      {
        temp=temp%10;
      }
      targetbuf[i*2]=0x00;
      if(temp>3)
      {
        targetbuf[i*2+1]=(ASCIItoHEX(sourcebuf[1])>>(temp%4))&0x01;
      }
      else
      {
        targetbuf[i*2+1]=(ASCIItoHEX(sourcebuf[2])>>temp)&0x01;
      }
      //targetbuf[2*i+1]=((temp>3)?((ASCIItoHEX(sourcebuf[2])>>(temp%4)):(ASCIItoHEX(sourcebuf[3])>>temp)));
    }
    g_u16_RecvTransLen[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]]= 
       RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].datalenth*2;
  }
  else if((RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].regtype == 'D') ||
          (RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].regtype == 'T'))
  {
    for(i=0;i<(datalen-4)/4;i++)
    {
      targetbuf[2*i]  =(ASCIItoHEX(sourcebuf[4*i+3])<<4)+ASCIItoHEX(sourcebuf[4*i+4]);
      targetbuf[2*i+1]=(ASCIItoHEX(sourcebuf[4*i+1])<<4)+ASCIItoHEX(sourcebuf[4*i+2]);
    }
    g_u16_RecvTransLen[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]]=
          RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].datalenth * 2;
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
void UartRs232_SIMFX_Rece(unsigned char  l_u8ReceData)
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

	//l_u8ReceData = l_u8ReceData	& 0x7F;

  //PLC状态响应
  if (MIT_FX_ACK == l_u8ReceData)
  {//PLC正确相应
    g_u8StateRece    = TRUE;
    if(GW_WriteStatus(Fx2nProgram)==WRITE_WAITRESPOND)
    {
      GW_WriteStatus(Fx2nProgram)=WRITE_RECVSUCCESS;
      memcpy(&g_u8_EthRespData[Fx2nProgram][(g_u16_StartAddr[Fx2nProgram][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
    }
  }
  else if (MIT_FX_NCK == l_u8ReceData)
  {//PLC错误相应
    g_u8StateRece    = ERR;
    if(GW_WriteStatus(Fx2nProgram)==WRITE_WAITRESPOND)
    {
      GW_WriteStatus(Fx2nProgram)=WRITE_RECVSUCCESS;
      NegativeResponse(Err_MBcmd);
    }
  }
  else
  {//其他
    g_u8StateRece    = FALSE;
  }

  //接受帧头
  if ((MIT_FX_STX == l_u8ReceData) && (FALSE == sl_u8FlagGetData) && (FALSE == sl_u8FlagEndData))
  {//当前没有在接受帧内数据
    sl_u8FlagGetData    = TRUE;//正在接受数据
    sl_u8CountGetData   = NONE;
    memset(g_u8ArrReceData,0,FX2NPROGRAM_RECE_LENGTH);
    g_u8ArrReceData[sl_u8CountGetData] = l_u8ReceData;
		return;
  }

  //接受帧尾    
  if ((MIT_FX_ETX == l_u8ReceData) && (TRUE == sl_u8FlagGetData) && (FALSE == sl_u8FlagEndData))
  {//正在接受数据
    sl_u8FlagGetData    = FALSE;
    sl_u8FlagEndData    = TRUE;
    sl_u8CountCheck     = NONE;
		sl_u8CountGetData++;
    g_u8ArrReceData[sl_u8CountGetData] = l_u8ReceData;
		return;
  }

  //接受数据
  if (TRUE == sl_u8FlagGetData)
  {
    sl_u8CountGetData++;
    g_u8ArrReceData[sl_u8CountGetData] = l_u8ReceData;
  }

    //接受校验
  if (TRUE == sl_u8FlagEndData)
  {
    sl_u8CountCheck++;
		sl_u8CountGetData++;
    g_u8ArrReceData[sl_u8CountGetData] = l_u8ReceData;
    if (1 == sl_u8CountCheck)
    {
      sl_u8DataCheckH = l_u8ReceData;
    }
    if (2 == sl_u8CountCheck)
    {
      sl_u8DataCheckL = l_u8ReceData;
      sl_u8FlagEndData = FALSE;
      l_u8FlagEndRece = TRUE;
    }
  }

  //校验数据
  if (TRUE == l_u8FlagEndRece)
  {
    l_u8FlagEndRece = FALSE;
    for (i=1; i<=sl_u8CountGetData-2; i++)
    {
      l_u8DataCheck = l_u8DataCheck + g_u8ArrReceData[i];
    }
    if ((sl_u8DataCheckH == HEXtoASCII((l_u8DataCheck>>4)&0x0F)) && (sl_u8DataCheckL == HEXtoASCII(l_u8DataCheck&0x0F)))
    {
      GW_ReadStatus(Fx2nProgram)= READ_RECVSUCCESS;
      g_u8LengthArrEditData   = 0;

      Fx2nProgram_RecDataHandle(RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].regtype,g_u8ArrReceData,
	  	&g_u8_EthRespData[Fx2nProgram][g_u16_StartAddr[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]]*2],sl_u8CountGetData+1);	  
    }
    else
    {
        memset(g_u8ArrReceData,0,FX2NPROGRAM_RECE_LENGTH);
    }
  }
}

void UART_Fx2nProgram_Recv(u8 data)
{
  if((WRITE_WAITRESPOND==GW_WriteStatus(Fx2nProgram)) && (data == 0x06) )
	{
		if(GW_WriteStatus(Fx2nProgram)==WRITE_WAITRESPOND)
    {
      GW_WriteStatus(Fx2nProgram)=WRITE_RECVSUCCESS;
      memcpy(&g_u8_EthRespData[Fx2nProgram][(g_u16_StartAddr[Fx2nProgram][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
    }
	}
	else
	{
    UartRs232_SIMFX_Rece(data);
	}
}

		 																		
void Fx2nProgram_Write(unsigned char l_u8TypeReg, unsigned int  l_u16Offset, unsigned char l_u8Len)
{
    unsigned int  l_u16Adress   = 0;//寄存器地址
    unsigned char l_u8LenByteH  = 0;//ASCII值 字节的高4位
    unsigned char l_u8LenByteL  = 0;//ASCII值 字节的低4位
    unsigned char l_u8CheckH    = 0;//ASCII值 校验和的高4位
    unsigned char l_u8CheckL    = 0;//ASCII值 校验和的低4位
	  u8 i,index=0;
	  u8 j,sendbuf[200];
    l_u8Len = l_u8Len * 2;
    Clear_ArrSendData();
    Fx2n_SendBuffer[index++]  = 0x02;//帧头
    switch(l_u8TypeReg)
    {
        case 'Y':
            l_u16Adress=(l_u16Offset/10)*8+(l_u16Offset%10);
            if((g_u8_Writedata[1])&0x01)
            {
              Fx2n_SendBuffer[index++]  = 0x37;
            }
            else
            {
              Fx2n_SendBuffer[index++]  = 0x38;
            }
            Fx2n_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>4)&0x0F);//
            Fx2n_SendBuffer[index++]  = HEXtoASCII((l_u16Adress)&0x0F);//
            Fx2n_SendBuffer[index++]  = 0x30;//
            Fx2n_SendBuffer[index++]  = 0x35;//
            break;
        case 'M':
            l_u16Adress = 0x800+l_u16Offset;
            if((g_u8_Writedata[1])&0x01)
            {
              Fx2n_SendBuffer[index++]  = 0x37;
            }
            else
            {
              Fx2n_SendBuffer[index++]  = 0x38;
            }
            Fx2n_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>4)&0x0F);//
            Fx2n_SendBuffer[index++]  = HEXtoASCII((l_u16Adress)&0x0F);//
            Fx2n_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>12)&0x0F);//
            Fx2n_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>8)&0x0F);//
            break;
        case 'D': 
        case 'T':
        case 'C':
            if(l_u8TypeReg=='D')
            {
              l_u16Adress = 0x4000 + 2*l_u16Offset;
            }
            else if(l_u8TypeReg=='T')
            {
              l_u16Adress = 0x800+l_u16Offset*2;	
            }
            else if(l_u8TypeReg=='C')
            {
              l_u16Adress = 0xA00+l_u16Offset*2;	
            }
            else{

            }
            for(i=0;i<l_u8Len;i++)
            {
              sendbuf[4*i]=HEXtoASCII((g_u8_Writedata[i*2+1]>>4)&0x0F);
              sendbuf[4*i+1]=HEXtoASCII((g_u8_Writedata[i*2+1])&0x0F);
              sendbuf[4*i+2]=HEXtoASCII((g_u8_Writedata[i*2+0]>>4)&0x0F);
              sendbuf[4*i+3]=HEXtoASCII((g_u8_Writedata[i*2+0])&0x0F);
            }

            Fx2n_SendBuffer[index++]  = 0x45;
            Fx2n_SendBuffer[index++]  = 0x31;//命令-写 
			//dizhi 
            Fx2n_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>16)&0x0F);//
            Fx2n_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>12)&0x0F);//
            Fx2n_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>8)&0x0F);//
            Fx2n_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>4)&0x0F);//
            Fx2n_SendBuffer[index++]  = HEXtoASCII((l_u16Adress)&0x0F);//
			//changdu
            Fx2n_SendBuffer[index++]  = HEXtoASCII((l_u8Len>>4)&0x0F);//读取得字节数H
            Fx2n_SendBuffer[index++]  = HEXtoASCII(l_u8Len&0x0F);//读取得字节数L
			//shuju
          	for(i=0;i<l_u8Len*2 ;i++)
      			{						 // 34 12  31 32 33 34
      			  Fx2n_SendBuffer[index++] = sendbuf[i];
      			}
            
            break;
        default:
            break;
    }
    Fx2n_SendBuffer[index++ ]  = 0x03;//帧尾
                  //Calc_Check(g_u8ArrSendData, (11+l_u8LenReadWord*2), &l_u8CheckH, &l_u8CheckL);
		for(i=1;i<index;i++)
		{
			 l_u8CheckH += (Fx2n_SendBuffer[i]);
		}
    Fx2n_SendBuffer[index++] = HEXtoASCII((l_u8CheckH>>4)&0x0F);
    Fx2n_SendBuffer[index++] = HEXtoASCII(l_u8CheckH&0x0F);
    Fx2nProgram_Send(Fx2n_SendBuffer,index );
}
//======---------------------------------------------------------------------------------
void TIM_1ms_Fx2nProgram(void)
{
  if(Fx2nProgram_Enable==ON)
  {
  	g_u16_SwitchTimer[Fx2nProgram]++;
  }
  if((ON==MB_NeedWrite(Fx2nProgram))&&(READ_IDLE==GW_ReadStatus(Fx2nProgram)))
	{ 	
    if(GW_WriteStatus(Fx2nProgram)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[Fx2nProgram]>1000)
	     {
	     		g_u16_SwitchTimer[Fx2nProgram]=0;
					GW_WriteStatus(Fx2nProgram)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(Fx2nProgram)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[Fx2nProgram]>WriteTime))
	     {	
		   		GW_WriteStatus(Fx2nProgram)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[Fx2nProgram]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(Fx2nProgram)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(Fx2nProgram)=WRITE_DELAY;
      g_u16_SwitchTimer[Fx2nProgram]=0;
	  }
	  else if(g_u16_SwitchTimer[Fx2nProgram]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[Fx2nProgram]=0;
      GW_WriteStatus(Fx2nProgram)=WRITE_IDLE;
	    MB_NeedWrite(Fx2nProgram)=OFF;
	    g_u8_threadIdx[Fx2nProgram]++;
		  ThreadNew(Fx2nProgram)=ON;
	  }
	  else{

	  }
	}
	else
	{
		if((g_u16_SwitchTimer[Fx2nProgram]>UpdateCycle[Fx2nProgram])&&
		    (g_u8_ProtocalNum[Fx2nProgram][READ]!=0))
		{	  
		  g_u16_SwitchTimer[Fx2nProgram]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(Fx2nProgram))
		  {
		    GW_ReadStatus(Fx2nProgram)=READ_IDLE;
		    g_u16_TimeoutCnt[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]]=0;
		    if(OFF==MB_NeedWrite(Fx2nProgram))
		    {
          g_u8_threadIdx[Fx2nProgram]++;
          while(RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[Fx2nProgram]++;
            if(g_u8_ProtocalNum[Fx2nProgram][READ]<=g_u8_threadIdx[Fx2nProgram])
      			{			  
      	      g_u8_threadIdx[Fx2nProgram]=0;
      			}
          }
		      ThreadNew(Fx2nProgram)=ON;
		    }
		    else
		    {
          
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]]++;
        if(g_u16_TimeoutCnt[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]]>
          (THRES_TIMOUTCNT/UpdateCycle[Fx2nProgram]))
        {
          g_u16_RecvTransLen[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]]=0;
          g_u16_TimeoutCnt[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]]=0;
          if(OFF==MB_NeedWrite(Fx2nProgram))
  		    {
            g_u8_threadIdx[Fx2nProgram]++;
            while(RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[Fx2nProgram]++;
              if(g_u8_ProtocalNum[Fx2nProgram][READ]<=g_u8_threadIdx[Fx2nProgram])
        			{			  
        	      g_u8_threadIdx[Fx2nProgram]=0;
        			}
            }
  		      ThreadNew(Fx2nProgram)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(Fx2nProgram)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[Fx2nProgram][READ]<=g_u8_threadIdx[Fx2nProgram])
	{			  
		g_u8_threadIdx[Fx2nProgram]=0;
	}
}




//======------------------------------------------------------------------------------
void fx2n232Program_read(u8 LeiXing, u16 Adderadd,u8 len )
{
	u8 sum = 0,index=0;
	u8 i = 0;
  memset(Fx2n_SendBuffer,0,FX2NPROGRAM_SEND_LENGTH);
	//帧头命令字
	Fx2n_SendBuffer[index++]  = 0x02; 
	
	if((LeiXing == 'x')||(LeiXing == 'X'))
	{
		Adderadd = (Adderadd/10)+0x80;		
  	Fx2n_SendBuffer[index++]  = 0x30;
  	//地址
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>12)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//偏移量
  	//len = len *2;
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else if((LeiXing == 'y')||(LeiXing == 'Y'))
	{
		Adderadd = (Adderadd/10)+0xA0;	
		Fx2n_SendBuffer[index++]  = 0x30;
  	//地址
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>12)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//偏移量
  	//len = len *2;
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else if((LeiXing == 'm')||(LeiXing == 'M'))
	{
    if(Adderadd<8000)
    {
      Adderadd = 0x100+Adderadd/8;
    }
    else
    {
      Adderadd= 0x1E0 + (Adderadd-8000)/8;
    }
    Fx2n_SendBuffer[index++]  = 0x30;
  	//地址
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>12)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//偏移量
  	//len = len *2;
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else if((LeiXing == 'd')||(LeiXing == 'D'))
	{
		Adderadd = 0x4000 + 2 * Adderadd;
		Fx2n_SendBuffer[index++]  = 0x45;
  	Fx2n_SendBuffer[index++]  = 0x30;
  	//地址
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>16)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>12)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//偏移量
  	len = len *2;
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else if((LeiXing == 't')||(LeiXing == 'T'))
	{
		Adderadd = 0x800+Adderadd*2;	
		Fx2n_SendBuffer[index++]  = 0x45;
  	Fx2n_SendBuffer[index++]  = 0x30;
  	//地址
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>16)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>12)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//偏移量
  	len = len *2;
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	} 
	else if((LeiXing == 'c')||(LeiXing == 'C'))
	{
		Adderadd = 0xA00+Adderadd*2;	
		Fx2n_SendBuffer[index++]  = 0x45;
  	Fx2n_SendBuffer[index++]  = 0x30;
  	//地址
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>16)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>12)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2n_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//偏移量
  	len = len *2;
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2n_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else
	{

	}

	//帧未
	Fx2n_SendBuffer[index++] = 0x03;
	//校验
	for(i=1;i<index;i++)
	{
		sum+=Fx2n_SendBuffer[i];
	}

	Fx2n_SendBuffer[index++]  =  HEXtoASCII((sum>>4)&0x0f);
	Fx2n_SendBuffer[index++]  =  HEXtoASCII((sum)&0x0f);
  Fx2nProgram_Send(Fx2n_SendBuffer,13);
}
//===----------------------------------------------------------------------------
void f_Fx2nProgram_task(void)
{
	u8 i = 0;
	u16 dataaddr;
	//当前空闲 且 没有写操作
	if(ThreadNew(Fx2nProgram) == ON)
	{
		ThreadNew(Fx2nProgram) = OFF;
		GW_ReadStatus(Fx2nProgram)=READ_WAITRESPOND;
    dataaddr=(RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].regaddr[6]<<8)
      + RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].regaddr[7];
    fx2n232Program_read(RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].regtype,dataaddr,
      RegisterCfgBuff[Fx2nProgram][g_u8_threadIdx[Fx2nProgram]].datalenth);
	}	
  else if(WRITE_PRESEND==GW_WriteStatus(Fx2nProgram))
  {		   
    GW_WriteStatus(Fx2nProgram)=WRITE_WAITRESPOND;
    dataaddr=(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
    Fx2nProgram_Write(RegisterCfgBuff[Fx2nProgram][g_u8_RespondID].regtype,
      dataaddr,g_u16_WriteLen);	// 孔数
  }
  else
  {

  }
}
