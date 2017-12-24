#include "main.h"
  u8 S7300MPI_SendBuf[50]; //发送缓冲区
  u8 S7300MPI_transRecvAlready=0;//接收数据的数量
  u8 S7300MPI_readMech=0;
  u8 S7300MPI_RxBuf[200];
//MPI TEST
  u16 S7300MPI_MPIDelayTimer;
  u8  S7300MPI_MPIStr7_2[50];
  u8  S7300MPI_MPISn;
  u16 S7300MPI_RcnTimer1S;//1s重连
  u8  S7300MPI_staAddr;
  u16 S7300MPI_waitLen;
  u16 S7300MPI_RegAddr;
  u8  S7300MPI_ConnectMechine=0;
  u8  S7300MPI_chksum=0;
  u8  S7300MPI_FrameStart=0;
  u8  S7300MPI_head00RecvFlg=0;
  u8  S7300MPI_head0CRecvFlg=0;
  u8  S7300MPI_Tail10RecvFlg=0;
  u8  S7300MPI_Tail03RecvFlg=0;
  u8  S7300MPI_FrameRecvFlg=0;	 
  u8  S7300MPI_FrameDoneFlg=0;
  u8  S7300MPI_NewReqFlg=0;
  //Rx
  const u8 S7300MPI_ConResLen[17]={0,0,1,11,0,1,22,0,1,8,0,1,9,0,28,0,0};//有效数据从下标1开始
  //Tx
  u8 S7300ReadRes=0;
  u8 S7300SendLen;
  //MPI
  u8 S7300MPIHeader[50]={0x00,0x0C,0x03,0x14,0xF1,0x01,0x32,0x01,0x00,0x00,0x33,0x01,0x00,0x0E,0x00,0x00,0x04,0x01,0x12,0x0A,0x10,0x10,0x02,0x00,0x01,0x00,0x00,0x81,0x00,0x00,0x00,0x10,0x03,0x68,0xFF,0xFF};
                        //                    5                                                                   22      24  25  26  27  28  29  30
   u8 S7300MPI_ConReqLen[17]={0,1,26,1,2,23,1,2,9,1,2,27,1,1,1,2,10};//有效数据从下标1开始
   u8 S7300MPI_ReqStr1[40]  ={0x02};
   u8 S7300MPI_ReqStr2[40]  ={0x01,0x03,0x02,0x27,0x00,0x64,0x00,0x25,0x00,0x3C,0x00,0x16,0x00,0x00,0x0A,0x01,0x00,0x1F,0x01,0x01,0x01,0x03,0x80,0x10,0x03,0xC9};
   u8 S7300MPI_ReqStr3[40]  ={0x10};
   u8 S7300MPI_ReqStr4[40]  ={0x10,0x02};
   u8 S7300MPI_ReqStr5[40]  ={0x00,0x0D,0x00,0x14,0xE0,0x04,0x00,0x80,0x00,0x02,0x01,0x06,0x02,0x00,0x00,0x01,0x02,0x02,0x02,0x02,0x10,0x03,0x68};
   u8 S7300MPI_ReqStr6[40]  ={0x10};
   u8 S7300MPI_ReqStr7[40]  ={0x10,0x02};
   u8 S7300MPI_ReqStr8[40]  ={0x00,0x0C,0x03,0x14,0x05,0x01,0x10,0x03,0x0C};
   u8 S7300MPI_ReqStr9[40]  ={0x10};
   u8 S7300MPI_ReqStr10[40] ={0x10,0x02};
   u8 S7300MPI_ReqStr11[40] ={0x00,0x0C,0x03,0x14,0xF1,0x00,0x32,0x01,0x00,0x00,0x12,0x01,0x00,0x08,0x00,0x00,0xF0,0x00,0x00,0x01,0x00,0x02,0x01,0xE0,0x10,0x03,0xC3};
   u8 S7300MPI_ReqStr12[40] ={0x10};
   u8 S7300MPI_ReqStr13[40] ={0x10};
   u8 S7300MPI_ReqStr14[40] ={0x10};
   u8 S7300MPI_ReqStr15[40] ={0x10,0x02};
   u8 S7300MPI_ReqStr16[40] ={0x00,0x0C,0x03,0x14,0xB0,0x01,0x00,0x10,0x03,0xB9};
   u8 S7300MPI_DataEnd[40] ={0x00,0x0C,0x03,0x14,0xB0,0x01,0x00,0x10,0x03,0xB9,0x33,0x01,0x00,0x0E,0x00,0x00,0x04,0x01,0x12,0x0A,0x10,0x10,0x02,0x00,0x01,0x00,0x00,0x81,0x00,0x00,0x00,0x10,0x03,0x68,0xFF,0xFF};
 #define  S7300_INTERVAL    10

u8 S7300_mpi;
u8 S7300_mpi_Enable=OFF;

u16 timer,delta_timer;

void S7300_mpi_Init(u8 nodeID)
{
  S7300_mpi=nodeID;
  S7300_mpi_Enable=ON;
}

