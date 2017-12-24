#include "main.h" 
#include "Task_LED.h"

volatile  Tdef_Byte                          _SystemFlag[NUM_UARTCHANNEL];
#define SystemFlag(n)                        _SystemFlag[n].Byte
#define g_bit_SCIFrameStart(n)               _SystemFlag[n].Bits.bit0    //串口帧起始
#define g_bit_SCIFrameEnd(n)                 _SystemFlag[n].Bits.bit1    //串口帧结束

static u16 g_u16_SCISingalFrameRecTime[NUM_UARTCHANNEL];             //串口单帧数据接收时间计时，单位ms
static u16 g_u16_Message_Length[NUM_UARTCHANNEL];                     //当前已接收到的数据个数
static u8 l_u8_Receive_Buffer[NUM_UARTCHANNEL][SCI_BUF_MAXLEN];          //用于保存串口接收到的数据
static u16 byteidx_faramend[NUM_UARTCHANNEL];//帧结束的字节序号

enum Framestatus_t{
  frame_idle=0,frame_head,frame_data,frame_end,frame_chk  
};
enum Framestatus_t FrameStatus[NUM_UARTCHANNEL];

void CopyRecData(u8 channel);

void USART_Timer1ms(void)
{
    u8 channel;
    
	for(channel=0;channel<NUM_UARTCHANNEL;channel++)
	{
		if(FrameStatus[channel] != frame_idle)
		{
			if(g_u16_SCISingalFrameRecTime[channel]<=0xfff0)
			{
				g_u16_SCISingalFrameRecTime[channel]++;
			}
		}
		else
		{
			g_u16_SCISingalFrameRecTime[channel]=0;
		}
        
        if(g_u16_SCISingalFrameRecTime[channel]>=USARTCAN.tout)
        {
            if(((USARTCAN.UsartProt[channel].FrameEndInfo.T_byte & FrameEndEn) != FrameEndEn)&&(FrameStatus[channel] == frame_data))
            {
                g_u16_SCISingalFrameRecTime[channel]=0;
                FrameStatus[channel] = frame_idle;
                CopyRecData(channel);
            }
            else
            {
                FrameStatus[channel] = frame_idle;
                g_u16_SCISingalFrameRecTime[channel]=0;
                g_u16_Message_Length[channel]=0;
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
        }
	}
}

void CopyRecData(u8 channel)
{
	u8 *start;
	if((USARTCAN.UsartProt[channel].FrameStartInfo.T_byte & FrameStartEn) == FrameStartEn)
	{
		start = &l_u8_Receive_Buffer[channel][USARTCAN.UsartProt[channel].FrameStartInfo.Bits.btn];
		USARTCAN_Recv[channel].lenth = g_u16_Message_Length[channel] + 1 -USARTCAN.UsartProt[channel].FrameStartInfo.Bits.btn;
	}
	else
	{
		start = &l_u8_Receive_Buffer[channel][0];
        USARTCAN_Recv[channel].lenth = g_u16_Message_Length[channel] + 1;
	}
	if((USARTCAN.UsartProt[channel].FrameStartInfo.T_byte & FrameEndEn) == FrameEndEn)
	{
		USARTCAN_Recv[channel].lenth = USARTCAN_Recv[channel].lenth - USARTCAN.UsartProt[channel].FrameEndInfo.Bits.btn;
	}
	else if(USARTCAN.UsartProt[channel].checksum == ChkSum_And)
	{
		USARTCAN_Recv[channel].lenth = USARTCAN_Recv[channel].lenth-2;
	}
	else
	{
		USARTCAN_Recv[channel].lenth = g_u16_Message_Length[channel]+1;
	}
	g_u16_SCISingalFrameRecTime[channel]=0;
    USARTCAN_Recv[channel].datatype = USARTCAN.Usart[channel][uartDatatype];
    memcpy(USARTCAN_Recv[channel].databuf,start,USARTCAN_Recv[channel].lenth);
    memset(l_u8_Receive_Buffer[channel],0,SCI_BUF_MAXLEN);
    USARTCAN_Recv[channel].newupd=ON;
}

void UsartRecieveData(u8 channel,u8 recdata)
{
	u8 Temp=0,Chk=0;
	Temp = (u8) recdata;
    g_u16_SCISingalFrameRecTime[channel]=0;
	if(FrameStatus[channel] == frame_idle)
	{
        g_u16_Message_Length[channel]=0;
		if((USARTCAN.UsartProt[channel].FrameStartInfo.T_byte & FrameStartEn) == FrameStartEn)//有帧头
		{
			if(Temp != USARTCAN.UsartProt[channel].FrameStart[g_u16_Message_Length[channel]])
			{
				return;
			}
            if(USARTCAN.UsartProt[channel].FrameStartInfo.Bits.btn==byte_1)
            {
                FrameStatus[channel] = frame_data;
            }
            else
            {
                FrameStatus[channel] = frame_head;
            }
        }
        else
        {
            FrameStatus[channel] = frame_data;
        }
        l_u8_Receive_Buffer[channel][g_u16_Message_Length[channel]] = Temp;
    }
    else if(FrameStatus[channel] == frame_head)
    {
        g_u16_Message_Length[channel]++;
		l_u8_Receive_Buffer[channel][g_u16_Message_Length[channel]] = Temp;
        if(Temp != USARTCAN.UsartProt[channel].FrameStart[g_u16_Message_Length[channel]])
        {
            FrameStatus[channel] = frame_idle;
            g_u16_SCISingalFrameRecTime[channel]=0;
            g_u16_Message_Length[channel]=0;
            return;
        }
        if((g_u16_Message_Length[channel]+1) == USARTCAN.UsartProt[channel].FrameStartInfo.Bits.btn)
        {
            FrameStatus[channel] = frame_data;
        }
    }
    else if(FrameStatus[channel] == frame_data)
    {
        g_u16_Message_Length[channel]++;
		l_u8_Receive_Buffer[channel][g_u16_Message_Length[channel]] = Temp;
        if((USARTCAN.UsartProt[channel].FrameEndInfo.T_byte & FrameEndEn) == FrameEndEn)
		{
            if(Temp==USARTCAN.UsartProt[channel].FrameEnd[0])
			{
                if(USARTCAN.UsartProt[channel].FrameEndInfo.Bits.btn==byte_1)
                {
                    if(USARTCAN.UsartProt[channel].checksum ==CheckSum_None)
                    {
                        FrameStatus[channel] = frame_idle;
                        CopyRecData(channel);
                    }
                    else
                    {
                        FrameStatus[channel] = frame_chk;//此功能暂未做
                    }
                }
                else
                {
                    byteidx_faramend[channel]=g_u16_Message_Length[channel];
                    FrameStatus[channel] = frame_end;
                }
            }
        }
    }
    else if(FrameStatus[channel] == frame_end)
    {
        g_u16_Message_Length[channel]++;
        l_u8_Receive_Buffer[channel][g_u16_Message_Length[channel]] = Temp;
        if(Temp!=USARTCAN.UsartProt[channel].FrameEnd[g_u16_Message_Length[channel]-byteidx_faramend[channel]])
        {
            FrameStatus[channel] = frame_data;
            byteidx_faramend[channel]=0;
        }
        if((g_u16_Message_Length[channel]-byteidx_faramend[channel]+1) == USARTCAN.UsartProt[channel].FrameEndInfo.Bits.btn)
        {
            if(USARTCAN.UsartProt[channel].checksum ==CheckSum_None)
            {
                FrameStatus[channel] = frame_idle;
                CopyRecData(channel);
            }
            else
            {
                FrameStatus[channel] = frame_chk;//此功能暂未做
            }
        }
    }
    else if(FrameStatus[channel] == frame_data)
    {
        //此功能暂未做
    }
    else
    {
        FrameStatus[channel] = frame_idle;
        byteidx_faramend[channel]=0;
        g_u16_Message_Length[channel]=0;
    }
}

void USART1_IRQ(u8 data)/*  */
{
    
}

void USART2_IRQ(u8 data)/*  */
{
    UsartRecieveData(RS232_1,data);
}

void USART3_IRQ(u8 data)/*  */
{
    UsartRecieveData(RS232_2,data);
}

void UART4_IRQ(u8 data)/*  */
{
    UsartRecieveData(RS485,data);
}

void UART5_IRQ(u8 data)/*  */
{
    
}

#ifdef UART_DMA_ENABLE
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
#endif



void DrUSART1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure; 

    /****************************???USART2??**************************/
    RCC_APB1PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); 
    NVIC_InitTypeDef NVIC_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;//USART1_TX ->PA2           
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);  	   

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_3;//USART1_RX ->PA3           
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //GPIO_Mode_AF_PP  GPIO_Mode_IN_FLOATING
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    if((USARTCAN.Usart[RS232_1][Databits]==0)&&(USARTCAN.Usart[RS232_1][Chkbits]!=0))//7 odd/even
    {
        USARTCAN.Usart[RS232_1][Databits]=0;
    }
    else if((USARTCAN.Usart[RS232_1][Databits]==1)&&(USARTCAN.Usart[RS232_1][Chkbits]==0))//8 none
    {
        USARTCAN.Usart[RS232_1][Databits]=0;
    }
    else if((USARTCAN.Usart[RS232_1][Databits]==1)&&(USARTCAN.Usart[RS232_1][Chkbits]!=0))//8 odd/even
    {
        USARTCAN.Usart[RS232_1][Databits]=1;
    }
    else
    {
        USARTCAN.Usart[RS232_1][Databits]=0;
        USARTCAN.Usart[RS232_1][Chkbits]=0;
    }
    
    
    USART_InitStructure.USART_BaudRate = RS232_baud[USARTCAN.Usart[RS232_1][uartBaudrate]];
    USART_InitStructure.USART_WordLength = RS232_lenth[USARTCAN.Usart[RS232_1][Databits]];//8???
    USART_InitStructure.USART_StopBits = RS232_stop[USARTCAN.Usart[RS232_1][Stopbits]];//1????
    USART_InitStructure.USART_Parity = RS232_parity[USARTCAN.Usart[RS232_1][Chkbits]];//?????
    USART_InitStructure.USART_HardwareFlowControl = RS232_FlowCntl[USARTCAN.Usart[RS232_1][Flowctrl]]; //???????
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //???????
    USART_Init(USART1, &USART_InitStructure); 

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    USART_ClearITPendingBit(USART1, USART_IT_RXNE);//????RX?  
    USART_ClearITPendingBit(USART1, USART_IT_TC);//????TC?  

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//??1????
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=USART1_PRE;//?????3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//????3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ????
    NVIC_Init(&NVIC_InitStructure);	//??????????VIC????

    USART_Cmd(USART1, ENABLE);
}

