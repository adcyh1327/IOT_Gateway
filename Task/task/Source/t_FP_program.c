#include "main.h"

#define FPPROGRAM_SEND_LENGTH             220          //发送缓冲区大小
#define FPPROGRAM_RECE_LENGTH             220         //接受缓冲区大小

u8 fp_length=0;
u8 fp_readresult[FPPROGRAM_RECE_LENGTH];
u8 FP_SendBuffer[FPPROGRAM_SEND_LENGTH];
u8 fp_success=0;
u8 fp_read_rcv_flag;
u8 fp_right;
u8 fp_recvOrignBuf[FPPROGRAM_RECE_LENGTH];
u8 fp_w_f;
u8 FP_Program=0xff;;
u8 FP_Program_Enable=OFF;

u8 xxx;
u8 sw[20]={'%',0x30,0x31,'#','R','D','D',0x30,0x30,0x30,0x31,0x30,0x30,0x30,0x30,0x31,0x30,0x35,0x35,0x0D};
u8 test[10];

void FPprogram_Init(u8 nodeID)
{
  FP_Program = nodeID;
  FP_Program_Enable=ON;
}

	
/******************************************************************************************************/
//功能：将解析出来的数据，转换成特定的形式存储在最终数组中
//数据形式包括状态信息，一个字节；正常数据，4个字节。
//目前配置帧中一般设置为：
//（状态寄存器:操作模式，数据长度<字节>）B,1 /（数据寄存器：操作模式，数据长度<字>）w,1
//最终数组中数据长度：单位为：字节
/******************************************************************************************************/
void fpprogram_RecDataType(u8 regtype,u8 *sourcebuf,u8 *targetbuf,u8 datalen)
{
  u8 i;
  if((regtype=='R')||(regtype=='X')||(regtype=='Y'))			  //数据为状态信息
  {
    for(i=0;i<datalen;i++)
    {
      targetbuf[2*i]=ASCII2BYTE('0','0');
      targetbuf[2*i+1]=ASCII2BYTE('0',sourcebuf[i]);
    }
    g_u16_RecvTransLen[FP_Program][g_u8_threadIdx[FP_Program]]= 						  //数据长度：单位是字节
       RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].datalenth*2;
  }
  else if((regtype=='D')||(regtype=='T'))	  //数据为正常数据
  {
				for(i=0;i<(datalen/4);i++)
				{
					targetbuf[2*i]=ASCII2BYTE(sourcebuf[4*i+2],sourcebuf[4*i+3]);
					targetbuf[2*i+1]=ASCII2BYTE(sourcebuf[4*i+0],sourcebuf[4*i+1]);
				}
				g_u16_RecvTransLen[FP_Program][g_u8_threadIdx[FP_Program]]=						  //数据长度：单位是字节
        RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].datalenth * 2;
  }
  else
  {

  }
}

