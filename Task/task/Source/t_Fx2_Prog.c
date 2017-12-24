#include "main.h"
//#include "config.h"

#define FX2PROG_SEND_LENGTH             120          //���ͻ�������С
#define FX2PROG_RECE_LENGTH             120         //���ܻ�������С

static u8 Fx2_SendBuffer[FX2PROG_SEND_LENGTH];
static u8 Fx2_ReceData[FX2PROG_RECE_LENGTH];          //�忨�����豸�������ݵĻ�����
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
//��������uart5_rs232_rece
//���ܣ�  �������ݽ��ܼ�����
//������  �������ݼĴ�������
//ʹ�ã�  �������봮�ڽ����ж��У�������ܵ��ֽ����ݡ�
//        ������ɱ�־g_u8FlagRece,����(���Ĵ���)����g_u8LengthArrEditData,����g_u8ArrEditData[]
void UartRs232_SIMF2_Rece(unsigned char  l_u8ReceData)
{//���ڽ����ж�

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

  //PLC״̬��Ӧ
  if (MIT_FX_ACK == l_u8ReceData)
  {//PLC��ȷ��Ӧ
    Fx2_StateRece    = TRUE;
    if(GW_WriteStatus(Fx2Prog)==WRITE_WAITRESPOND)
    {
      GW_WriteStatus(Fx2Prog)=WRITE_RECVSUCCESS;
      memcpy(&g_u8_EthRespData[Fx2Prog][(g_u16_StartAddr[Fx2Prog][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
    }
  }
  else if (MIT_FX_NCK == l_u8ReceData)
  {//PLC������Ӧ
    Fx2_StateRece    = ERR;
    if(GW_WriteStatus(Fx2Prog)==WRITE_WAITRESPOND)
    {
      GW_WriteStatus(Fx2Prog)=WRITE_RECVSUCCESS;
      NegativeResponse(Err_MBcmd);
    }
  }
  else
  {//����
    Fx2_StateRece    = FALSE;
  }

  //����֡ͷ
  if ((MIT_FX_STX == l_u8ReceData) && (FALSE == sl_u8FlagGetData) && (FALSE == sl_u8FlagEndData))
  {//��ǰû���ڽ���֡������
    sl_u8FlagGetData    = TRUE;//���ڽ�������
    sl_u8CountGetData   = NONE;
    memset(Fx2_ReceData,0,FX2PROG_RECE_LENGTH);
    Fx2_ReceData[sl_u8CountGetData] = l_u8ReceData;
		return;
  }

  //����֡β    
  if ((MIT_FX_ETX == l_u8ReceData) && (TRUE == sl_u8FlagGetData) && (FALSE == sl_u8FlagEndData))
  {//���ڽ�������
    sl_u8FlagGetData    = FALSE;
    sl_u8FlagEndData    = TRUE;
    sl_u8CountCheck     = NONE;
		sl_u8CountGetData++;
    Fx2_ReceData[sl_u8CountGetData] = l_u8ReceData;
		return;
  }

  //��������
  if (TRUE == sl_u8FlagGetData)
  {
    sl_u8CountGetData++;
    Fx2_ReceData[sl_u8CountGetData] = l_u8ReceData;
  }

    //����У��
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

  //У������
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
    unsigned int  l_u16Adress   = 0;//�Ĵ�����ַ
    unsigned char l_u8LenByteH  = 0;//ASCIIֵ �ֽڵĸ�4λ
    unsigned char l_u8LenByteL  = 0;//ASCIIֵ �ֽڵĵ�4λ
    unsigned char l_u8CheckH    = 0;//ASCIIֵ У��͵ĸ�4λ
    unsigned char l_u8CheckL    = 0;//ASCIIֵ У��͵ĵ�4λ
	  u8 i,index=0;
	  u8 j,sendbuf[100];
    l_u8Len = l_u8Len * 2;
    memset(Fx2_SendBuffer,0,FX2PROG_SEND_LENGTH);
    Fx2_SendBuffer[index++]  = 0x02;//֡ͷ
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

            Fx2_SendBuffer[index++]  = 0x31;//����-д 
			//dizhi 
//             Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>16)&0x0F);//
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>12)&0x0F);//
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>8)&0x0F);//
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress>>4)&0x0F);//
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u16Adress)&0x0F);//
			//changdu
            Fx2_SendBuffer[index++]  = HEXtoASCII((l_u8Len>>4)&0x0F);//��ȡ���ֽ���H
            Fx2_SendBuffer[index++]  = HEXtoASCII(l_u8Len&0x0F);//��ȡ���ֽ���L
			//shuju
          	for(i=0;i<l_u8Len*2 ;i++)
      			{						 // 34 12  31 32 33 34
      			  Fx2_SendBuffer[index++] = sendbuf[i];
      			}
            
            break;
        default:
            break;
    }
    Fx2_SendBuffer[index++ ]  = 0x03;//֡β
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
	//֡ͷ������
	Fx2_SendBuffer[index++]  = 0x02; 
	
	if((LeiXing == 'x')||(LeiXing == 'X'))
	{	
    Adderadd=(Adderadd/10)*8+(Adderadd%10)+0x80;
  	Fx2_SendBuffer[index++]  = 0x30;
  	//��ַ
    Fx2_SendBuffer[index++]  = 0x30;
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//ƫ����
  	//len = len *2;
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else if((LeiXing == 'y')||(LeiXing == 'Y'))
	{
		Adderadd=(Adderadd/10)*8+0xa0;	
//     Adderadd = 0xa0*8+Adderadd;
		Fx2_SendBuffer[index++]  = 0x30;
  	//��ַ
    Fx2_SendBuffer[index++]  = 0x30;
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    
  	//ƫ����
  	//len = len *2;
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else if((LeiXing == 'm')||(LeiXing == 'M'))
	{
    Adderadd=(Adderadd/10)*8+(Adderadd%10)+0x100;
    Fx2_SendBuffer[index++]  = 0x30;
  	//��ַ
    Fx2_SendBuffer[index++]  = 0x30;
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//ƫ����
  	//len = len *2;
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else if((LeiXing == 'd')||(LeiXing == 'D'))
	{
		Adderadd = 0x1000 + 2 * Adderadd;
  	Fx2_SendBuffer[index++]  = 0x30;
  	//��ַ
//   	Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>16)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>12)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//ƫ����
  	len = len *2;
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else if((LeiXing == 't')||(LeiXing == 'T'))
	{
		Adderadd = 0x800+Adderadd*2;	
		//Fx2_SendBuffer[index++]  = 0x45;
  	Fx2_SendBuffer[index++]  = 0x30;
  	//��ַ
  	//Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>16)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>12)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//ƫ����
  	len = len *2;
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	} 
	else if((LeiXing == 'c')||(LeiXing == 'C'))
	{
		Adderadd = 0xA00+Adderadd*2;	
		//Fx2_SendBuffer[index++]  = 0x45;
  	Fx2_SendBuffer[index++]  = 0x30;
  	//��ַ
//   	Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>16)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>12)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>8)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd>>4)&0x0F);
    Fx2_SendBuffer[index++]  = HEXtoASCII((Adderadd)&0x0F);
  	//ƫ����
  	len = len *2;
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len>>4)&0x0f);
  	Fx2_SendBuffer[index++]  = HEXtoASCII((len)&0x0f);
	}
	else
	{

	}

	//֡δ
	Fx2_SendBuffer[index++] = 0x03;
	//У��
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
	//��ǰ���� �� û��д����
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
      dataaddr,g_u16_WriteLen);	// ����
  }
  else
  {

  }
}
