
#include "main.h"
/*********USART2************/
void DMA1_Channel7_HW_Start(u8 *_buf, u16 _len)
{
    DMA1_Channel7->CMAR = (u32)_buf;
    DMA1_Channel7->CNDTR = _len;
    DMA_Cmd(DMA1_Channel7, ENABLE);
}
void DMA1_Channel7_IRQHandler(void)
{
    DMA_ClearITPendingBit(DMA1_IT_GL7 | DMA1_IT_TC7 | DMA1_IT_HT7 | DMA1_IT_TE7);
    DMA_Cmd(DMA1_Channel7, DISABLE);
    
    USART_ITConfig(USART2, USART_IT_TC, ENABLE);
}

/*********USART3************/
void DMA1_Channel2_HW_Start(u8 *_buf, u16 _len)
{
    DMA1_Channel2->CMAR = (u32)_buf;
    DMA1_Channel2->CNDTR = _len;
    DMA_Cmd(DMA1_Channel2, ENABLE);
}
void DMA1_Channel2_IRQHandler(void)
{
    DMA_ClearITPendingBit(DMA1_IT_GL2 | DMA1_IT_TC2 | DMA1_IT_HT2 | DMA1_IT_TE2);
    DMA_Cmd(DMA1_Channel2, DISABLE);
    
    USART_ITConfig(USART3, USART_IT_TC, ENABLE);
}

/*********UART4************/
void DMA2_Channel5_HW_Start(u8 *_buf, u16 _len)
{
    DMA2_Channel5->CMAR = (u32)_buf;
    DMA2_Channel5->CNDTR = _len;
    DMA_Cmd(DMA2_Channel5, ENABLE);
}
void DMA2_Channel5_IRQHandler(void)
{
    DMA_ClearITPendingBit(DMA2_IT_GL5 | DMA2_IT_TC5 | DMA2_IT_HT5 | DMA2_IT_TE5);
    DMA_Cmd(DMA2_Channel5, DISABLE);
    
    USART_ITConfig(UART4, USART_IT_TC, ENABLE);
}


void DAQ_LIB_Init(void)
{
	u8 i;
	for(i=0;i<MAXNUM_NODE-1;i++)//最后一个节点固定为其他用途，如版本号，IO控制等
	{
		switch(g_u8_ProtocalID[i])
		{
			case NODE_FX2N485BD:Fx2n485bd_Init(i);
					 break;
			case NODE_FX2NPROGRAM:Fx2nProgram_Init(i);
					 break;
			case NODE_S7200PPI:S7200PPI_Init(i);
					 break;
			case NODE_FPPROGRAM:FPprogram_Init(i);
					 break;
			case NODE_CP1LCOM:CP1LCom_Init(i);
					 break;
			case NODE_MODBUSRTU:ModbusRTU_Init(i);
					 break;
			case NODE_M70V_NC:M70V_NC_TCP_Init(i);
					 break;
			case NODE_S7200TCP:S7200TCP_Init(i);
					 break;
			case NODE_S7300TCP:S7300TCP_Init(i);
					 break;
			case NODE_MBTCP:ModbusTCp_Init(i);
					 break;
			case NODE_MITSUBISHIQ:MitsubishiQtcp_Init(i);
					 break;
			case NODE_MitsubishiA:MitsubishiA_Init(i);
					 break;
			case NODE_FX3UENET_ADP2:Fx3uEnet_adp2_Init(i);
					 break;
			case NODE_AB_COMPACTTCP:AB_Compact_tcp_Init(i);
                     break;
            case NODE_FX2PROG:Fx2Prog_Init(i);
					 break;
			case NODE_AB_Micro232:AB_Micro232_Init(i);
                     break;
			case NODE_S7300MPI:S7300_mpi_Init(i);
                     break;
			case NODE_CP1L_FINS:
                     break;
			case NODE_MITSUBISHIQBIN:MitsubishiQBINtcp_Init(i);
					 break;
			case NODE_MITSUBISHIQSERIAL:MitsubishiQserial_Init(i);
					 break;
			case NODE_S7400TCP:S7400TCP_Init(i);
					 break;
            case NODE_S1200TCP:S1200TCP_Init(i);
                     break;
            case NODE_SMART200TCP:Smart200TCP_Init(i);
                     break;
			default:break;
		}
	}
}