void DrUSART2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure; 
    #ifdef UART_DMA_ENABLE
    DMA_InitTypeDef   DMA_InitStructure;
    #endif
    u8 databits;
    /****************************???USART2??**************************/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
    RCC_APB2PeriphClockCmd(USART1_GPIO_APB,ENABLE); 
    NVIC_InitTypeDef NVIC_InitStructure;

    GPIO_InitStructure.GPIO_Pin = USART2_RTS_PIN | USART2_TX_PIN;//USART2_TX ->PA2        PA1-RTS   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(USART2_PORT, &GPIO_InitStructure);  	   

    GPIO_InitStructure.GPIO_Pin = USART2_CTS_PIN | USART2_RX_PIN;//USART2_RX ->PA3   PA0-CTS        
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //GPIO_Mode_AF_PP  GPIO_Mode_IN_FLOATING
    GPIO_Init(USART2_PORT, &GPIO_InitStructure);
    
    #ifdef UART_DMA_ENABLE
    /* DMA channel1 configuration ----------------------------------------------*/
    DMA_DeInit(DMA1_Channel7);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(USART2->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel7, &DMA_InitStructure);
    #endif

    if((USARTCAN.Usart[RS232_1][Databits]==1)&&(USARTCAN.Usart[RS232_1][Chkbits]!=0))//7 odd/even
    {
        databits=0;
    }
    else if((USARTCAN.Usart[RS232_1][Databits]==0)&&(USARTCAN.Usart[RS232_1][Chkbits]==0))//8 none
    {
        databits=0;
    }
    else if((USARTCAN.Usart[RS232_1][Databits]==0)&&(USARTCAN.Usart[RS232_1][Chkbits]!=0))//8 odd/even
    {
        databits=1;
    }
    else
    {
        USARTCAN.Usart[RS232_1][Databits]=0;
        USARTCAN.Usart[RS232_1][Chkbits]=0;
    }

    USART_InitStructure.USART_BaudRate = RS232_baud[USARTCAN.Usart[RS232_1][uartBaudrate]];
    USART_InitStructure.USART_WordLength = RS232_lenth[databits];//8???
    USART_InitStructure.USART_StopBits = RS232_stop[USARTCAN.Usart[RS232_1][Stopbits]];//1????
    USART_InitStructure.USART_Parity = RS232_parity[USARTCAN.Usart[RS232_1][Chkbits]];//?????
    USART_InitStructure.USART_HardwareFlowControl = RS232_FlowCntl[USARTCAN.Usart[RS232_1][Flowctrl]]; //???????
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //???????
    USART_Init(USART2, &USART_InitStructure); 

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    USART_ClearITPendingBit(USART2, USART_IT_RXNE);//????RX?  
    USART_ClearITPendingBit(USART2, USART_IT_TC);//????TC?  

    #ifdef UART_DMA_ENABLE
    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel7, DISABLE);
    // USART2 
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART2_PRE;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = USART2_SUB;
    NVIC_Init(&NVIC_InitStructure);
    #endif
    
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//??1????
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=USART2_PRE;//?????3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =USART2_SUB;		//????3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ????
    NVIC_Init(&NVIC_InitStructure);	//??????????VIC????
    
    USART_Cmd(USART2, ENABLE);
    
    databits=' ';
    USART2_Send_Data(&databits,1);
}

