#include "main.h"
#include <string.h>
#include <stdio.h>
#include "m_w5500.h"
u8  g_u8M70VNcParaRead[20]={0};  
u8  g_u8M70VNcParaReadValid=0;
u8  g_u8M70VNcConMech=0;//0 start 
u8 g_u8M70Vnc_Enable=OFF;
 //1 s0 handshake ok wait to connect
 //2 s0 send connect wait response
 //3 s0 receive response wait to send s1 handshake 
 //4 s1 begin send conconnect response then connect s1, 3 s1 connect ok ,
/****************************读写命令******************************/
	
static u8 g_u8M70Vcnc_s0send_1[14] = {0x00,0x00,0x00,0x00,0x00,0x08,0x01,0x03,0x00,0x00,0x00,0x01,0x69,0xd3};
static u8 g_u8M70Vcnc_s0send_2[14] = {0x00,0x00,0x00,0x00,0x00,0x08,0x01,0x03,0x00,0x01,0x00,0x01,0x38,0x13};
static u8 g_u8M70Vcnc_s0send_3[14] = {0x00,0x00,0x00,0x00,0x00,0x08,0x01,0x03,0x00,0x02,0x00,0x01,0xc8,0x13};
static u8 g_u8M70Vcnc_s0send_4[14] = {0x00,0x00,0x00,0x00,0x00,0x08,0x01,0x03,0x00,0x03,0x00,0x01,0x99,0xd3};

u8 M70VNC_TCP=0xff;;
u8 M70VNC_Recbuf[30];
/*******************************************************************************
* 函数名  : Process_Socket_Data
* 描述    : W5500接收并发送接收到的数据
* 输入    : s:端口号
* 输出    : 无
* 返回值  : 无
* 说明    : 本过程先调用S_rx_process()从W5500的端口接收数据缓冲区读取数据,
*			然后将读取的数据从Rx_Buffer拷贝到Temp_Buffer缓冲区进行处理。
*			处理完毕，将数据从Temp_Buffer拷贝到Tx_Buffer缓冲区。调用S_tx_process()
*			发送数据。
*******************************************************************************/
	


void M70V_NC_TCP_Init(u8 nodeID)
{
  g_u8M70VNcConMech=0;
  M70VNC_TCP=nodeID;
  g_u8M70Vnc_Enable=ON;
}

/********************************************************************************************/	 
/********************************************************************************************/

void M70V_NC_SocketConnect(u8 socket)
{
	 //MasterWrite_SOCK_Data_Buffer(0,g_u8cnc_s0send_1,80);

		DAQ_EthSend(socket,g_u8M70Vcnc_s0send_1,14);
	
}