//求异或和
u8 f_S7300MPI_calc_sum(u8 sendArray[], u8 startIdx,u8 endIdx) 
{
  u8 i; 
  u8 sum;
  sum=0;
  for(i=startIdx;i<endIdx;i++)
  {
    sum=sum^sendArray[i];
  }
  return sum;
}

void  f_S7300MPI_com_save(u8 sendArray[], u8 bufIdx)
{
  u8 i;
  for(i=0;i<bufIdx;i++)
  {
    S7300MPI_MPIStr7_2[i]=sendArray[i];
  }
}

void f_S7300MPI_com_send(u8 sendArray[],u8 bufIdx)
{
  DAQ_UartSend(sendArray,bufIdx,CHN_UART_CFG);
}

void f_S7300MPI_setread_V_req(u8 regtype, u16 blocknum, u16 regAddr, u32 len)
{		   
  u8  i;     
  u8  j;
  u8  localArr[50];
  u16 regNum;
  if(S7300MPI_ConnectMechine!=17)
  {
    return;
  }
  S7300ReadRes=1;
  if(1==S7300MPI_NewReqFlg)
  {
    S7300MPI_ConnectMechine=0;	  
    S7300MPI_NewReqFlg=0;
	  return;
  }
  S7300MPI_NewReqFlg=1;
  
  if(RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].operatesize==0x10)
  {
    len=len*2;
  }
  
  S7300MPI_staAddr=RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].plcaddrstation;//从站地址
  S7300MPI_waitLen=27+len;
  regNum=regAddr*8;
  if(255==S7300MPI_MPISn)
  {
    S7300MPI_MPISn=1;
  }
  else
  {
    S7300MPI_MPISn=S7300MPI_MPISn+1; 
  } 
  S7300SendLen=34;
  //读取V区
  if((regtype=='v')||(regtype=='V'))
  {
    S7300MPI_readMech=0;//通信空闲
    S7300MPIHeader[ 5] =S7300MPI_MPISn;
    S7300MPIHeader[22] =0x02;//数据类型
    S7300MPIHeader[23] =0x00;//数据类型
    S7300MPIHeader[24] =len;//读取数量
    S7300MPIHeader[25] =blocknum/256;//块号
    S7300MPIHeader[26] =blocknum%256;//块号
    S7300MPIHeader[27] =0x84;//存储器类型
    S7300MPIHeader[28] = regNum / 65536;//寄存器地址
    S7300MPIHeader[29] =(regNum % 65536)/ 256;//寄存器地址
    S7300MPIHeader[30] =(regNum % 256);//寄存器地址
    S7300MPIHeader[31] =0x10;//寄存器地址
    S7300MPIHeader[32] =0x03;//寄存器地址	
		for(i=0,j=0;i<33;i++)
    {
      localArr[j++]=S7300MPIHeader[i];
      if((0x10==S7300MPIHeader[i])&&(i!=20)&&(i!=21)&&(i!=31))
		  {
		    localArr[j++]=S7300MPIHeader[i];
				S7300SendLen++;
		  }
    }
    localArr[j] =f_S7300MPI_calc_sum(localArr,0,j);
  }
  f_S7300MPI_com_save(localArr,S7300SendLen);
  S7300MPI_transRecvAlready=0;
  S7300MPI_readMech=1;//等待PLC确认
}

