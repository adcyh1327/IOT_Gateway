//#ifdef MitsubishiQserial_H

#include "main.h"

#define MITSUBISHIQSERIAL_SEND_LENGTH            220
#define MITSUBISIHQSERIAL_RECE_LENGTH            220

//static u8 MitsubishiQserial_SendBufferw[MITSUBISHIQSERIAL_SEND_LENGTH];
static u8 MitsubishiQserial_SendBuffer[MITSUBISHIQSERIAL_SEND_LENGTH];
static u8 MitsubishiQserialReceData[MITSUBISIHQSERIAL_RECE_LENGTH];          //板卡接受设备返回数据的缓冲区
static u8 headRecvd;
static u8 frameLen;
static u8 frameLenw;
static u8 frameLenWait;
static u8 writeResp;	
static u8 writecenter;
static u8 readResp;
static u8 readcenter;
static u8 readten = 0;
static u8 readnum;
static u8 i;
static u8 readtencenter = 0;
static u8 MitsubishiQserial=0xff;
static u8 MitsubishiQserial_Enable=OFF;
static u16 checksum;



void MitsubishiQserial_Init(u8 nodeID)
{
  u8 i;
  headRecvd=0;
  frameLen=0;
  frameLenw = 0;
  writeResp=0;
  for(i=0;i<MITSUBISHIQSERIAL_SEND_LENGTH;i++)
  {
    MitsubishiQserial_SendBuffer[i]=0;
  }
  for(i=0;i<MITSUBISIHQSERIAL_RECE_LENGTH;i++)
  {
    MitsubishiQserialReceData[i]=0;
  }
  MitsubishiQserial=nodeID;
  MitsubishiQserial_Enable=ON;
}

void MitsubishiQserial_Send(u8 *sendbuf, u8 len)
{
  DAQ_UartSend(sendbuf,len,CHN_UART_CFG);
}

void TIM_1ms_MitsubishiQserial(void)
{
  if(MitsubishiQserial_Enable==ON)
  {
  	g_u16_SwitchTimer[MitsubishiQserial]++; 
  }
  if((ON==MB_NeedWrite(MitsubishiQserial))&&(READ_IDLE==GW_ReadStatus(MitsubishiQserial)))
	{
    g_u16_SwitchTimer[MitsubishiQserial]++; 	
    if(GW_WriteStatus(MitsubishiQserial)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[MitsubishiQserial]>1000)
	     {
	     		g_u16_SwitchTimer[MitsubishiQserial]=0;
					GW_WriteStatus(MitsubishiQserial)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(MitsubishiQserial)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[MitsubishiQserial]>WriteTime))
	     {	
		   		GW_WriteStatus(MitsubishiQserial)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[MitsubishiQserial]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(MitsubishiQserial)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(MitsubishiQserial)=WRITE_DELAY;
      g_u16_SwitchTimer[MitsubishiQserial]=0;
	  }
	  else if(g_u16_SwitchTimer[MitsubishiQserial]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[MitsubishiQserial]=0;
      GW_WriteStatus(MitsubishiQserial)=WRITE_IDLE;
	    MB_NeedWrite(MitsubishiQserial)=OFF;
	    g_u8_threadIdx[MitsubishiQserial]++;
		  ThreadNew(MitsubishiQserial)=ON;
	  }
	  else{

	  }
	}
	else
	{	
		if((g_u16_SwitchTimer[MitsubishiQserial]>UpdateCycle[MitsubishiQserial])&&
		    (g_u8_ProtocalNum[MitsubishiQserial][READ]!=0))
		{	  
		  g_u16_SwitchTimer[MitsubishiQserial]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(MitsubishiQserial))
		  {
		    GW_ReadStatus(MitsubishiQserial)=READ_IDLE;
		    g_u16_TimeoutCnt[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]]=0;
		    if(OFF==MB_NeedWrite(MitsubishiQserial))
		    {
          g_u8_threadIdx[MitsubishiQserial]++;
          while(RegisterCfgBuff[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[MitsubishiQserial]++;
            if(g_u8_ProtocalNum[MitsubishiQserial][READ]<=g_u8_threadIdx[MitsubishiQserial])
      			{			  
      	      g_u8_threadIdx[MitsubishiQserial]=0;
      			}
          }
		      ThreadNew(MitsubishiQserial)=ON;
		    }
		    else
		    {
          
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]]++;
        if(g_u16_TimeoutCnt[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]]>
          (THRES_TIMOUTCNT/UpdateCycle[MitsubishiQserial]))
        {
          g_u16_RecvTransLen[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]]=0;
          g_u16_TimeoutCnt[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]]=0;
          if(OFF==MB_NeedWrite(MitsubishiQserial))
  		    {
            g_u8_threadIdx[MitsubishiQserial]++;
            while(RegisterCfgBuff[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[MitsubishiQserial]++;
              if(g_u8_ProtocalNum[MitsubishiQserial][READ]<=g_u8_threadIdx[MitsubishiQserial])
        			{			  
        	      g_u8_threadIdx[MitsubishiQserial]=0;
        			}
            }
  		      ThreadNew(MitsubishiQserial)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(MitsubishiQserial)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[MitsubishiQserial][READ]<=g_u8_threadIdx[MitsubishiQserial])
	{			  
		g_u8_threadIdx[MitsubishiQserial]=0;
	}
}