void TIM_1ms_M70V_NC(void)
{
  if(g_u8M70Vnc_Enable==ON)
  {
  	g_u16_SwitchTimer[M70VNC_TCP]++; 
  }	
  if((ON==MB_NeedWrite(M70VNC_TCP))&&(READ_WAITRESPOND!=GW_ReadStatus(M70VNC_TCP)))
	{
    if(GW_WriteStatus(M70VNC_TCP)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[M70VNC_TCP]>1000)
	     {
	     		g_u16_SwitchTimer[M70VNC_TCP]=0;
					GW_WriteStatus(M70VNC_TCP)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(M70VNC_TCP)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[M70VNC_TCP]>WriteTime))
	     {	
		   		GW_WriteStatus(M70VNC_TCP)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[M70VNC_TCP]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(M70VNC_TCP)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(M70VNC_TCP)=WRITE_DELAY;
      g_u16_SwitchTimer[M70VNC_TCP]=0;
	  }
	  else if(g_u16_SwitchTimer[M70VNC_TCP]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[M70VNC_TCP]=0;
      GW_WriteStatus(M70VNC_TCP)=WRITE_IDLE;
	    MB_NeedWrite(M70VNC_TCP)=OFF;
	    g_u8_threadIdx[M70VNC_TCP]++;
		  ThreadNew(M70VNC_TCP)=ON;
	  }
	  else{

	  }
	}
	else
	{
		if((g_u16_SwitchTimer[M70VNC_TCP]>UpdateCycle[M70VNC_TCP])&&
		    (g_u8_ProtocalNum[M70VNC_TCP][READ]!=0))
		{	  
		  g_u16_SwitchTimer[M70VNC_TCP]=0;
		  
		  if(READ_RECVSUCCESS == GW_ReadStatus(M70VNC_TCP))
		  {
		    GW_ReadStatus(M70VNC_TCP)=READ_IDLE;
		    g_u16_TimeoutCnt[M70VNC_TCP][g_u8_threadIdx[M70VNC_TCP]]=0;
		    if(OFF==MB_NeedWrite(M70VNC_TCP))
		    {
          g_u8_threadIdx[M70VNC_TCP]++;
          while(RegisterCfgBuff[M70VNC_TCP][g_u8_threadIdx[M70VNC_TCP]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[M70VNC_TCP]++;
            if(g_u8_ProtocalNum[M70VNC_TCP][READ]<=g_u8_threadIdx[M70VNC_TCP])
      			{			  
      	      g_u8_threadIdx[M70VNC_TCP]=0;
      			}
          }
		      ThreadNew(M70VNC_TCP)=ON;
		    }
		    else
		    {
          GW_ReadStatus(M70VNC_TCP)=READ_IDLE;
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[M70VNC_TCP][g_u8_threadIdx[M70VNC_TCP]]++;
        if(g_u16_TimeoutCnt[M70VNC_TCP][g_u8_threadIdx[M70VNC_TCP]]>
          (THRES_TIMOUTCNT/UpdateCycle[M70VNC_TCP]))
        {
          g_u16_RecvTransLen[M70VNC_TCP][g_u8_threadIdx[M70VNC_TCP]]=0;
          g_u16_TimeoutCnt[M70VNC_TCP][g_u8_threadIdx[M70VNC_TCP]]=0;
          if(OFF==MB_NeedWrite(M70VNC_TCP))
  		    {
            g_u8_threadIdx[M70VNC_TCP]++;
            while(RegisterCfgBuff[M70VNC_TCP][g_u8_threadIdx[M70VNC_TCP]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[M70VNC_TCP]++;
              if(g_u8_ProtocalNum[M70VNC_TCP][READ]<=g_u8_threadIdx[M70VNC_TCP])
        			{			  
        	      g_u8_threadIdx[M70VNC_TCP]=0;
        			}
            }
  		      ThreadNew(M70VNC_TCP)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(M70VNC_TCP)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[M70VNC_TCP][READ]<=g_u8_threadIdx[M70VNC_TCP])
	{			  
		g_u8_threadIdx[M70VNC_TCP]=0;
	}
}


void M70V_nc_read(void)
{		
  //发送
  u16 size,i;
	
    //GW_RecvSuccess(NODE_M70V_NC)=0;
	
  g_u8M70VNcConMech =g_u8_threadIdx[M70VNC_TCP];
	switch(g_u8M70VNcConMech)
	  {
	     case 0: //读取主轴设定速度低字        
						 
  	   // MasterWrite_SOCK_Data_Buffer(0,g_u8cnc_s0send_1,80);
			  DAQ_EthSend(0,g_u8M70Vcnc_s0send_1,14);

		 break;

	     case 1: //读取主轴设定速度高字
          
  	   //MasterWrite_SOCK_Data_Buffer(0,g_u8cnc_s0send_2,80);
			  DAQ_EthSend(0,g_u8M70Vcnc_s0send_2,14);
			// g_u8_RecvTransBuf[NODE_M70V_NC][ReadRegBuff[NODE_M70V_NC][g_u8_threadIdx[NODE_M70V_NC]].regaddr[7]][0] = mb_tcp_master_Rx_Buffer[37];
       //g_u8_RecvTransBuf[NODE_M70V_NC][ReadRegBuff[NODE_M70V_NC][g_u8_threadIdx[NODE_M70V_NC]].regaddr[7]][1] = mb_tcp_master_Rx_Buffer[36];						  
			// GW_SendStatus(NODE_M70V_NC)=RECVFINISH; 
			 
      						 
		 break;

		 case 2://读取主轴当前数度低字	    
		  
  	 // MasterWrite_SOCK_Data_Buffer(0,g_u8cnc_s0send_3,80);
		   DAQ_EthSend(0,g_u8M70Vcnc_s0send_3,14);

		 break;

		 case 3://读取主轴当前数度高字	  			 
  	 //MasterWrite_SOCK_Data_Buffer(0,g_u8cnc_s0send_4,80); 
		 DAQ_EthSend(0,g_u8M70Vcnc_s0send_4,14); 

		 break;			
		 		 		 	
		 default:
		 break;
	 };
 // }
}	



void f_M70V_NCTCP_task(void)
{

	 //GW_RecvSuccess(NODE_M70V_NC)=0;
	
	mb_tcp_master_W5500_Socket_Set(0);//检查连接
  mb_tcp_master_W5500_Interrupt_Process(0);//W5500中断处理程序框架
	if(MasterRead_SOCK_Data_Buffer(0,M70VNC_Recbuf)){
		   GW_WriteStatus(M70VNC_TCP)=READ_RECVSUCCESS     ;
		   g_u16_RecvTransLen[M70VNC_TCP][g_u8_threadIdx[M70VNC_TCP]]=RegisterCfgBuff[M70VNC_TCP][g_u8_threadIdx[M70VNC_TCP]].datalenth*2;
		   memcpy(&g_u8_EthRespData[M70VNC_TCP][g_u16_StartAddr[M70VNC_TCP][g_u8_threadIdx[M70VNC_TCP]]*2],
              &M70VNC_Recbuf[9],2);
	}
	
	//GW_SendStatus(NODE_M70V_NC)=RECVFINISH;
	//NegativeResponse(Err_MBcmd);
	
	//当前空闲 且 没有写操作
	if((ThreadNew(M70VNC_TCP) == ON))
	{
		ThreadNew(M70VNC_TCP) = OFF;
		M70V_nc_read();
		
	}	
  else if(WRITE_PRESEND==GW_WriteStatus(M70VNC_TCP))
  {		   
    GW_WriteStatus(M70VNC_TCP)=WRITE_WAITRESPOND;
  }
  else
  {

  }
}