void DrUSART3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure; 
    #ifdef UART_DMA_ENABLE
    DMA_InitTypeDef   DMA_InitStructure;
    #endif
    u8 databits;

    /****************************以下为USART3配置**************************/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); 
    NVIC_InitTypeDef NVIC_InitStructure;

    GPIO_InitStructure.GPIO_Pin = USART3_TX_PIN | USART3_RTS_PIN;//USART3_TX ->PB10	         
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(USART3_PORT, &GPIO_InitStructure);		   

    GPIO_InitStructure.GPIO_Pin = USART3_RX_PIN | USART3_CTS_PIN;//USART2_RX ->PB11	        
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
    GPIO_Init(USART3_PORT, &GPIO_InitStructure);
    
    #ifdef UART_DMA_ENABLE
    /* DMA channel1 configuration ----------------------------------------------*/
    DMA_DeInit(DMA1_Channel2);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(USART3->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel2, &DMA_InitStructure);
    #endif
    
    if((USARTCAN.Usart[RS232_2][Databits]==1)&&(USARTCAN.Usart[RS232_2][Chkbits]!=0))//7 odd/even
    {
        databits=0;
    }
    else if((USARTCAN.Usart[RS232_2][Databits]==0)&&(USARTCAN.Usart[RS232_2][Chkbits]==0))//8 none
    {
        databits=0;
    }
    else if((USARTCAN.Usart[RS232_2][Databits]==0)&&(USARTCAN.Usart[RS232_2][Chkbits]!=0))//8 odd/even
    {
        databits=1;
    }
    else
    {
        USARTCAN.Usart[RS232_2][Databits]=0;
        USARTCAN.Usart[RS232_2][Chkbits]=0;
    }

    USART_InitStructure.USART_BaudRate = RS232_baud[USARTCAN.Usart[RS232_2][uartBaudrate]];
    USART_InitStructure.USART_WordLength = RS232_lenth[databits];//8位数据
    USART_InitStructure.USART_StopBits = RS232_stop[USARTCAN.Usart[RS232_2][Stopbits]];//1位停止位
    USART_InitStructure.USART_Parity = RS232_parity[USARTCAN.Usart[RS232_2][Chkbits]];//无奇偶校验
    USART_InitStructure.USART_HardwareFlowControl = RS232_FlowCntl[USARTCAN.Usart[RS232_2][Flowctrl]]; //硬件流控制失能
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //发送和接受使能
    USART_Init(USART3, &USART_InitStructure); 
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_ClearITPendingBit(USART3, USART_IT_TC);//清除中断TC位	
    
    #ifdef UART_DMA_ENABLE
    DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel2, DISABLE);
    // USART3 
    USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART3_PRE;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = USART3_SUB;
    NVIC_Init(&NVIC_InitStructure);
    #endif
    
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//??1????
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=USART3_PRE;//?????3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =USART3_SUB;		//????3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ????
    NVIC_Init(&NVIC_InitStructure);	//??????????VIC????

    USART_Cmd(USART3, ENABLE); 
    
    databits=' ';
    USART3_Send_Data(&databits,1);
}