void DAQ_UartSend(u8 *sendbuf, u8 lenth,u8 specchn)
{
	u8 uartchn;
  if((specchn == CHN_UART_CFG)&&(specchn>CHANNEL_UART5))
  {
    uartchn = CommPort;
  }
  else
  {
    uartchn = specchn;
  }
  
  switch(uartchn)
	{
    case CHANNEL_UART1: 
      USART1_Send_Data(sendbuf,lenth); 
      break;
    case CHANNEL_UART2: 
      USART2_485_TX_ENABLE;
      DMA1_Channel7_HW_Start(sendbuf,lenth);
      //USART2_Send_Data(sendbuf,lenth); 
      break;
    case CHANNEL_UART3: 
      USART3_485_TX_ENABLE;
      DMA1_Channel2_HW_Start(sendbuf,lenth);
      //USART3_Send_Data(sendbuf,lenth); 
      break;
    case CHANNEL_UART4: 
      DMA2_Channel5_HW_Start(sendbuf,lenth);
      //UART4_Send_Data(sendbuf,lenth); 
      break;
    case CHANNEL_UART5: 
      UART5_Send_Data(sendbuf,lenth); 
      break;
    default:break;
	}
}

void DAQ_DataRecv(u8 channel,u8 data)
{
  if(CommPort == channel)
  {
    if((CommParmCfg.databit == USART_WordLength_8b)&&(CommParmCfg.paritybit!=USART_Parity_No))
    {
      data = data & 0x7f;
    }
		//如果该板卡同时接多个节点，使用不止一个串口时，需要加此if判断是哪个串口的数据
    switch(g_u8_ProtocalID[0])
		{
			case NODE_FX2N485BD:UART_Fx2n485bd_Recv(data);
					 break;
			case NODE_FX2NPROGRAM:UART_Fx2nProgram_Recv(data);
					 break;
			case NODE_S7200PPI:UART_S7200ppi_Recv(data);
					 break;
			case NODE_FPPROGRAM:UART_FPProgram_Recv(data);
					 break;
			case NODE_CP1LCOM:UART_CPCom_Recv(data);
					 break;
			case NODE_MODBUSRTU:UART_ModbusRTU_Recv(data);
					 break;
			case NODE_MitsubishiA:UART_MitsubishiA_Recv(data);
					 break;
			case NODE_FX2PROG:UART_Fx2Prog_Recv(data);
					 break;
			case NODE_AB_Micro232:UART_AB_Micro232_Recv(data);
			     break;
			case NODE_S7300MPI:UART_S7300MPI_Recv(data);
			     break;
      case NODE_MITSUBISHIQSERIAL:UART_MitsubishiQserial_Recv(data);
           break;
			default:break;
		}
  }
}

void DAQ_EthSend(u8 socket,u8 *sendbufer,u8 lenth)
{
  MasterWrite_SOCK_Data_Buffer(socket,sendbufer,lenth);
}

void SocketConnectHook(u8 socket)
{
  switch(g_u8_ProtocalID[0])//如果同一个板卡接多个不同接口设备，需要先配置以太网口
	{
    case NODE_M70V_NC: 
      M70V_NC_SocketConnect(socket); 
      break;
    default:break;
	}
}