//请求读取数据
void f_S7300MPI_setread_decrete_req(u8 regtype, u16 regAddr, u32 len)
{
  u8 i;	   
  u8 j;
  u8 localArr[50];		 
  if(S7300MPI_ConnectMechine!=17)
  {
    return;
  }	
  S7300ReadRes=1;
  if(1==S7300MPI_NewReqFlg)
  {
    S7300MPI_ConnectMechine=0;	  
    S7300MPI_NewReqFlg=0;
		return;
  }
  S7300MPI_NewReqFlg=1;

  S7300MPI_staAddr=RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].plcaddrstation;//从站地址
  S7300MPI_waitLen=27+len;
  S7300MPI_RegAddr=(regAddr>>8);
  S7300MPI_RegAddr=S7300MPI_RegAddr*8;
  S7300MPI_chksum=0;
  S7300MPI_FrameStart=0;
  S7300MPI_readMech=0;//通信空闲
  S7300MPI_MPIDelayTimer=0;
  //读取E点
  if(255==S7300MPI_MPISn)
  {
    S7300MPI_MPISn=1;
  }
  else
  {
    S7300MPI_MPISn=S7300MPI_MPISn+1; 
  } 
  S7300SendLen=34;
  S7300MPIHeader[5] =S7300MPI_MPISn;
  if((regtype=='I')||(regtype=='i'))
  {                 
    S7300MPI_readMech=0;//通信空闲
    S7300MPIHeader[22] =0x02;//数据类型
    S7300MPIHeader[23] =0x00;//数据类型
    S7300MPIHeader[24] =len;//读取数量
    S7300MPIHeader[25] =0x00;//存储器类型
    S7300MPIHeader[26] =0x00;//存储器类型
    S7300MPIHeader[27] =0x81;//存储器类型
    S7300MPIHeader[28] = S7300MPI_RegAddr / 65536;//寄存器地址
    S7300MPIHeader[29] =(S7300MPI_RegAddr % 65536)/ 256;//寄存器地址
    S7300MPIHeader[30] =(S7300MPI_RegAddr % 256);//寄存器地址
    S7300MPIHeader[31] =0x10;//寄存器地址
    S7300MPIHeader[32] =0x03;//寄存器地址		   
	for(i=0,j=0;i<33;i++)
    {
      localArr[j++]=S7300MPIHeader[i];
      if((0x10==S7300MPIHeader[i])&&(i!=20)&&(i!=21)&&(i!=31))
	  {
	    localArr[j++]=S7300MPIHeader[i];
		S7300SendLen++;		  		
	  }
    }
    localArr[j] =f_S7300MPI_calc_sum(localArr,0,S7300SendLen-1);
  //读取A点
  } 
  else if((regtype=='Q')||(regtype=='q'))
  {
    S7300MPI_readMech=0;//通信空闲
    S7300MPIHeader[22] =0x02;//数据类型
    S7300MPIHeader[23] =0x00;//数据类型
    S7300MPIHeader[24] =len;//读取数量
    S7300MPIHeader[25] =0x00;//存储器类型
    S7300MPIHeader[26] =0x00;//存储器类型
    S7300MPIHeader[27] =0x82;//存储器类型
    S7300MPIHeader[28] = S7300MPI_RegAddr / 65536;//寄存器地址
    S7300MPIHeader[29] =(S7300MPI_RegAddr % 65536)/ 256;//寄存器地址
    S7300MPIHeader[30] =(S7300MPI_RegAddr % 256);//寄存器地址
    S7300MPIHeader[31] =0x10;//寄存器地址
    S7300MPIHeader[32] =0x03;//寄存器地址  
    for(i=0,j=0;i<33;i++)
    {
      localArr[j++]=S7300MPIHeader[i];
      if((0x10==S7300MPIHeader[i])&&(i!=20)&&(i!=21)&&(i!=31))
      {
        localArr[j++]=S7300MPIHeader[i];
        S7300SendLen++;
      }
    }
    localArr[j] =f_S7300MPI_calc_sum(localArr,0,S7300SendLen-1);
  //读取Q点
  } 
  else  if((regtype=='m')||(regtype=='M'))
  {
    S7300MPI_readMech=0;//通信空闲
    S7300MPIHeader[22] =0x02;//数据类型
    S7300MPIHeader[23] =0x00;//数据类型
    S7300MPIHeader[24] =len;//读取数量
    S7300MPIHeader[25] =0x00;//存储器类型
    S7300MPIHeader[26] =0x00;//存储器类型
    S7300MPIHeader[27] =0x83;//存储器类型
    S7300MPIHeader[28] = S7300MPI_RegAddr / 65536;//寄存器地址
    S7300MPIHeader[29] =(S7300MPI_RegAddr % 65536)/ 256;//寄存器地址
    S7300MPIHeader[30] =(S7300MPI_RegAddr % 256);//寄存器地址
    S7300MPIHeader[31] =0x10;//寄存器地址
    S7300MPIHeader[32] =0x03;//寄存器地址  
	  for(i=0,j=0;i<33;i++)
    {
      localArr[j++]=S7300MPIHeader[i];
      if((0x10==S7300MPIHeader[i])&&(i!=20)&&(i!=21)&&(i!=31))
	  {
	    localArr[j++]=S7300MPIHeader[i];
		S7300SendLen++;
	  }
    }
    localArr[j] =f_S7300MPI_calc_sum(localArr,0,S7300SendLen-1);
  //读取SM点
  } 
  else if((regtype=='t')||(regtype=='T'))
  {
    S7300MPI_readMech=0;//通信空闲
    S7300MPIHeader[22] =0x1D;//数据类型
    S7300MPIHeader[23] =0x00;//数据类型
    S7300MPIHeader[24] =len;//读取数量
    S7300MPIHeader[25] =0x00;//存储器类型
    S7300MPIHeader[26] =0x00;//存储器类型
    S7300MPIHeader[27] =0x1d;//存储器类型
    S7300MPI_RegAddr=S7300MPI_RegAddr / 8;
    S7300MPIHeader[28] = S7300MPI_RegAddr / 65536;//寄存器地址
    S7300MPIHeader[29] =(S7300MPI_RegAddr % 65536)/ 256;//寄存器地址
    S7300MPIHeader[30] =(S7300MPI_RegAddr % 256);//寄存器地址
    S7300MPIHeader[31] =0x10;//寄存器地址
    S7300MPIHeader[32] =0x03;//寄存器地址  
	  for(i=0,j=0;i<33;i++)
    {
      localArr[j++]=S7300MPIHeader[i];
      if((0x10==S7300MPIHeader[i])&&(i!=20)&&(i!=21)&&(i!=31))
	    {
	      localArr[j++]=S7300MPIHeader[i];
		    S7300SendLen++;
	    }
    }
    localArr[j] =f_S7300MPI_calc_sum(localArr,0,S7300SendLen-1);
  };
  f_S7300MPI_com_save(localArr,S7300SendLen);
  S7300MPI_transRecvAlready=0;
  S7300MPI_readMech=1;//等待PLC确认
}