void DrUART4_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure; 
    #ifdef UART_DMA_ENABLE
    DMA_InitTypeDef   DMA_InitStructure;
    #endif
    NVIC_InitTypeDef NVIC_InitStructure;
    u8 databits;

    /****************************以下为UART4配置**************************/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
    RCC_APB2PeriphClockCmd(UART4_GPIO_APB,ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //TX
    GPIO_InitStructure.GPIO_Pin = UART4_TX_PIN;           
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(UART4_PORT, &GPIO_InitStructure);  	   

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   //RX
    GPIO_InitStructure.GPIO_Pin = UART4_RX_PIN;  
    GPIO_Init(UART4_PORT, &GPIO_InitStructure);
    
    #ifdef UART_DMA_ENABLE
    /* DMA channel1 configuration ----------------------------------------------*/
    DMA_DeInit(DMA2_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(UART4->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA2_Channel5, &DMA_InitStructure);
    #endif
    
    if((USARTCAN.Usart[RS485][Databits]==1)&&(USARTCAN.Usart[RS485][Chkbits]!=0))//7 odd/even
    {
        databits=0;
    }
    else if((USARTCAN.Usart[RS485][Databits]==0)&&(USARTCAN.Usart[RS485][Chkbits]==0))//8 none
    {
        databits=0;
    }
    else if((USARTCAN.Usart[RS485][Databits]==0)&&(USARTCAN.Usart[RS485][Chkbits]!=0))//8 odd/even
    {
        databits=1;
    }
    else
    {
        USARTCAN.Usart[RS485][Databits]=0;
        USARTCAN.Usart[RS485][Chkbits]=0;
    }

    USART_InitStructure.USART_BaudRate = RS232_baud[USARTCAN.Usart[RS485][uartBaudrate]];
    USART_InitStructure.USART_WordLength = RS232_lenth[databits];//8位数据
    USART_InitStructure.USART_StopBits = RS232_stop[USARTCAN.Usart[RS485][Stopbits]];//1位停止位
    USART_InitStructure.USART_Parity = RS232_parity[USARTCAN.Usart[RS485][Chkbits]];//无奇偶校验
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //发送和接受使能
    USART_Init(UART4, &USART_InitStructure);
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_ClearITPendingBit(UART4, USART_IT_TC);//清除中断TC位
    #ifdef UART_DMA_ENABLE
    DMA_ITConfig(DMA2_Channel5, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA2_Channel5, DISABLE);
    // USART2 
    USART_DMACmd(UART4, USART_DMAReq_Tx, ENABLE);
    
    #ifdef STM32F10X_CL 
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel5_IRQn;
    #elif (STM32F10X_HD)
    NVIC_InitStructure.NVIC_IRQChannel =DMA2_Channel4_5_IRQn;
    #else
    
    #endif 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART4_PRE;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART4_SUB;
    NVIC_Init(&NVIC_InitStructure);
    #endif
    
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;//??1????
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=UART4_PRE;//?????3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =UART4_SUB;		//????3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ????
    NVIC_Init(&NVIC_InitStructure);	//??????????VIC????

    USART_Cmd(UART4, ENABLE); 
    
    GPIO_InitStructure.GPIO_Pin = UART4_EN_PIN;//GPIOD->4，RS485方向控制
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(UART4_EN_PORT, &GPIO_InitStructure);

    UART4_485_RX_ENABLE;  	//485接收使能
    
    databits=' ';
    UART4_Send_Data(&databits,1);
}

void DrUART5_Init(void)
{ 
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure; 
	
/****************************以下为UART5配置**************************/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE);
  RCC_APB2PeriphClockCmd(UART5_GPIO_APB,ENABLE);
	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
  GPIO_InitStructure.GPIO_Pin = UART5_TX_PIN;	         
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_Init(UART5_PORT, &GPIO_InitStructure);		   
	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Pin = UART5_RX_PIN;  
  GPIO_Init(UART5_PORT, &GPIO_InitStructure);
	
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;//9位数据
  USART_InitStructure.USART_StopBits = USART_StopBits_1;//1位停止位
  USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //硬件流控制失能
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //发送和接受使能
  USART_Init(UART5, &USART_InitStructure); 	
  USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);

  USART_Cmd(UART5, ENABLE); 
  USART_ClearITPendingBit(UART5, USART_IT_TC);//清除中断TC位
}


