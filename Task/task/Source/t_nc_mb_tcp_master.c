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
/****************************��д����******************************/
	
static u8 g_u8M70Vcnc_s0send_1[14] = {0x00,0x00,0x00,0x00,0x00,0x08,0x01,0x03,0x00,0x00,0x00,0x01,0x69,0xd3};
static u8 g_u8M70Vcnc_s0send_2[14] = {0x00,0x00,0x00,0x00,0x00,0x08,0x01,0x03,0x00,0x01,0x00,0x01,0x38,0x13};
static u8 g_u8M70Vcnc_s0send_3[14] = {0x00,0x00,0x00,0x00,0x00,0x08,0x01,0x03,0x00,0x02,0x00,0x01,0xc8,0x13};
static u8 g_u8M70Vcnc_s0send_4[14] = {0x00,0x00,0x00,0x00,0x00,0x08,0x01,0x03,0x00,0x03,0x00,0x01,0x99,0xd3};

u8 M70VNC_TCP=0xff;;
u8 M70VNC_Recbuf[30];
/*******************************************************************************
* ������  : Process_Socket_Data
* ����    : W5500���ղ����ͽ��յ�������
* ����    : s:�˿ں�
* ���    : ��
* ����ֵ  : ��
* ˵��    : �������ȵ���S_rx_process()��W5500�Ķ˿ڽ������ݻ�������ȡ����,
*			Ȼ�󽫶�ȡ�����ݴ�Rx_Buffer������Temp_Buffer���������д���
*			������ϣ������ݴ�Temp_Buffer������Tx_Buffer������������S_tx_process()
*			�������ݡ�
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
  //����
  u16 size,i;
	
    //GW_RecvSuccess(NODE_M70V_NC)=0;
	
  g_u8M70VNcConMech =g_u8_threadIdx[M70VNC_TCP];
	switch(g_u8M70VNcConMech)
	  {
	     case 0: //��ȡ�����趨�ٶȵ���        
						 
  	   // MasterWrite_SOCK_Data_Buffer(0,g_u8cnc_s0send_1,80);
			  DAQ_EthSend(0,g_u8M70Vcnc_s0send_1,14);

		 break;

	     case 1: //��ȡ�����趨�ٶȸ���
          
  	   //MasterWrite_SOCK_Data_Buffer(0,g_u8cnc_s0send_2,80);
			  DAQ_EthSend(0,g_u8M70Vcnc_s0send_2,14);
			// g_u8_RecvTransBuf[NODE_M70V_NC][ReadRegBuff[NODE_M70V_NC][g_u8_threadIdx[NODE_M70V_NC]].regaddr[7]][0] = mb_tcp_master_Rx_Buffer[37];
       //g_u8_RecvTransBuf[NODE_M70V_NC][ReadRegBuff[NODE_M70V_NC][g_u8_threadIdx[NODE_M70V_NC]].regaddr[7]][1] = mb_tcp_master_Rx_Buffer[36];						  
			// GW_SendStatus(NODE_M70V_NC)=RECVFINISH; 
			 
      						 
		 break;

		 case 2://��ȡ���ᵱǰ���ȵ���	    
		  
  	 // MasterWrite_SOCK_Data_Buffer(0,g_u8cnc_s0send_3,80);
		   DAQ_EthSend(0,g_u8M70Vcnc_s0send_3,14);

		 break;

		 case 3://��ȡ���ᵱǰ���ȸ���	  			 
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
	
	mb_tcp_master_W5500_Socket_Set(0);//�������
  mb_tcp_master_W5500_Interrupt_Process(0);//W5500�жϴ��������
	if(MasterRead_SOCK_Data_Buffer(0,M70VNC_Recbuf)){
		   GW_WriteStatus(M70VNC_TCP)=READ_RECVSUCCESS     ;
		   g_u16_RecvTransLen[M70VNC_TCP][g_u8_threadIdx[M70VNC_TCP]]=RegisterCfgBuff[M70VNC_TCP][g_u8_threadIdx[M70VNC_TCP]].datalenth*2;
		   memcpy(&g_u8_EthRespData[M70VNC_TCP][g_u16_StartAddr[M70VNC_TCP][g_u8_threadIdx[M70VNC_TCP]]*2],
              &M70VNC_Recbuf[9],2);
	}
	
	//GW_SendStatus(NODE_M70V_NC)=RECVFINISH;
	//NegativeResponse(Err_MBcmd);
	
	//��ǰ���� �� û��д����
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