void S7300_MPI_read(u8 regtype, u16 blocknum, u16 regAddr, u32 len)
{	
  if((regtype=='v')||(regtype=='V'))
  {
    f_S7300MPI_setread_V_req(regtype,blocknum,regAddr,len);
  }
  else
  {
    f_S7300MPI_setread_decrete_req(regtype,regAddr,len);
  }
}

void MPI_TimerHandler(void)
{
  u8 sendTemp[20];
  u8 i;
  u8 j;
  u8 localArr[50];
  //重连
  
  if(S7300MPI_RcnTimer1S<2000)
  {
    S7300MPI_RcnTimer1S++;
    if(S7300MPI_RcnTimer1S>1999)
    {
      S7300MPI_RcnTimer1S=0;
      if(S7300MPI_ConnectMechine<17)
      {
        S7300MPI_ConnectMechine=1;	  		
        S7300MPI_MPISn=255;
				sendTemp[0]=0x02;
				f_S7300MPI_com_send(sendTemp,1);
      }
    }
  }

  //定时管理
  if(S7300MPI_ConnectMechine==17)
  {
    //S7300MPI_RcnTimer1S=0;
    if(1==S7300MPI_readMech)
    {
      if(1==S7300ReadRes)
      {
		  	S7300ReadRes=0;
        S7300MPI_readMech=S7300MPI_readMech+1; 
	      sendTemp[0]=0x02;
	      f_S7300MPI_com_send(sendTemp,1);
      }
    }
		else if(2==S7300MPI_readMech)
    {
      if(1==S7300ReadRes)
      {
	      S7300ReadRes=0;
        S7300MPI_readMech=S7300MPI_readMech+1;  
      	f_S7300MPI_com_send(S7300MPI_MPIStr7_2,S7300SendLen);
      }
    } 
		else if(3==S7300MPI_readMech)
    {
      if(1==S7300ReadRes)
      {
		  	S7300ReadRes=0;
        S7300MPI_readMech=S7300MPI_readMech+1; 
	      sendTemp[0]=0x10;
	      f_S7300MPI_com_send(sendTemp,1);		  
      }
    } 
		else if(4==S7300MPI_readMech)
    {
      if(1==S7300ReadRes)
      {
		  	S7300ReadRes=0;
        S7300MPI_readMech=S7300MPI_readMech+1; 
	      sendTemp[0]=0x10;
	      f_S7300MPI_com_send(sendTemp,1);		  
      }
    }
		else if(5==S7300MPI_readMech)
    {
      if(1==S7300ReadRes)
      {
		  	S7300ReadRes=0;
        S7300MPI_readMech=S7300MPI_readMech+1; 
	      sendTemp[0]=0x10;
	      f_S7300MPI_com_send(sendTemp,1);	  
      }		
    } 
		else if(6==S7300MPI_readMech)
    {
      if(1==S7300ReadRes)
      {
		  	S7300ReadRes=0;
        S7300MPI_readMech=S7300MPI_readMech+1;
	      sendTemp[0]=0x10;	   	   
	      sendTemp[1]=0x02;
	      f_S7300MPI_com_send(sendTemp,2); 		  
      }
    } 
		else if(7==S7300MPI_readMech)  //-----------------------------------------------------------------------------
    {
      if(1==S7300ReadRes)
      {
		  	S7300ReadRes=0;
        S7300MPI_readMech=S7300MPI_readMech+1; 
		  	S7300SendLen=10;
        S7300MPI_DataEnd[6] =S7300MPI_MPISn;
	      for(i=0,j=0;i<9;i++)
        {
          localArr[j++]=S7300MPI_DataEnd[i];
          if((0x10==S7300MPI_DataEnd[i])&&(i!=7))
        	{
	          localArr[j++]=S7300MPI_DataEnd[i];
	          S7300SendLen++;
	        }
        }	 
        localArr[j] =f_S7300MPI_calc_sum(localArr,0,S7300SendLen-1);
        f_S7300MPI_com_send(localArr,S7300SendLen);		  
      }
    }
		else if(8==S7300MPI_readMech)
    {
      if(1==S7300MPI_FrameRecvFlg)
      {
        //S7300MPI_FrameRecvFlg=0;   //CSP
        //I_display(};
        //Q_display(};
        //M_display(}; //读取一个字节作为测试
        //S_display(}; //读取两个字节作为测试
        //V_display(};
      }
    } 
		else
		{

		}
  }
}