/******************************************************************************************************/
//功能：串口中断处理函数，将数据解析出来后放到特定数组中，供modbus tcp查询
/******************************************************************************************************/
void UART_FPProgram_Recv(u8 data)
{
	static u8 i;
	u8 j;			  

	//data = data & 0x7f;
	if(data=='$')									   //正确的应答帧（写/读的应答帧）
	{
		fp_right=1;
		memset(fp_readresult,0,FPPROGRAM_RECE_LENGTH);
		if(GW_WriteStatus(FP_Program)==WRITE_WAITRESPOND)			//检测写成功后向上发PositiveResponse
			{
				GW_WriteStatus(FP_Program)=WRITE_RECVSUCCESS;
				memcpy(&g_u8_EthRespData[FP_Program][(g_u16_StartAddr[FP_Program][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
				//PositiveResponse();
			}
	}
	else if(data=='!')								   //错误的应答帧
	{
//		if(GW_SendStatus(FP_Program)=STARTSEND)	GW_SendStatus(FP_Program)=SENDIDLE;		//写错误，置写空闲
		NegativeResponse(Err_MBcmd);
	}
	else
	{
		
	}
	if(fp_right==1)			   //确定为正确的应答帧后，记录之后的数据帧
	{												   //只处理读数据时的中断响应，写数据时的中断响应不做处理
		fp_readresult[i]=data;
		i++;
		if(data==0x0D)								   //帧结束标志
		{
			i=0;
			fp_right=0;
			fp_success=1;							   //数据帧记录结束
		}
	}
	if(fp_success==1)								   //数据帧记录结束后，截取中间的数据部分
	{
		if(fp_readresult[2]=='C')							   //该区数据为状态信息，1个字节，ascii表示
		{
			fp_recvOrignBuf[0]=fp_readresult[3];
			fp_length=1;
		}
		if(fp_readresult[2]=='D'||fp_readresult[2]=='K')	  //该区数据为正常数据，4个字节，ascii表示
		{
      fp_length=RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].datalenth*4;
			for(j=0;j<fp_length;j++)
			{
				fp_recvOrignBuf[j]=fp_readresult[3+j];	
			}
		}													  //根据配置的操作模式，进行相应的转换，存储转换后的数据
		fpprogram_RecDataType(RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].regtype,fp_recvOrignBuf,
			&g_u8_EthRespData[FP_Program][g_u16_StartAddr[FP_Program][g_u8_threadIdx[FP_Program]]*2],fp_length);
		GW_ReadStatus(FP_Program)=READ_RECVSUCCESS;				//读成功后向上发数据，检测数据长度为零则发NegativeResponse
		fp_success=0;	
//		readwaite=0;
	}
}
			
/******************************************************************************************************/
//功能：定时发送写或读数据命令帧，并对超时进行处理
//写标志：发送空闲/发送开始/发送完成/发送响应
//读标志：读空闲/读等待/读成功
//tips:接受中断只需要关心发送完成/发送响应/读成功
/******************************************************************************************************/
void TIM_1ms_FPProgram(void)							  				//Timer5的1ms定时中断处理函数
{	 						 		
  if(FP_Program_Enable==ON)
  {
  	g_u16_SwitchTimer[FP_Program]++;
  }
  if((ON==MB_NeedWrite(FP_Program))&&(READ_IDLE==GW_ReadStatus(FP_Program)))
	{ 	
    if(GW_WriteStatus(FP_Program)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[FP_Program]>1000)
	     {
	     		g_u16_SwitchTimer[FP_Program]=0;
					GW_WriteStatus(FP_Program)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(FP_Program)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[FP_Program]>WriteTime))
	     {	
		   		GW_WriteStatus(FP_Program)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[FP_Program]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(FP_Program)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(FP_Program)=WRITE_DELAY;
      g_u16_SwitchTimer[FP_Program]=0;
	  }
	  else if(g_u16_SwitchTimer[FP_Program]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[FP_Program]=0;
      GW_WriteStatus(FP_Program)=WRITE_IDLE;
	    MB_NeedWrite(FP_Program)=OFF;
	    g_u8_threadIdx[FP_Program]++;
		  ThreadNew(FP_Program)=ON;
	  }
	  else{

	  }
	}
	else
	{
		if((g_u16_SwitchTimer[FP_Program]>UpdateCycle[FP_Program])&&
		    (g_u8_ProtocalNum[FP_Program][READ]!=0))
		{	  
		  g_u16_SwitchTimer[FP_Program]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(FP_Program))
		  {
		    GW_ReadStatus(FP_Program)=READ_IDLE;
		    g_u16_TimeoutCnt[FP_Program][g_u8_threadIdx[FP_Program]]=0;
		    if(OFF==MB_NeedWrite(FP_Program))
		    {
          g_u8_threadIdx[FP_Program]++;
          while(RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[FP_Program]++;
            if(g_u8_ProtocalNum[FP_Program][READ]<=g_u8_threadIdx[FP_Program])
      			{			  
      	      g_u8_threadIdx[FP_Program]=0;
      			}
          }
		      ThreadNew(FP_Program)=ON;
		    }
		    else
		    {
          
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[FP_Program][g_u8_threadIdx[FP_Program]]++;
        if(g_u16_TimeoutCnt[FP_Program][g_u8_threadIdx[FP_Program]]>
          (THRES_TIMOUTCNT/UpdateCycle[FP_Program]))
        {
          g_u16_RecvTransLen[FP_Program][g_u8_threadIdx[FP_Program]]=0;
          g_u16_TimeoutCnt[FP_Program][g_u8_threadIdx[FP_Program]]=0;
          if(OFF==MB_NeedWrite(FP_Program))
  		    {
            g_u8_threadIdx[FP_Program]++;
            while(RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[FP_Program]++;
              if(g_u8_ProtocalNum[FP_Program][READ]<=g_u8_threadIdx[FP_Program])
        			{			  
        	      g_u8_threadIdx[FP_Program]=0;
        			}
            }
  		      ThreadNew(FP_Program)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(FP_Program)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[FP_Program][READ]<=g_u8_threadIdx[FP_Program])
	{			  
		g_u8_threadIdx[FP_Program]=0;
	}
}

/******************************************************************************************************/
//功能：生成不同寄存器读数据命令帧，通过串口发送出去（编程口）
/******************************************************************************************************/
void fpprogram_read(u8 registertype,u16 addr,u16 lenth)
{
		u8 i,index=0;
		u8 sum;
    memset(FP_SendBuffer,0,FPPROGRAM_SEND_LENGTH);
  	FP_SendBuffer[index++]='%';	 
  	FP_SendBuffer[index++]=DEC2ASCII(										  	//设备站地址
  	  RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].plcaddrstation/16);
  	FP_SendBuffer[index++]=DEC2ASCII(
      RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].plcaddrstation%16);
  	FP_SendBuffer[index++]='#';
  	FP_SendBuffer[index++]='R';												 	//读
  	switch(registertype)
		{
			case 'D':																	  		//寄存器类型
					FP_SendBuffer[index++]='D';									  
					FP_SendBuffer[index++]='D'; 								  		//数据代码
					FP_SendBuffer[index++]=DEC2ASCII(addr/10000); 
					FP_SendBuffer[index++]=DEC2ASCII((addr % 10000)/1000);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
					FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
					addr = addr + lenth-1;
					FP_SendBuffer[index++]=DEC2ASCII(addr/10000);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 10000)/1000);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
					FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
					sum=0;
					for(i=0;i<index;i++)
					{
						sum=sum^FP_SendBuffer[i];
					}
					FP_SendBuffer[index++]=DEC2ASCII(sum/16);
					FP_SendBuffer[index++]=DEC2ASCII(sum%16);
					FP_SendBuffer[index++]=0x0D;
					break;

			case 'T':
					FP_SendBuffer[index++]='K';
					FP_SendBuffer[index++]=DEC2ASCII(addr/1000);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
					FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
					addr = addr + lenth-1;
					FP_SendBuffer[index++]=DEC2ASCII(addr/1000);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
					FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
					sum=0;
					for(i=0;i<index;i++)
					{
							sum=sum^FP_SendBuffer[i];
					}
					FP_SendBuffer[index++]=DEC2ASCII(sum/16);
					FP_SendBuffer[index++]=DEC2ASCII(sum%16);
					FP_SendBuffer[index++]=0x0D;
					break;
				
				case 'R':
				case 'X':
				case 'Y':
          FP_SendBuffer[index++]='C';
				  FP_SendBuffer[index++]='S';	
					FP_SendBuffer[index++]=registertype;
					FP_SendBuffer[index++]=DEC2ASCII((addr/16)/100);
  				FP_SendBuffer[index++]=DEC2ASCII(((addr/16)%100)/10);
  				FP_SendBuffer[index++]=DEC2ASCII(((addr/16)%100)%10);
  				FP_SendBuffer[index++]=DEC2ASCII(addr%16);			
  				sum=0;
  				for(i=0;i<index;i++)
  				{
  					sum=sum^FP_SendBuffer[i];
  				}
  				FP_SendBuffer[index++]=DEC2ASCII(sum/16);
  				FP_SendBuffer[index++]=DEC2ASCII(sum%16);
  				FP_SendBuffer[index++]=0x0D;
					break;
				default:break;
		}
		DAQ_UartSend(FP_SendBuffer,index,CHN_UART_CFG);
}

/******************************************************************************************************/
//功能：生成不同寄存器写数据命令帧，通过串口发送出去（编程口）
/******************************************************************************************************/
void fpprogram_write(u8 registertype,u16 addr,u16 lenth)
{
		u8 i,index=0;
  	u8 sum;
    memset(FP_SendBuffer,0,FPPROGRAM_SEND_LENGTH);
  	FP_SendBuffer[index++]='%';	 
  	FP_SendBuffer[index++]=DEC2ASCII(RegisterCfgBuff[FP_Program][g_u8_RespondID].plcaddrstation/16);
  	FP_SendBuffer[index++]=DEC2ASCII(RegisterCfgBuff[FP_Program][g_u8_RespondID].plcaddrstation%16);
  	FP_SendBuffer[index++]='#';
  	FP_SendBuffer[index++]='W';			  
  	switch(registertype)
		{
				case 'D':
						FP_SendBuffer[index++]='D';
						FP_SendBuffer[index++]='D';
						FP_SendBuffer[index++]=DEC2ASCII(addr/10000); 
						FP_SendBuffer[index++]=DEC2ASCII((addr % 10000)/1000);
						FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
						FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
						FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
						addr = addr + lenth-1;
  					FP_SendBuffer[index++]=DEC2ASCII(addr/10000);
  					FP_SendBuffer[index++]=DEC2ASCII((addr % 10000)/1000);
  					FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
  					FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
  					FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
						for(i=0;i<lenth;i++)
						{
								FP_SendBuffer[index+4*i]=DEC2ASCII(g_u8_Writedata[2*i+1]  /16);
								FP_SendBuffer[index+4*i+1]=DEC2ASCII(g_u8_Writedata[2*i+1] %16);
								FP_SendBuffer[index+4*i+2]=DEC2ASCII(g_u8_Writedata[2*i]  /16);
								FP_SendBuffer[index+4*i+3]=DEC2ASCII(g_u8_Writedata[2*i] %16);
								
						}
						index += lenth*4;
						sum=0;
						for(i=0;i<index;i++)
						{
								sum=sum^FP_SendBuffer[i];
						}
						FP_SendBuffer[index++]=DEC2ASCII(sum/16);
						FP_SendBuffer[index++]=DEC2ASCII(sum%16);
						FP_SendBuffer[index++]=0x0D;
						break;

				case 'T':
						FP_SendBuffer[index++]='K';
						FP_SendBuffer[index++]=DEC2ASCII(addr/1000);
						FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
						FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
						FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
						addr = addr + lenth-1;
  					FP_SendBuffer[index++]=DEC2ASCII(addr/1000);
  					FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
  					FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
  					FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
						for(i=0;i<lenth;i++)
						{
								FP_SendBuffer[index+4*i+1]=DEC2ASCII(g_u8_Writedata[i]  /16);
								FP_SendBuffer[index+4*i]=DEC2ASCII(g_u8_Writedata[i] %16);
								FP_SendBuffer[index+4*i+3]=DEC2ASCII(g_u8_Writedata[i+1]  /16);
								FP_SendBuffer[index+4*i+2]=DEC2ASCII(g_u8_Writedata[i+1] %16);
						}
						index += lenth*4;
						sum=0;
						for(i=0;i<18;i++)
						{
								sum=sum^FP_SendBuffer[i];
						}
						FP_SendBuffer[index++]=DEC2ASCII(sum/16);
						FP_SendBuffer[index++]=DEC2ASCII(sum%16);
						FP_SendBuffer[index++]=0x0D;
						break;

				case 'R':
				    FP_SendBuffer[index++]='C';
				    FP_SendBuffer[index++]='S';	
						FP_SendBuffer[index++]=registertype;
						FP_SendBuffer[index++]=DEC2ASCII((addr/16)/100);
    				FP_SendBuffer[index++]=DEC2ASCII(((addr/16)%100)/10);
    				FP_SendBuffer[index++]=DEC2ASCII(((addr/16)%100)%10);
    				FP_SendBuffer[index++]=DEC2ASCII(addr%16);			
    				FP_SendBuffer[index++]=DEC2ASCII(g_u8_Writedata[1]&0x0f);	//modbus至少发送一个字
    				sum=0;
    				for(i=0;i<13;i++)
    				{
    					sum=sum^FP_SendBuffer[i];
    				}
    				FP_SendBuffer[index++]=DEC2ASCII(sum/16);
    				FP_SendBuffer[index++]=DEC2ASCII(sum%16);
    				FP_SendBuffer[index++]=0x0D;
						break;
				default:break;
		}
		DAQ_UartSend(FP_SendBuffer,index,CHN_UART_CFG);
}

/******************************************************************************************************/
//主task一直在循环
//功能：发送读/写的请求命令帧
//通过判断标志位，确定是否需要发送。标志位在定时中断中置位。
/******************************************************************************************************/
void f_FPprogram_task(void)
{				
//	u8 datatyppe;
  	u16 dataaddr;
  	if(0!=ThreadNew(FP_Program))									   //板卡读
  	{    	
			ThreadNew(FP_Program)=0;															//清除任务标志															
    	GW_ReadStatus(FP_Program)=READ_WAITRESPOND;
			dataaddr=(RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].regaddr[6]<<8)	//读取数据地址,g_u8_threadIdx[FP_Program],该值表示不同的寄存器，在定时中断中会累加
      		+ RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].regaddr[7];
    	fpprogram_read(RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].regtype,dataaddr,			 //寄存器类型/数据地址
      			RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].datalenth);				 //数据长度
  	}
  	else if(WRITE_PRESEND==GW_WriteStatus(FP_Program))					   //板卡写
  	{		   
			GW_WriteStatus(FP_Program)=WRITE_WAITRESPOND;
			dataaddr=(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
    	fpprogram_write(RegisterCfgBuff[FP_Program][g_u8_RespondID].regtype,dataaddr,g_u16_WriteLen);
  	}
  	else
  	{

  	}
}