void DAQ_Timer(void)
{
	static u16 LED_blinktime,g_TimeCount;
	u8 i;
  for(i=0;i<MAXNUM_NODE-1;i++)//最后一个节点固定为其他用途，如版本号，IO控制等
	{
		switch(g_u8_ProtocalID[i])
		{
			case NODE_FX2NPROGRAM:TIM_1ms_Fx2nProgram();
					 break;
            case NODE_FX2N485BD:TIM_1ms_Fx2n485bd();
					 break;
			case NODE_S7200PPI:TIM_1ms_S7200PPI();
					 break;
			case NODE_FPPROGRAM:TIM_1ms_FPProgram();
					 break;
			case NODE_CP1LCOM:TIM_1ms_CP1LCom();
					 break;
			case NODE_MODBUSRTU:TIM_1ms_ModbusRTU();
					 break;
			case NODE_M70V_NC:TIM_1ms_M70V_NC();
					 break;
			case NODE_S7200TCP:TIM_1ms_S7200TCP();
					 break;
			case NODE_S7300TCP:TIM_1ms_S7300TCP();
					 break;
			case NODE_MBTCP:TIM_1ms_MBTCPMaster();
					 break;
			case NODE_MITSUBISHIQ:TIM_1ms_MitsubishiQtcp();
					 break;
			case NODE_MitsubishiA:TIM_1ms_MitsubishiA();
					 break;
			case NODE_FX3UENET_ADP2:TIM_1ms_Fx3uEnet_adp2();
					 break;
			case NODE_AB_COMPACTTCP:TIM_1ms_ABCompacttcp();
			     break;
            case NODE_FX2PROG:TIM_1ms_Fx2Prog();
					 break;	
			case NODE_AB_Micro232:TIM_1ms_AB_Micro232();
			     break;
			case NODE_S7300MPI:TIM_1ms_S7300MPI();
			     break;
			case NODE_CP1L_FINS:
			     break;
			case NODE_MITSUBISHIQBIN:TIM_1ms_MitsubishiQBINtcp();
					 break;
			case NODE_MITSUBISHIQSERIAL:TIM_1ms_MitsubishiQserial();
					 break;
			case NODE_S7400TCP:TIM_1ms_S7400TCP();
					 break;
            case NODE_S1200TCP:TIM_1ms_S1200TCP();
                     break;
            case NODE_SMART200TCP:TIM_1ms_Smart200TCP();
			default:break;
		}
	}
	
	UpdateTime++;
	//1.延时函数
	if(TimeDelayCount>0)TimeDelayCount--;	

	//2.重连管理
    //f_S7200_TCPConnectTimer();
	g_u16_ETH_Timeout++;
	//3.led 1s 闪烁灯 
	g_TimeCount++; 
	if(g_TimeCount<=100)
	{
		LED1_OFF;						
	}
	else if(g_TimeCount<200)
	{
		LED1_ON;
	}
	else 
	{  
		g_TimeCount=0;
	}

	if(DetectTime)
	{
		DetectTime--;
	}
}

void DAQ_Mainfunction(void)
{ 
  u8 i;
  f_Acquisition_Task();
  f_mb_tcp_slave_task();

	for(i=0;i<MAXNUM_NODE-1;i++)//最后一个节点固定为其他用途，如版本号，IO控制等
	{
		switch(g_u8_ProtocalID[i])
		{
			case NODE_FX2NPROGRAM:f_Fx2nProgram_task();
					 break;
      case NODE_FX2N485BD:f_Fx2n485bd_task();
					 break;
			case NODE_S7200PPI:f_s7200ppi_task();
					 break;
			case NODE_FPPROGRAM:f_FPprogram_task();
					 break;
			case NODE_CP1LCOM:f_CPcom_task();
					 break;
			case NODE_MODBUSRTU:f_ModbusRTU_task();
					 break;
			case NODE_M70V_NC:f_M70V_NCTCP_task();
					 break;
			case NODE_S7200TCP:f_S7200TCP_Task();
					 break;
			case NODE_S7300TCP:f_S7300TCP_Task();
					 break;
			case NODE_MBTCP:f_mb_tcp_master_task();
					 break;
			case NODE_MITSUBISHIQ:f_MitsubishiQtcp_task();
					 break;
			case NODE_MitsubishiA:f_MitsubishiA_task();
					 break;
			case NODE_FX3UENET_ADP2:f_Fx3uEnet_adp2_task();
					 break;
			case NODE_AB_COMPACTTCP:f_AB_Compact_tcp_task();
			     break;
            case NODE_FX2PROG:f_Fx2Prog_task();
					 break;
			case NODE_AB_Micro232:f_AB_Micro232_task();
			     break;
			case NODE_S7300MPI:f_S7300MPI_task();
			     break;
			case NODE_CP1L_FINS:
			     break;
			case NODE_MITSUBISHIQBIN:f_MitsubishiQBINtcp_task();
					 break;
			case NODE_MITSUBISHIQSERIAL:f_MitsubishiQserial_task();
					 break;
			case NODE_S7400TCP:f_S7400TCP_Task();
					 break;
            case NODE_S1200TCP:f_S1200TCP_Task();
                     break;
            case NODE_SMART200TCP:f_Smart200TCP_Task();
			default:break;
		}
	}

  f_MBTCP_Transmit(TCPcount);
}