void TIM_1ms_S7300MPI(void)
{
  static u8 flip=OFF;
  flip=~flip;
  timer++;
  if(!flip) return;
  MPI_TimerHandler();
  if(S7300MPI_ConnectMechine==17)
  {
    if(S7300_mpi_Enable==ON)
    {
      g_u16_SwitchTimer[S7300_mpi]++;
    }
    if((ON==MB_NeedWrite(S7300_mpi))&&(READ_IDLE==GW_ReadStatus(S7300_mpi)))
  	{	
      if(GW_WriteStatus(S7300_mpi)<WRITE_RECVSUCCESS)
  	  {
  	     if(g_u16_SwitchTimer[S7300_mpi]>1000)
  	     {
  	     		g_u16_SwitchTimer[S7300_mpi]=0;
  					GW_WriteStatus(S7300_mpi)=WRITE_RECVSUCCESS;
  					NegativeResponse(Err_MBcmd);
  	     } 
  	     else if((GW_WriteStatus(S7300_mpi)==WRITE_IDLE)&&
  	            (g_u16_SwitchTimer[S7300_mpi]>WriteTime))
  	     {	
  		   		GW_WriteStatus(S7300_mpi)=WRITE_PRESEND;
  		   		g_u16_SwitchTimer[S7300_mpi]=0;
  	     }
  	     else
  	     {

  	     }
  	  }
  	  else if(GW_WriteStatus(S7300_mpi)==WRITE_RECVSUCCESS)
  	  {
        GW_WriteStatus(S7300_mpi)=WRITE_DELAY;
        g_u16_SwitchTimer[S7300_mpi]=0;
  	  }
  	  else if(g_u16_SwitchTimer[S7300_mpi]>=WriteTime)
  	  {
        PositiveResponse();
        g_u16_SwitchTimer[S7300_mpi]=0;
        GW_WriteStatus(S7300_mpi)=WRITE_IDLE;
  	    MB_NeedWrite(S7300_mpi)=OFF;
  	    g_u8_threadIdx[S7300_mpi]++;
  		  ThreadNew(S7300_mpi)=ON;
  	  }
  	  else{

  	  }
  	}
  	else
  	{	
  		if((g_u16_SwitchTimer[S7300_mpi]>=UpdateCycle[S7300_mpi])&&
  		    (g_u8_ProtocalNum[S7300_mpi][READ]!=0))
  		{	  
  		  g_u16_SwitchTimer[S7300_mpi]=0;
  		  
  		  if(READ_RECVSUCCESS==GW_ReadStatus(S7300_mpi))
  		  {
  		    GW_ReadStatus(S7300_mpi)=READ_IDLE;
  		    g_u16_TimeoutCnt[S7300_mpi][g_u8_threadIdx[S7300_mpi]]=0;
  		    if(OFF==MB_NeedWrite(S7300_mpi))
  		    {
            g_u8_threadIdx[S7300_mpi]++;
            while(RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[S7300_mpi]++;
              if(g_u8_ProtocalNum[S7300_mpi][READ]<=g_u8_threadIdx[S7300_mpi])
        			{			  
        	      g_u8_threadIdx[S7300_mpi]=0;
        			}
            }
  		      ThreadNew(S7300_mpi)=ON;
  		    }
  		    else
  		    {
            
  		    }
  		  }
  		  else
  		  {
          g_u16_TimeoutCnt[S7300_mpi][g_u8_threadIdx[S7300_mpi]]++;
          if(g_u16_TimeoutCnt[S7300_mpi][g_u8_threadIdx[S7300_mpi]]>=
            (THRES_TIMOUTCNT/UpdateCycle[S7300_mpi]))
          {
            g_u16_RecvTransLen[S7300_mpi][g_u8_threadIdx[S7300_mpi]]=0;
            g_u16_TimeoutCnt[S7300_mpi][g_u8_threadIdx[S7300_mpi]]=0;
            if(OFF==MB_NeedWrite(S7300_mpi))
    		    {
              g_u8_threadIdx[S7300_mpi]++;
              while(RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].cmdstatus.Bits.fbrd==ON)
              {
                g_u8_threadIdx[S7300_mpi]++;
                if(g_u8_ProtocalNum[S7300_mpi][READ]<=g_u8_threadIdx[S7300_mpi])
          			{			  
          	      g_u8_threadIdx[S7300_mpi]=0;
          			}
              }
    		      ThreadNew(S7300_mpi)=ON;
    		    }
    		    else
    		    {
              GW_ReadStatus(S7300_mpi)=READ_IDLE;
    		    }
          }
  		  }
  		}
  	}
  	if(g_u8_ProtocalNum[S7300_mpi][READ]<=g_u8_threadIdx[S7300_mpi])
  	{			  
  		g_u8_threadIdx[S7300_mpi]=0;
  	}
  }
}