//mitsubishiq配置成了 9600,8,1,奇校验
void MitsubishiQserial_read(u8 LeiXing,u16 Adderadd,u16 send_len)
{
	u32 addr = 0;
	u8 sum = 0;
	u8 ret = 0,index = 0,n = 0,i = 0;
	u16 tmp = 0;

	addr = 0x00 + Adderadd;//对应的基地址加上寄存器编号，目前基地址都设置为0；
	memset(MitsubishiQserial_SendBuffer,'0',MITSUBISHIQSERIAL_SEND_LENGTH);
	MitsubishiQserial_SendBuffer[index++] = 0x10;
	MitsubishiQserial_SendBuffer[index++] = 0x02;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0xfc;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x11;
	MitsubishiQserial_SendBuffer[index++] = 0x11;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0xff;
	MitsubishiQserial_SendBuffer[index++] = 0xff;
	MitsubishiQserial_SendBuffer[index++] = 0x03;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x1a;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x1c;
	MitsubishiQserial_SendBuffer[index++] = 0x08;
	MitsubishiQserial_SendBuffer[index++] = 0x0a;
	MitsubishiQserial_SendBuffer[index++] = 0x08;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x04;
	MitsubishiQserial_SendBuffer[index++] = 0x01;
	switch(LeiXing)
	{
		case 'X':
		{
			MitsubishiQserial_SendBuffer[index++] = 0x0d;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x9c;

			//以下是读取线圈时协议帧的相同部分
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			//地址
			MitsubishiQserial_SendBuffer[index++] = addr&0xff;

			if((addr&0xff) == 0x10)
			{
				//附加一个0x10
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}
			MitsubishiQserial_SendBuffer[index++] = (addr>>8)&0xff;
			if(((addr>>8)&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}

			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			//16进制读取
			MitsubishiQserial_SendBuffer[index++] = 0x10;
			//因为前面是0x10,所以在后面附加0x10
			MitsubishiQserial_SendBuffer[index++] = 0x10;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x10;
			MitsubishiQserial_SendBuffer[index++] = 0x03;

			tmp = 44 + n;

			for(i = 2;i <= tmp;i++)
			{
				sum += MitsubishiQserial_SendBuffer[i];
			}
			if(((addr&0xff) == 0x10) || (((addr>>8)&0xff) == 0x10))
			{
				if(n == 1)
				{
					sum = sum - 0x10;
				}
				else if(n ==2)
				{
					sum = sum - 0x10 - 0x10;
				}
			}
			sum = sum&0xff;
			MitsubishiQserial_SendBuffer[index++] = HEXtoASCII((sum>>4)&0x0f);
			MitsubishiQserial_SendBuffer[index++] = HEXtoASCII(sum&0x0f);

			ret = index;
			break;
		}
		case 'Y':
		{
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x9d;
			//以下是读取线圈时协议帧的相同部分
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			//地址
			MitsubishiQserial_SendBuffer[index++] = addr&0xff;


			if((addr&0xff) == 0x10)
			{
				//附加一个0x10
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}
			MitsubishiQserial_SendBuffer[index++] = (addr>>8)&0xff;
			if(((addr>>8)&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}

			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			//16进制读取
			MitsubishiQserial_SendBuffer[index++] = 0x10;
			//因为前面是0x10,所以在后面附加0x10
			MitsubishiQserial_SendBuffer[index++] = 0x10;
			MitsubishiQserial_SendBuffer[index++] = 0x00;

			MitsubishiQserial_SendBuffer[index++] = 0x10;
			MitsubishiQserial_SendBuffer[index++] = 0x03;

			tmp = 44 + n;

			for(i = 2;i <= tmp;i++)
			{
				sum += MitsubishiQserial_SendBuffer[i];
			}
			if(((addr&0xff) == 0x10) || (((addr>>8)&0xff) == 0x10))
			{
				if(n == 1)
				{
					sum = sum - 0x10;
				}
				else if(n ==2){
					sum = sum - 0x10 - 0x10;
				}
			}
			sum = sum&0xff;
			MitsubishiQserial_SendBuffer[index++] = HEXtoASCII((sum>>4)&0x0f);
			MitsubishiQserial_SendBuffer[index++] = HEXtoASCII(sum&0x0f);

			ret = index;
			break;
		}
		case 'M':
		{
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x90;
			//以下是读取线圈时协议帧的相同部分
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			//地址
			MitsubishiQserial_SendBuffer[index++] = addr&0xff;


			if((addr&0xff) == 0x10)
			{
				//附加一个0x10
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}
			MitsubishiQserial_SendBuffer[index++] = (addr>>8)&0xff;
			if(((addr>>8)&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}

			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			//16进制读取
			MitsubishiQserial_SendBuffer[index++] = 0x10;
			//因为前面是0x10,所以在后面附加0x10
			MitsubishiQserial_SendBuffer[index++] = 0x10;
			MitsubishiQserial_SendBuffer[index++] = 0x00;

			MitsubishiQserial_SendBuffer[index++] = 0x10;
			MitsubishiQserial_SendBuffer[index++] = 0x03;

			tmp = 44 + n;
			for(i = 2;i <= tmp;i++)
			{
				sum += MitsubishiQserial_SendBuffer[i];
			}
			if(((addr&0xff) == 0x10) || (((addr>>8)&0xff) == 0x10))
			{
				if(n == 1)
				{
					sum = sum - 0x10;
				}
				else if(n ==2){
					sum = sum - 0x10 - 0x10;
				}
			}

			sum = sum&0xff;
			MitsubishiQserial_SendBuffer[index++] = HEXtoASCII((sum>>4)&0x0f);
			MitsubishiQserial_SendBuffer[index++] = HEXtoASCII(sum&0x0f);

			ret = index;
			break;
		}
		case 'D':
		{
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0xa8;
			//以下是读取线圈时协议帧的相同部分
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			//地址
			MitsubishiQserial_SendBuffer[index++] = addr&0xff;


			if((addr&0xff) == 0x10)
			{
				//附加一个0x10
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}
			MitsubishiQserial_SendBuffer[index++] = (addr>>8)&0xff;
			if(((addr>>8)&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}

			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			//读取长度
			MitsubishiQserial_SendBuffer[index++] = send_len;
			if(send_len == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}

			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x10;
			MitsubishiQserial_SendBuffer[index++] = 0x03;

			tmp = 45 + n;
			for(i = 2;i <= tmp;i++)
			{
				sum += MitsubishiQserial_SendBuffer[i];
			}
			if(((addr&0xff) == 0x10) || (((addr>>8)&0xff) == 0x10) || (send_len == 0x10))
			{
				if(n == 1)
				{
					sum = sum - 0x10;
				}
				else if(n ==2){
					sum = sum - 0x10 - 0x10;
				}
			}
			sum = sum&0xff;
			MitsubishiQserial_SendBuffer[index++] = HEXtoASCII((sum>>4)&0x0f);
			MitsubishiQserial_SendBuffer[index++] = HEXtoASCII(sum&0x0f);

			ret = index;
			break;
		}
		case 'T':
		{
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0xc2;
			//以下是读取线圈时协议帧的相同部分
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			//地址
			MitsubishiQserial_SendBuffer[index++] = addr&0xff;


			if((addr&0xff) == 0x10)
			{
				//附加一个0x10
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}
			MitsubishiQserial_SendBuffer[index++] = (addr>>8)&0xff;
			if(((addr>>8)&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			//读取长度
			MitsubishiQserial_SendBuffer[index++] = send_len;

			if(send_len == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}

			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x10;
			MitsubishiQserial_SendBuffer[index++] = 0x03;

			tmp = 45 + n;
			for(i = 2;i <= tmp;i++)
			{
				sum += MitsubishiQserial_SendBuffer[i];
			}
			if(((addr&0xff) == 0x10) || (((addr>>8)&0xff) == 0x10) || (send_len == 0x10))
			{
				if(n == 1)
				{
					sum = sum - 0x10;
				}
				else if(n ==2){
					sum = sum - 0x10 - 0x10;
				}
			}
			sum = sum&0xff;
			MitsubishiQserial_SendBuffer[index++] = HEXtoASCII((sum>>4)&0x0f);
			MitsubishiQserial_SendBuffer[index++] = HEXtoASCII(sum&0x0f);

			ret = index;
			break;
		}
		default:
		{
		}
	}

	MitsubishiQserial_Send(MitsubishiQserial_SendBuffer, ret);
	
}

void MitsubishiQserial_write(u8 LeiXing,u16 Adderadd,u16 l_u8Len)
{
	u16 tmp = 0;
	u16 j = 0,sum = 0,ret = 0,index = 0;
	u8 n = 0,i = 0,k = 0;

	memset(MitsubishiQserial_SendBuffer,'0',MITSUBISHIQSERIAL_SEND_LENGTH);
	MitsubishiQserial_SendBuffer[index++] = 0x10;
	MitsubishiQserial_SendBuffer[index++] = 0x02;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0xfc;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x11;
	MitsubishiQserial_SendBuffer[index++] = 0x11;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0xff;
	MitsubishiQserial_SendBuffer[index++] = 0xff;
	MitsubishiQserial_SendBuffer[index++] = 0x03;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x22;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x1c;
	MitsubishiQserial_SendBuffer[index++] = 0x08;
	MitsubishiQserial_SendBuffer[index++] = 0x0a;
	MitsubishiQserial_SendBuffer[index++] = 0x08;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x00;
	MitsubishiQserial_SendBuffer[index++] = 0x14;
	MitsubishiQserial_SendBuffer[index++] = 0x02;

	switch(LeiXing)
	{
		case 'X':
		{
			MitsubishiQserial_SendBuffer[index++] = 0x0c;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x01;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x9c;
			MitsubishiQserial_SendBuffer[index++] = 0x00;

			//地址
			MitsubishiQserial_SendBuffer[index++] = Adderadd&0xff;


			if((Adderadd&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}
			MitsubishiQserial_SendBuffer[index++] = (Adderadd>>8)&0xff;
			if(((Adderadd>>8)&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}

			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;



			//写入要写的数
		 	for(i=0;i<l_u8Len;i++)
      {
       MitsubishiQserial_SendBuffer[(index++)+i*2]  = (g_u8_Writedata[1]&0x01);
       MitsubishiQserial_SendBuffer[(index++)+i*2]  = 0x00;
      }
			break;
		}
		case 'Y':
		{
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x01;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x9d;
			MitsubishiQserial_SendBuffer[index++] = 0x00;

			//地址
			MitsubishiQserial_SendBuffer[index++] = Adderadd;


			if((Adderadd&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}
			MitsubishiQserial_SendBuffer[index++] = (Adderadd>>8)&0xff;
			if(((Adderadd>>8)&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}

			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;

			//写入要写的数
		 	for(i=0;i<l_u8Len;i++)
		    {
				 MitsubishiQserial_SendBuffer[(index++)+i*2]  = (g_u8_Writedata[1]&0x01);
				 MitsubishiQserial_SendBuffer[(index++)+i*2]  = 0x00;
		    }
			break;
		}
		case 'M':
		{
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x01;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x90;
			MitsubishiQserial_SendBuffer[index++] = 0x00;

			//地址
			MitsubishiQserial_SendBuffer[index++] = Adderadd;


			if((Adderadd&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}
			MitsubishiQserial_SendBuffer[index++] = (Adderadd>>8)&0xff;
			if(((Adderadd>>8)&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}

			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;

			//写入要写的数
		 	for(i=0;i<l_u8Len;i++)
      {
       MitsubishiQserial_SendBuffer[(index++)+i*2]  = (g_u8_Writedata[1]&0x01);
       MitsubishiQserial_SendBuffer[(index++)+i*2]  = 0x00;
      }
			break;
		}
		case 'D':
		{
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x01;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0xa8;
			MitsubishiQserial_SendBuffer[index++] = 0x00;

			//地址
			MitsubishiQserial_SendBuffer[index++] = Adderadd&0xff;


			if((Adderadd&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}

			MitsubishiQserial_SendBuffer[index++] = (Adderadd>>8)&0xff;
			if(((Adderadd>>8)&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}

			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;

			//写入要写的数
		 	for(i=0;i<l_u8Len;i++)
		    {
				 MitsubishiQserial_SendBuffer[(index++)+i*2]  = g_u8_Writedata[i*2+1]&0xff;

				 if((g_u8_Writedata[i*2+1]&0xff) == 0x10)
				 {
					 MitsubishiQserial_SendBuffer[(index++)+i*2] = 0x10;
					 n++;
				 }
				 MitsubishiQserial_SendBuffer[(index++)+i*2]  = (g_u8_Writedata[i*2+0])&0xff;
				 if(((g_u8_Writedata[i*2+0])&0xff) == 0x10)
				 {
					 MitsubishiQserial_SendBuffer[(index++)+i*2] = 0x10;
					 n++;
				 }
		    }
			break;
		}
		case 'T':
		{
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x01;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0xc2;
			MitsubishiQserial_SendBuffer[index++] = 0x00;

			//地址
			MitsubishiQserial_SendBuffer[index++] = Adderadd&0xff;


			if((Adderadd&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}

			MitsubishiQserial_SendBuffer[index++] = (Adderadd>>8)&0xff;
			if(((Adderadd>>8)&0xff) == 0x10)
			{
				MitsubishiQserial_SendBuffer[index++] = 0x10;
				n++;
			}

			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;
			MitsubishiQserial_SendBuffer[index++] = 0x00;

			//写入要写的数
		 	for(i=0;i<l_u8Len;i++)
		    {
				 MitsubishiQserial_SendBuffer[(index++)+i*2]  = g_u8_Writedata[i*2+1]&0xff;

				 if((g_u8_Writedata[i*2+1]&0xff) == 0x10)
				 {
					 MitsubishiQserial_SendBuffer[(index++)+i*2] = 0x10;
					 n++;
				 }
				 MitsubishiQserial_SendBuffer[(index++)+i*2]  = (g_u8_Writedata[i*2+0])&0xff;
				 if(((g_u8_Writedata[i*2+0])&0xff) == 0x10)
				 {
					 MitsubishiQserial_SendBuffer[(index++)+i*2] = 0x10;
					 n++;
				 }
		    }
			break;
		}
	}

	ret = 52 + l_u8Len*2 + n;

	j = ret;

	MitsubishiQserial_SendBuffer[j] = 0x10;
	MitsubishiQserial_SendBuffer[j+1] = 0x03;

	for(k = 2;k < ret;k++)
	{
		sum += MitsubishiQserial_SendBuffer[k];
	}
	if(((Adderadd&0xff) == 0x10) || (((Adderadd>>8)&0xff) == 0x10))
	{
		sum = sum - 0x10;
	}
  sum = sum - n*0x10;
	sum = (sum&0xff);
	MitsubishiQserial_SendBuffer[j+2] = HEXtoASCII((sum>>4)&0x0f);
	MitsubishiQserial_SendBuffer[j+3] = HEXtoASCII(sum&0x0f);

	MitsubishiQserial_Send(MitsubishiQserial_SendBuffer, j+4);

}

void MitsubishiQserial_RecDataHandle(u8 regtype,u8 *sourcebuf,u8 *targetbuf,u8 datalen)
{
	u8 i = 0 ;
	u8 tmp = 0 ;
	if((regtype=='X')||(regtype=='Y')||(regtype=='M'))
	{
		for(i = 0;i < datalen;i++)
		{
			tmp = sourcebuf[i+40]&0x01;
			targetbuf[0] = tmp;
		}
		g_u16_RecvTransLen[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]]= 
       RegisterCfgBuff[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]].datalenth*2;
	}

	else if((regtype=='D')||(regtype=='T'))
	{
		for(i = 0;i < datalen*2;i=i+2)
		{
			targetbuf[i] = sourcebuf[41+i];
			targetbuf[i+1] = sourcebuf[40+i];
      if(sourcebuf[40+i] == 0x10)
      {
          targetbuf[i] = sourcebuf[42+i];
      }
		}
		g_u16_RecvTransLen[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]]=
       RegisterCfgBuff[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]].datalenth * 2;
	}
}


void UART_MitsubishiQserial_Recv(u8 data)
{
  u8 i;			  
  static u8 chksum;
  if(data == 0x10)
  {   
    writeResp = 1;
    readResp = 1;
  }
	if((writeResp == 1)&&(GW_WriteStatus(MitsubishiQserial)==WRITE_WAITRESPOND))
	{
    if((0==frameLenw)&&(data== 0x10))
    {
       writecenter = 1;
    }
    else if((writecenter == 1) && (1 == frameLenw))
    {
      if((1==frameLenw)&&(data== 0x02))
      {
         writecenter = 0;
      }
      else
      {
         frameLenw = 0;
         writeResp = 0;
         writecenter = 0;
         NegativeResponse(Err_MBcmd);
      }
    }    
    
    frameLenw++;
    
		if(GW_WriteStatus(MitsubishiQserial)==WRITE_WAITRESPOND)
		{
				GW_WriteStatus(MitsubishiQserial)=WRITE_RECVSUCCESS;
				memcpy(&g_u8_EthRespData[MitsubishiQserial][(g_u16_StartAddr[MitsubishiQserial][g_u8_RespondID]+g_u8_WriteAddrOffset)*2],
								&g_u8_Writedata,g_u16_WriteLen*2);  
        writeResp = 0;
        frameLenw = 0;
		}	
		else
		{
      writeResp = 0;
      frameLenw = 0;
			GW_WriteStatus(MitsubishiQserial)=WRITE_RECVSUCCESS;
			NegativeResponse(Err_MBcmd);
		}
     
	}
  else if((readResp == 1)&&(GW_ReadStatus(MitsubishiQserial)==READ_WAITRESPOND))
  {
    MitsubishiQserialReceData[frameLen]=data;
    
    if((data == 0x10) && (frameLen >= 40))
    {
      readten++;
    }
    if((0==frameLen)&&(data== 0x10))
    {
       readcenter = 1;
    }
    else if((readcenter == 1) && (1 == frameLen))
    {
      if((1==frameLen)&&(data== 0x02))
      {
         readcenter = 0;
      }
      else
      {
         frameLen = 0;
         readResp = 0;
         readcenter = 0;
         NegativeResponse(Err_MBcmd);
      }
    }
      
    frameLen++;

    if(frameLen >= RegisterCfgBuff[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]].datalenth*2+44 + readten / 2)
    {
      MitsubishiQserial_RecDataHandle(RegisterCfgBuff[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]].regtype,MitsubishiQserialReceData,
      &g_u8_EthRespData[MitsubishiQserial][g_u16_StartAddr[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]]*2],RegisterCfgBuff[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]].datalenth);
      GW_ReadStatus(MitsubishiQserial) = READ_RECVSUCCESS;
      frameLen = 0;
      readResp = 0;
      readten = 0;
      readcenter = 0;
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
void f_MitsubishiQserial_task(void)
{				
  u8  datatyppe;
  u16 dataaddr;
  if(0 != ThreadNew(MitsubishiQserial))
  {
    ThreadNew(MitsubishiQserial)=0;
    GW_ReadStatus(MitsubishiQserial)=READ_WAITRESPOND;
    dataaddr=(RegisterCfgBuff[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]].regaddr[6]<<8)
      + RegisterCfgBuff[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]].regaddr[7];
    MitsubishiQserial_read(RegisterCfgBuff[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]].regtype,dataaddr,
      RegisterCfgBuff[MitsubishiQserial][g_u8_threadIdx[MitsubishiQserial]].datalenth);
  }
  else if(WRITE_PRESEND==GW_WriteStatus(MitsubishiQserial))
  {		   
    GW_WriteStatus(MitsubishiQserial)=WRITE_WAITRESPOND;
    dataaddr=(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
    MitsubishiQserial_write(RegisterCfgBuff[MitsubishiQserial][g_u8_RespondID].regtype,dataaddr,g_u16_WriteLen);
  }
  else
  {

  }
}

//#endif
