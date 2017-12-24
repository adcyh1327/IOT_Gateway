#include "main.h"
//#include "config.h"

#define FX2PROG_SEND_LENGTH             120          //发送缓冲区大小
#define FX2PROG_RECE_LENGTH             120         //接受缓冲区大小

static u8 Fx2_SendBuffer[FX2PROG_SEND_LENGTH];
static u8 Fx2_ReceData[FX2PROG_RECE_LENGTH];          //板卡接受设备返回数据的缓冲区
static u8 Fx2_StateRece;
static u8 Fx2Prog=0xff;
static u8 RecTimer;
static u8 sl_u8FlagGetData;
static u8 Fx2_Enable=OFF;

void Fx2Prog_Init(u8 nodeID)
{
  u8 i;
  for(i=0;i<FX2PROG_RECE_LENGTH;i++)
  {
    Fx2_ReceData[i]=0;
  }
  for(i=0;i<FX2PROG_SEND_LENGTH;i++)
  {
    Fx2_SendBuffer[i]=0;
  }
  Fx2Prog = nodeID;
  RecTimer=0;
  sl_u8FlagGetData   = FALSE;
  Fx2_Enable=ON;
}
  
void Fx2Prog_Send(u8 *sendbuf,u8 len)
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


void Fx2Prog_RecDataHandle(u8 regtype,u8 *sourcebuf,u8 *targetbuf,u8 datalen)
{
  u8 i,temp;
  if((RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].regtype=='X')||
     (RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].regtype=='Y')||
     (RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].regtype=='M'))
  {
    for(i=0;i<(datalen-4)/2;i++)
    {
      temp=(RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].regaddr[6]<<8) +
            RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].regaddr[7]+i;
      if(RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].regtype=='M')
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
        targetbuf[i*2+1]=((ASCIItoHEX(sourcebuf[1]))>>(temp%4))&0x01;
      }
      else
      {
        targetbuf[i*2+1]=(ASCIItoHEX(sourcebuf[2])>>temp)&0x01;
      }
      //targetbuf[2*i+1]=((temp>3)?((ASCIItoHEX(sourcebuf[2])>>(temp%4)):(ASCIItoHEX(sourcebuf[3])>>temp)));
    }
    g_u16_RecvTransLen[Fx2Prog][g_u8_threadIdx[Fx2Prog]]= 
       RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].datalenth*2;
  }
  else if((RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].regtype == 'D') ||
          (RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].regtype == 'T'))
  {
    for(i=0;i<(datalen-4)/4;i++)
    {
      targetbuf[2*i]  =(ASCIItoHEX(sourcebuf[4*i+3])<<4)+ASCIItoHEX(sourcebuf[4*i+4]);
      targetbuf[2*i+1]=(ASCIItoHEX(sourcebuf[4*i+1])<<4)+ASCIItoHEX(sourcebuf[4*i+2]);
    }
    g_u16_RecvTransLen[Fx2Prog][g_u8_threadIdx[Fx2Prog]]=
          RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].datalenth * 2;
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
void UartRs232_SIMF2_Rece(unsigned char  l_u8ReceData)
{//串口接收中断

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
    Fx2_StateRece    = TRUE;
    if(GW_WriteStatus(Fx2Prog)==WRITE_WAITRESPOND)
    {
      GW_WriteStatus(Fx2Prog)=WRITE_RECVSUCCESS;
      memcpy(&g_u8_EthRespData[Fx2Prog][(g_u16_StartAddr[Fx2Prog][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
    }
  }
  else if (MIT_FX_NCK == l_u8ReceData)
  {//PLC错误相应
    Fx2_StateRece    = ERR;
    if(GW_WriteStatus(Fx2Prog)==WRITE_WAITRESPOND)
    {
      GW_WriteStatus(Fx2Prog)=WRITE_RECVSUCCESS;
      NegativeResponse(Err_MBcmd);
    }
  }
  else
  {//其他
    Fx2_StateRece    = FALSE;
  }

  //接受帧头
  if ((MIT_FX_STX == l_u8ReceData) && (FALSE == sl_u8FlagGetData) && (FALSE == sl_u8FlagEndData))
  {//当前没有在接受帧内数据
    sl_u8FlagGetData    = TRUE;//正在接受数据
    sl_u8CountGetData   = NONE;
    memset(Fx2_ReceData,0,FX2PROG_RECE_LENGTH);
    Fx2_ReceData[sl_u8CountGetData] = l_u8ReceData;
		return;
  }

  //接受帧尾    
  if ((MIT_FX_ETX == l_u8ReceData) && (TRUE == sl_u8FlagGetData) && (FALSE == sl_u8FlagEndData))
  {//正在接受数据
    sl_u8FlagGetData    = FALSE;
    sl_u8FlagEndData    = TRUE;
    sl_u8CountCheck     = NONE;
		sl_u8CountGetData++;
    Fx2_ReceData[sl_u8CountGetData] = l_u8ReceData;
		return;
  }

  //接受数据
  if (TRUE == sl_u8FlagGetData)
  {
    sl_u8CountGetData++;
    Fx2_ReceData[sl_u8CountGetData] = l_u8ReceData;
  }

    //接受校验
  if (TRUE == sl_u8FlagEndData)
  {
    sl_u8CountCheck++;
		sl_u8CountGetData++;
    Fx2_ReceData[sl_u8CountGetData] = l_u8ReceData;
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
      l_u8DataCheck = l_u8DataCheck + Fx2_ReceData[i];
    }
    if ((sl_u8DataCheckH == HEXtoASCII((l_u8DataCheck>>4)&0x0F)) && (sl_u8DataCheckL == HEXtoASCII(l_u8DataCheck&0x0F)))
    {
      GW_ReadStatus(Fx2Prog)= READ_RECVSUCCESS;

      Fx2Prog_RecDataHandle(RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].regtype,Fx2_ReceData,
	  	&g_u8_EthRespData[Fx2Prog][g_u16_StartAddr[Fx2Prog][g_u8_threadIdx[Fx2Prog]]*2],sl_u8CountGetData+1);	  
    }
    else
    {
        memset(Fx2_ReceData,0,FX2PROG_RECE_LENGTH);
    }
  }
}

void UART_Fx2Prog_Recv(u8 data)
{
  if((WRITE_WAITRESPOND==GW_WriteStatus(Fx2Prog)) && (data == 0x06) )
	{
		if(GW_WriteStatus(Fx2Prog)==WRITE_WAITRESPOND)
    {
      GW_WriteStatus(Fx2Prog)=WRITE_RECVSUCCESS;
      memcpy(&g_u8_EthRespData[Fx2Prog][(g_u16_StartAddr[Fx2Prog][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
    }
	}
	else
	{
    UartRs232_SIMF2_Rece(data);
	}
}

		 																		
void Fx2Prog_Write(unsigned char l_u8TypeReg, unsigned int  l_u16Offset, unsigned char l_u8Len)
{
    unsigned int  l_u16Adress   = 0;//寄存器地址
    unsigned char l_u8LenByteH  = 0;//ASCII值 字节的高4位
    unsigned char l_u8LenByteL  = 0;//ASCII值 字节的低4位
    unsigned char l_u8CheckH    = 0;//ASCII值 校验和的高4位
    unsigned char l_u8CheckL    = 0;//ASCII值 校验和的低4位
	  u8 i,index=0;
	  u8 j,sendbuf[100];
    l_u8Len = l_u8Len * 2;
    memset(Fx2_SendBuffer,0,FX2PROG_SEND_LENGTH);
    Fx2_SendBuffer[index++]  = 0x02;//帧头
    switch(l_u8TypeReg)
    {
        case 'Y':
            
            l_u16Adress = 0xA0*8+l_u16Offset;
            if((g_u8_Writedata[1])&0x01)
            {
              Fx2_SendBuffer[index++]  = 0x37;
            }
            else
            {
              Fx2_SendBuffer[index++]  = 0x38;
            }
            Fx2_SendBuffer[index++]  = 0x30;
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress)&0x0F);
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>4)&0x0F);
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>8)&0x0F);
            break;
        case 'M':
            l_u16Adress = 0x80*8+l_u16Offset;
            if((g_u8_Writedata[1])&0x01)
            {
              Fx2_SendBuffer[index++]  = 0x37;
            }
            else
            {
              Fx2_SendBuffer[index++]  = 0x38;
            }
            Fx2_SendBuffer[index++]  = 0x30;
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress)&0x0F);
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>4)&0x0F);
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>8)&0x0F);
            break;
        case 'D': 
        case 'T':
        case 'C':
            if(l_u8TypeReg=='D')
            {
              l_u16Adress = 0x1000 + 2*l_u16Offset;
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

            Fx2_SendBuffer[index++]  = 0x31;//命令-写 
			//dizhi 
//             Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>16)&0x0F);//
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>12)&0x0F);//
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>8)&0x0F);//
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>4)&0x0F);//
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress)&0x0F);//
			//changdu
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u8Len>>4)&0x0F);//读取得字节数H
            Fx2_SendBuffer[index++]  = HEXtoASCII(l_u8Len&0x0F);//读取得字节数L
			//shuju
          	for(i=0;i<l_u8Len*2 ;i++)
      			{						 // 34 12  31 32 33 34
      			  Fx2_SendBuffer[index++] = sendbuf[i];
      			}
            
            break;
        default:
            break;
    }
    Fx2_SendBuffer[index++ ]  = 0x03;//帧尾
                  //Calc_Check(g_u8ArrSendData, (11+l_u8LenReadWord*2), &l_u8CheckH, &l_u8CheckL);
		for(i=1;i<index;i++)
		{
			 l_u8CheckH += (Fx2_SendBuffer[i]);
		}
    Fx2_SendBuffer[index++] = HEXtoASCII((l_u8CheckH>>4)&0x0F);
    Fx2_SendBuffer[index++] = HEXtoASCII(l_u8CheckH&0x0F);
    Fx2Prog_Send(Fx2_SendBuffer,index );
}
//======---------------------------------------------------------------------------------
void TIM_1ms_Fx2Prog(void)
{
  if(sl_u8FlagGetData==TRUE)
  {
    RecTimer++;
    if(RecTimer>=50)
    {
      RecTimer=0;
      sl_u8FlagGetData=FALSE;
    }
  }
  else
  {
    RecTimer=0;
  }
  if(Fx2_Enable==ON)
  {
    g_u16_SwitchTimer[Fx2Prog]++;
  }
  if((ON==MB_NeedWrite(Fx2Prog))&&(READ_IDLE==GW_ReadStatus(Fx2Prog)))
	{ 	
    if(GW_WriteStatus(Fx2Prog)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[Fx2Prog]>1000)
	     {
	     		g_u16_SwitchTimer[Fx2Prog]=0;
					GW_WriteStatus(Fx2Prog)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(Fx2Prog)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[Fx2Prog]>WriteTime))
	     {	
		   		GW_WriteStatus(Fx2Prog)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[Fx2Prog]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(Fx2Prog)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(Fx2Prog)=WRITE_DELAY;
      g_u16_SwitchTimer[Fx2Prog]=0;
	  }
	  else if(g_u16_SwitchTimer[Fx2Prog]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[Fx2Prog]=0;
      GW_WriteStatus(Fx2Prog)=WRITE_IDLE;
	    MB_NeedWrite(Fx2Prog)=OFF;
	    g_u8_threadIdx[Fx2Prog]++;
		  ThreadNew(Fx2Prog)=ON;
	  }
	  else{

	  }
	}
	else
	{
		if((g_u16_SwitchTimer[Fx2Prog]>UpdateCycle[Fx2Prog])&&
		    (g_u8_ProtocalNum[Fx2Prog][READ]!=0))
		{	  
		  g_u16_SwitchTimer[Fx2Prog]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(Fx2Prog))
		  {
		    GW_ReadStatus(Fx2Prog)=READ_IDLE;
		    g_u16_TimeoutCnt[Fx2Prog][g_u8_threadIdx[Fx2Prog]]=0;
		    if(OFF==MB_NeedWrite(Fx2Prog))
		    {
          g_u8_threadIdx[Fx2Prog]++;
          while(RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[Fx2Prog]++;
            if(g_u8_ProtocalNum[Fx2Prog][READ]<=g_u8_threadIdx[Fx2Prog])
      			{			  
      	      g_u8_threadIdx[Fx2Prog]=0;
      			}
          }
		      ThreadNew(Fx2Prog)=ON;
		    }
		    else
		    {
          
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[Fx2Prog][g_u8_threadIdx[Fx2Prog]]++;
        if(g_u16_TimeoutCnt[Fx2Prog][g_u8_threadIdx[Fx2Prog]]>
          (THRES_TIMOUTCNT/UpdateCycle[Fx2Prog]))
        {
          g_u16_RecvTransLen[Fx2Prog][g_u8_threadIdx[Fx2Prog]]=0;
          g_u16_TimeoutCnt[Fx2Prog][g_u8_threadIdx[Fx2Prog]]=0;
          if(OFF==MB_NeedWrite(Fx2Prog))
  		    {
            g_u8_threadIdx[Fx2Prog]++;
            while(RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[Fx2Prog]++;
              if(g_u8_ProtocalNum[Fx2Prog][READ]<=g_u8_threadIdx[Fx2Prog])
        			{			  
        	      g_u8_threadIdx[Fx2Prog]=0;
        			}
            }
  		      ThreadNew(Fx2Prog)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(Fx2Prog)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[Fx2Prog][READ]<=g_u8_threadIdx[Fx2Prog])
	{			  
		g_u8_threadIdx[Fx2Prog]=0;
	}
}




//======------------------------------------------------------------------------------
void fx2Prog_read(u8 LeiXing, u16 Adderadd,u8 len )
{
	u8 sum = 0,index=0;
	u8 i = 0;
  memset(Fx2_SendBuffer,0,FX2PROG_SEND_LENGTH);
	//帧头命令字
	Fx2_SendBuffer[index++]  = 0x02; 
	
	if((LeiXing == 'x')||(LeiXing == 'X'))
	{	
    Adderadd=(Adderadd/10)*8+(Adderadd%10)+0x80;
  	Fx2_SendBuffer[index++]  = 0x30;
  	//地址
    Fx2_SendBuffer[index++]  = 0x30;
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//偏移量
  	//len = len *2;
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else if((LeiXing == 'y')||(LeiXing == 'Y'))
	{
		Adderadd=(Adderadd/10)*8+0xa0;	
//     Adderadd = 0xa0*8+Adderadd;
		Fx2_SendBuffer[index++]  = 0x30;
  	//地址
    Fx2_SendBuffer[index++]  = 0x30;
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    
  	//偏移量
  	//len = len *2;
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else if((LeiXing == 'm')||(LeiXing == 'M'))
	{
    Adderadd=(Adderadd/10)*8+(Adderadd%10)+0x100;
    Fx2_SendBuffer[index++]  = 0x30;
  	//地址
    Fx2_SendBuffer[index++]  = 0x30;
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//偏移量
  	//len = len *2;
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else if((LeiXing == 'd')||(LeiXing == 'D'))
	{
		Adderadd = 0x1000 + 2 * Adderadd;
  	Fx2_SendBuffer[index++]  = 0x30;
  	//地址
//   	Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>16)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>12)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//偏移量
  	len = len *2;
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else if((LeiXing == 't')||(LeiXing == 'T'))
	{
		Adderadd = 0x800+Adderadd*2;	
		//Fx2_SendBuffer[index++]  = 0x45;
  	Fx2_SendBuffer[index++]  = 0x30;
  	//地址
  	//Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>16)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>12)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//偏移量
  	len = len *2;
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	} 
	else if((LeiXing == 'c')||(LeiXing == 'C'))
	{
		Adderadd = 0xA00+Adderadd*2;	
		//Fx2_SendBuffer[index++]  = 0x45;
  	Fx2_SendBuffer[index++]  = 0x30;
  	//地址
//   	Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>16)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>12)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//偏移量
  	len = len *2;
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else
	{

	}

	//帧未
	Fx2_SendBuffer[index++] = 0x03;
	//校验
	for(i=1;i<index;i++)
	{
		sum+=Fx2_SendBuffer[i];
	}

	Fx2_SendBuffer[index++]  =  HEXtoASCII((sum>>4)&0x0f);
	Fx2_SendBuffer[index++]  =  HEXtoASCII((sum)&0x0f);
  Fx2Prog_Send(Fx2_SendBuffer,index);
}
//===----------------------------------------------------------------------------
void f_Fx2Prog_task(void)
{
	u8 i = 0;
	u16 dataaddr;
	//当前空闲 且 没有写操作
	if(ThreadNew(Fx2Prog) == ON)
	{
		ThreadNew(Fx2Prog) = OFF;
		GW_ReadStatus(Fx2Prog)=READ_WAITRESPOND;
    dataaddr=(RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].regaddr[6]<<8)
      + RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].regaddr[7];
    fx2Prog_read(RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].regtype,dataaddr,
      RegisterCfgBuff[Fx2Prog][g_u8_threadIdx[Fx2Prog]].datalenth);
	}	
  else if(WRITE_PRESEND==GW_WriteStatus(Fx2Prog))
  {		   
    GW_WriteStatus(Fx2Prog)=WRITE_WAITRESPOND;
    dataaddr=(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
    Fx2Prog_Write(RegisterCfgBuff[Fx2Prog][g_u8_RespondID].regtype,
      dataaddr,g_u16_WriteLen);	// 孔数
  }
  else
  {

  }
}