void S7300MPI_RecDataHandle(u8 regtype,u8 *sourcebuf,u8 *targetbuf,u8 len)
{
  u8 i;
  if(regtype=='V')
  {
    if((RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].operatesize==0x01)&&
      ((RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].datalenth)==len))
    {
      targetbuf[0]=0x00;
      targetbuf[1]=(sourcebuf[0] >>  
                 (RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].regaddr[7]&0x07))&0x01 ;
    }
    if((RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].operatesize==0x08)&&
      ((RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].datalenth)==len))
    {
      for(i=0;i<len;i++)
      {
        targetbuf[2*i+0]=0x00;
        targetbuf[2*i+1]=sourcebuf[i];
      }
    }
    else if((RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].operatesize==0x10)&&
      ((RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].datalenth*2)==len))
    {
      memcpy(targetbuf,sourcebuf,len);
    }
    else
    {

    }
  }
  else if((RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].regtype=='I')||
          (RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].regtype=='Q')||
          (RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].regtype=='S')||
          (RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].regtype=='M'))
  {
    targetbuf[0]=0x00;
    targetbuf[1]=sourcebuf[0] >>  
                 (RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].regaddr[7]&0x07) ;
  }
  else
  {

  }
  g_u16_RecvTransLen[S7300_mpi][g_u8_threadIdx[S7300_mpi]]=
          RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].datalenth * 2;
}