void USART1_Send_Data(unsigned char *send_buff,unsigned int length)
{
    unsigned int i = 0;
//  USART1_485_TX_ENABLE;  	//485发送使能
//  delay_us(300);  	//稍作延时，注意延时的长短根据波特率来定，波特率越小，延时应该越长
  for(i = 0;i < length;i ++)
  {  	  
  	USART1->DR = send_buff[i];
  	while((USART1->SR&0X40)==0);  
  }
//  delay_us(50);   	//稍作延时，注意延时的长短根据波特率来定，波特率越小，延时应该越长
//  USART1_485_RX_ENABLE;    	//485接收使能
}

void USART2_Send_Data(unsigned char *send_buff,unsigned int length)
{
#ifdef UART_DMA_ENABLE
    DMA1_Channel7_HW_Start(send_buff,length);
#else
    unsigned int i = 0;
    for(i = 0;i < length;i ++)
    {  	  
        USART2->DR = send_buff[i];
        while((USART2->SR&0X40)==0);  
    }
#endif
}

void USART3_Send_Data(unsigned char *send_buff,unsigned int length)
{
#ifdef UART_DMA_ENABLE
    DMA1_Channel2_HW_Start(send_buff,length);
#else
    unsigned int i = 0;
    for(i = 0;i < length;i ++)
    {  	  
        USART3->DR = send_buff[i];
        while((USART3->SR&0X40)==0);  
    }
#endif
}