void UART_S7300MPI_Recv(u8 recvData)
{
  u8 i,buflen;
  u8 k;  
  u8 localArr[50];
  if(S7300MPI_ConnectMechine<17)
  {
    //提取数据
    S7300MPI_RxBuf[S7300MPI_transRecvAlready] =recvData;
    S7300MPI_transRecvAlready=S7300MPI_transRecvAlready+1;
  } 
  //已经建立连接
  else
  {
    if(6==S7300MPI_readMech)
    {
      //提取数据
        S7300MPI_chksum= S7300MPI_chksum^recvData;
        //没有收到帧头
        if(S7300MPI_FrameStart==0)
        {
          if(0x00==recvData)
          {
            S7300MPI_chksum=0;
            S7300MPI_transRecvAlready=0;
            S7300MPI_head00RecvFlg=1;
            S7300MPI_RxBuf[S7300MPI_transRecvAlready] =recvData;
            S7300MPI_transRecvAlready=S7300MPI_transRecvAlready+1;
          } 
		  		else if(1==S7300MPI_head00RecvFlg)
          {
            if(0x0C==recvData)
            {
              S7300MPI_FrameStart=1;
              S7300MPI_head00RecvFlg=0;
              S7300MPI_RxBuf[S7300MPI_transRecvAlready] =recvData;
              S7300MPI_transRecvAlready=S7300MPI_transRecvAlready+1;
            }
          }
          else
          {

          }
        } 
				else if(S7300MPI_FrameStart==1)
        {
          if((0x10==recvData)&&(1!=S7300MPI_Tail03RecvFlg))
          {
            if(1==S7300MPI_Tail10RecvFlg)
            {
              S7300MPI_Tail10RecvFlg=0;
            } 
						else
            {
              S7300MPI_RxBuf[S7300MPI_transRecvAlready] =recvData;
              S7300MPI_transRecvAlready=S7300MPI_transRecvAlready+1;
              S7300MPI_Tail10RecvFlg=1;
            }
          } 
		  		else
          {
            if(1==S7300MPI_Tail10RecvFlg)
            {
              S7300MPI_Tail10RecvFlg=0;
              S7300MPI_RxBuf[S7300MPI_transRecvAlready] =recvData;
              S7300MPI_transRecvAlready=S7300MPI_transRecvAlready+1;
              if(0x03==recvData)
              {
                S7300MPI_Tail03RecvFlg=1;
              }
            } 
						else if(1==S7300MPI_Tail03RecvFlg)
            {
              S7300MPI_RxBuf[S7300MPI_transRecvAlready] =recvData;
              S7300MPI_transRecvAlready=S7300MPI_transRecvAlready+1;
              
              if(S7300MPI_chksum==0)
              {
                S7300MPI_FrameRecvFlg=1;   
	              S7300ReadRes=1;
				        S7300MPI_NewReqFlg=0;   
				        delta_timer=timer;
				        buflen=S7300MPI_transRecvAlready-27;
                S7300MPI_RecDataHandle(RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].regtype,&S7300MPI_RxBuf[24],
		  	          &g_u8_EthRespData[S7300_mpi][g_u16_StartAddr[S7300_mpi][g_u8_threadIdx[S7300_mpi]]*2],buflen);
  			        GW_ReadStatus(S7300_mpi)=READ_RECVSUCCESS;
				        S7300MPI_transRecvAlready=0;
              } 
			  			else
              {
                S7300MPI_FrameRecvFlg=0;
              }
              S7300MPI_FrameStart=0;
              S7300MPI_head00RecvFlg=0;
              S7300MPI_head0CRecvFlg=0;
              S7300MPI_Tail10RecvFlg=0;
              S7300MPI_Tail03RecvFlg=0;
            } 
						else
            {
              S7300MPI_RxBuf[S7300MPI_transRecvAlready] =recvData;
              S7300MPI_transRecvAlready=S7300MPI_transRecvAlready+1;
            }
          }
      }
    }
	//41      [00000322]  IRP_MJ_READ                     Length: 0001, Data: 10 
	else if(2==S7300MPI_readMech)  //------------------------------------------------------------------------------------------
	{
	  if(0x10==recvData)
	  {
	    S7300ReadRes=1;
		S7300MPI_transRecvAlready=0;
	  }		  	  			   	  
	}	
	//43      [00000326]  IRP_MJ_READ                     Length: 0002, Data: 10 02 
	else if(3==S7300MPI_readMech)
	{	
      //提取数据
      S7300MPI_RxBuf[S7300MPI_transRecvAlready] =recvData;
      S7300MPI_transRecvAlready=S7300MPI_transRecvAlready+1;
	  
	  if(S7300MPI_transRecvAlready>=2)
	  {
	    if((0x10==S7300MPI_RxBuf[0])&&(0x02==S7300MPI_RxBuf[1]))
		{
	      S7300ReadRes=1;
		  S7300MPI_transRecvAlready=0;
		}
	  }	
	}
	//45      [00000328]  IRP_MJ_READ                     Length: 0010, Data: 00 0C 14 03 B0 01 01 10 03 B8 
    //270     [00000575]  IRP_MJ_READ                     Length: 0011, Data: 00 0C 14 03 B0 01 10 10 10 03 B9 
	else if(4==S7300MPI_readMech)
	{	
      //提取数据
      S7300MPI_RxBuf[S7300MPI_transRecvAlready] =recvData;
      S7300MPI_transRecvAlready=S7300MPI_transRecvAlready+1;
				   
	  if(S7300MPI_MPISn==16)	
	  {
	    if(S7300MPI_transRecvAlready>=11)
	    {
	      if((0x00==S7300MPI_RxBuf[0])&&
		     (0x0c==S7300MPI_RxBuf[1])&&
		     (0x14==S7300MPI_RxBuf[2])&&
		     (0x03==S7300MPI_RxBuf[3]))
		  {
	        S7300ReadRes=1;
		    S7300MPI_transRecvAlready=0;
		  }
	    }	
	  }
	  else
	  {	
	    if(S7300MPI_transRecvAlready>=10)
	    {
	      if((0x00==S7300MPI_RxBuf[0])&&
		     (0x0c==S7300MPI_RxBuf[1])&&
		     (0x14==S7300MPI_RxBuf[2])&&
		     (0x03==S7300MPI_RxBuf[3]))
		  {
	        S7300ReadRes=1;
		    S7300MPI_transRecvAlready=0;
		  }
	    }
	  }
	}
	//47      [00000330]  IRP_MJ_READ                     Length: 0001, Data: 02 
	else if(5==S7300MPI_readMech)
	{
      //提取数据
      S7300MPI_RxBuf[S7300MPI_transRecvAlready] =recvData;
      S7300MPI_transRecvAlready=S7300MPI_transRecvAlready+1;
	  
	  if(S7300MPI_transRecvAlready>=1)
	  {
	    if((0x02==S7300MPI_RxBuf[0]))
		{			   
	      S7300ReadRes=1;
		  S7300MPI_transRecvAlready=0;
		}
	  }	

	}
	//52      [00000335]  IRP_MJ_READ                     Length: 0001, Data: 10 
	else if(7==S7300MPI_readMech)
	{
      //提取数据
      S7300MPI_RxBuf[S7300MPI_transRecvAlready] =recvData;
      S7300MPI_transRecvAlready=S7300MPI_transRecvAlready+1;
	  
	  if(S7300MPI_transRecvAlready>=1)
	  {
	    if((0x10==S7300MPI_RxBuf[0]))
		{
	      S7300ReadRes=1;
		  S7300MPI_transRecvAlready=0;
		}
	  }	
	}
	//54      [00000337]  IRP_MJ_READ                     Length: 0001, Data: 10 
	else if(8==S7300MPI_readMech)
	{
      //提取数据
    S7300MPI_RxBuf[S7300MPI_transRecvAlready] =recvData;
    S7300MPI_transRecvAlready=S7300MPI_transRecvAlready+1;
	  
	  if(S7300MPI_transRecvAlready>=1)
	  {
	    if((0x10==S7300MPI_RxBuf[0]))
			{
	      S7300ReadRes=1;
		  	S7300MPI_transRecvAlready=0;
			}
	  }	
	}
  }
  
  if(S7300MPI_ConnectMechine==1)
  {
    if(recvData==0x10)
    {
      S7300MPI_ConnectMechine=2;
      S7300MPI_transRecvAlready=0;
	  	f_S7300MPI_com_send(S7300MPI_ReqStr2,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    }
  } 
  else if(S7300MPI_ConnectMechine==2)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[2])
    {
      S7300MPI_ConnectMechine=3;
      S7300MPI_transRecvAlready=0;  
	    f_S7300MPI_com_send(S7300MPI_ReqStr3,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } 
		else
    {
    }
  } 
  else if(S7300MPI_ConnectMechine==3)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[3])
    {
      S7300MPI_ConnectMechine=4;
      S7300MPI_transRecvAlready=0;
	  	f_S7300MPI_com_send(S7300MPI_ReqStr4,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } 
		else
    {
    }
  }
  else if(S7300MPI_ConnectMechine==4)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[4])
    {
      S7300MPI_ConnectMechine=5;
      S7300MPI_transRecvAlready=0;
	    f_S7300MPI_com_send(S7300MPI_ReqStr5,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } 
		else
    {
    }
  } 
  else if(S7300MPI_ConnectMechine==5)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[5])
    {
      S7300MPI_ConnectMechine=6;
      S7300MPI_transRecvAlready=0;
	    f_S7300MPI_com_send(S7300MPI_ReqStr6,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } 
		else
    {
    }
  } 
  else if(S7300MPI_ConnectMechine==6)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[6])
    {
      S7300MPI_ConnectMechine=7;
      S7300MPI_transRecvAlready=0;
	    f_S7300MPI_com_send(S7300MPI_ReqStr7,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } 
		else
    {
    }
  } 
  else if(S7300MPI_ConnectMechine==7)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[7])
    {
      S7300MPI_ConnectMechine=8;
      S7300MPI_transRecvAlready=0;
	   	f_S7300MPI_com_send(S7300MPI_ReqStr8,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } 
    else
    {
    }
	} 
  else if(S7300MPI_ConnectMechine==8)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[8])
    {
      S7300MPI_ConnectMechine=9;
      S7300MPI_transRecvAlready=0;
	    f_S7300MPI_com_send(S7300MPI_ReqStr9,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } 
    else
    {
    }
  } 
  else if(S7300MPI_ConnectMechine==9)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[9])
    {
        S7300MPI_ConnectMechine=10;
        S7300MPI_transRecvAlready=0;
	    f_S7300MPI_com_send(S7300MPI_ReqStr10,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } else
    {
    }
  } 
  else if(S7300MPI_ConnectMechine==10)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[10])
    {
        S7300MPI_ConnectMechine=11;
        S7300MPI_transRecvAlready=0;
	    f_S7300MPI_com_send(S7300MPI_ReqStr11,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } else
    {
    }
  } 
  else if(S7300MPI_ConnectMechine==11)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[11])
    {
        S7300MPI_ConnectMechine=12;
        S7300MPI_transRecvAlready=0;
	    f_S7300MPI_com_send(S7300MPI_ReqStr12,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } else
    {
    }
  }
  else if(S7300MPI_ConnectMechine==12)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[12])
    {
        S7300MPI_ConnectMechine=13;
        S7300MPI_transRecvAlready=0;
	    f_S7300MPI_com_send(S7300MPI_ReqStr13,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } 
	else
    {
    }
  } 
  else if(S7300MPI_ConnectMechine==13)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[13])
    {
        S7300MPI_ConnectMechine=14;
        S7300MPI_transRecvAlready=0;
	    f_S7300MPI_com_send(S7300MPI_ReqStr14,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } 
	else
    {
    }
  } 
  else if(S7300MPI_ConnectMechine==14)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[14])
    {
      S7300MPI_ConnectMechine=15;
      S7300MPI_transRecvAlready=0;
	    f_S7300MPI_com_send(S7300MPI_ReqStr15,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } 
		else
    {
    }
  } 
  else if(S7300MPI_ConnectMechine==15)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[15])
    {
      S7300MPI_ConnectMechine=16;
      S7300MPI_transRecvAlready=0;
	    f_S7300MPI_com_send(S7300MPI_ReqStr16,S7300MPI_ConReqLen[S7300MPI_ConnectMechine]);
    } 
		else
    {
    }
  } 
  else if(S7300MPI_ConnectMechine==16)
  {
    if(S7300MPI_transRecvAlready>S7300MPI_ConResLen[16])
    {
        S7300MPI_ConnectMechine=17;  
        S7300MPI_transRecvAlready=0;
    } 
		else
    {
    }
  }
}

void f_S7300MPI_task(void)
{
  u16 db_block,db_addr;
  
  if(ThreadNew(S7300_mpi) == ON)
	{
		ThreadNew(S7300_mpi) = OFF;
		timer=0;
		GW_ReadStatus(S7300_mpi)=READ_WAITRESPOND;
		db_block=(RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].regaddr[2]<<8)
      + RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].regaddr[3];
    db_addr=(RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].regaddr[6]<<8)
      + RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].regaddr[7];
    S7300_MPI_read(RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].regtype,
    	db_block,db_addr,RegisterCfgBuff[S7300_mpi][g_u8_threadIdx[S7300_mpi]].datalenth);
	}
}