void UART4_Send_Data(unsigned char *send_buff,unsigned int length)
{
    unsigned int i = 0;
    UART4_485_TX_ENABLE;
#ifdef UART_DMA_ENABLE
    DMA2_Channel5_HW_Start(send_buff,length);
#else
  for(i = 0;i < length;i ++)
  {  	  
  	UART4->DR = send_buff[i];
  	while((UART4->SR&0X40)==0);  
  }
#endif
}

void UART5_Send_Data(unsigned char *send_buff,unsigned int length)
{
   unsigned int i = 0;
//  UART5_485_TX_ENABLE;  	//485发送使能
//  delay_us(300);  	//稍作延时，注意延时的长短根据波特率来定，波特率越小，延时应该越长
  for(i = 0;i < length;i ++)
  {  	  
  	UART5->DR = send_buff[i];
  	while((UART5->SR&0X40)==0);  
  }
//  delay_us(50);   	//稍作延时，注意延时的长短根据波特率来定，波特率越小，延时应该越长
//  UART5_485_RX_ENABLE;    	//485接收使能
}


void DrUsart_Init(void)
{
    #ifdef UART_DMA_ENABLE
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1|RCC_AHBPeriph_DMA2, ENABLE);
    #endif
    
    if(USARTCAN.Usart[RS232_1][EnUart]==ON)
    {
        DrUSART2_Init();
    }
    if(USARTCAN.Usart[RS232_2][EnUart]==ON)
    {
        DrUSART3_Init();
    }
    if(USARTCAN.Usart[RS485][EnUart]==ON)
    {
        DrUART4_Init();
    }
}

