#include "main.h" 

const struct CAN_CFG CAN_CFG_t[CAN_BAUDCFG_NUM] =//共计14种波特率配置
{
	1000,{3,CAN_BS1_8tq,CAN_BS2_3tq},//波特率 分频因子 时间缓冲段1 时间缓冲段2
	800,{3,CAN_BS1_11tq,CAN_BS2_3tq},
	666,{6,CAN_BS1_6tq,CAN_BS2_2tq},
	500,{6,CAN_BS1_8tq,CAN_BS2_3tq},
	400,{6,CAN_BS1_12tq,CAN_BS2_2tq},
	250,{9,CAN_BS1_13tq,CAN_BS2_2tq},
	200,{9,CAN_BS1_16tq,CAN_BS2_3tq},
	125,{18,CAN_BS1_13tq,CAN_BS2_2tq},
	100,{45,CAN_BS1_6tq,CAN_BS2_1tq},
	80,{56,CAN_BS1_6tq,CAN_BS2_1tq},
	50,{90,CAN_BS1_6tq,CAN_BS2_1tq},
	40,{100,CAN_BS1_7tq,CAN_BS2_1tq},
	20,{360,CAN_BS1_3tq,CAN_BS2_1tq},
	10,{720,CAN_BS1_3tq,CAN_BS2_1tq},
};

void DrCAN1_Init(void)
{
	GPIO_InitTypeDef		GPIO_InitStructure;
	CAN_InitTypeDef			CAN_InitStructure;
    CAN_FilterInitTypeDef	CAN_FilterInitStructure;
    NVIC_InitTypeDef        NVIC_InitStructure;
// 	CAN_FilterInitTypeDef	CAN_FilterInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	RCC_APB2PeriphClockCmd(CAN_GPIO_APB, ENABLE);
    
	/* Configure CAN pin: RX */
	GPIO_InitStructure.GPIO_Pin = CAN_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAN_PORT, &GPIO_InitStructure);
	/* Configure CAN pin: TX */					
	GPIO_InitStructure.GPIO_Pin = CAN_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		        
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAN_PORT, &GPIO_InitStructure);
    GPIO_PinRemapConfig(GPIO_Remap2_CAN1, ENABLE);

	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);

	CAN_InitStructure.CAN_TTCM=DISABLE;		
	CAN_InitStructure.CAN_ABOM=ENABLE;			  
	CAN_InitStructure.CAN_AWUM=DISABLE;			  
	CAN_InitStructure.CAN_NART=DISABLE;			   
	CAN_InitStructure.CAN_RFLM=DISABLE;			  
	CAN_InitStructure.CAN_TXFP=DISABLE;			 
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	
	
	if(USARTCAN.can[canBaudrate]>CAN_BAUDCFG_NUM)
	{
		CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;		   
		CAN_InitStructure.CAN_BS1=CAN_CFG_t[3].bs_cfg.bs1;		 
		CAN_InitStructure.CAN_BS2=CAN_CFG_t[3].bs_cfg.bs2;		 
		CAN_InitStructure.CAN_Prescaler = CAN_CFG_t[3].bs_cfg.brp;;	//8±íê?500K
	}
    else
    {
        CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;		   
        CAN_InitStructure.CAN_BS1=CAN_CFG_t[USARTCAN.can[canBaudrate]].bs_cfg.bs1;		 
        CAN_InitStructure.CAN_BS2=CAN_CFG_t[USARTCAN.can[canBaudrate]].bs_cfg.bs2;		 
        CAN_InitStructure.CAN_Prescaler = CAN_CFG_t[USARTCAN.can[canBaudrate]].bs_cfg.brp;;	//8±íê?500K
    }
	
	CAN_Init(CAN1, &CAN_InitStructure);	
#if 0
	CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdList;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_16bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = CMD_ID << 5;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x00;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0xffff;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0xffff;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
#endif
    
    CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x00;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x00;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
    
	/* Enable FIFO 0 message pending Interrupt */
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN1_RX0_PRE;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = CAN1_RX0_SUB;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


u8 CAN1_SendData(u16 id, u8 *data,u8 len)
{
	CanTxMsg	TxMessage;
    u8 ret;
	u8 counting = 0;
	TxMessage.StdId =id;			//取值范围为0~0x7FF
	TxMessage.IDE =CAN_ID_STD;		//
 	TxMessage.Rtr =CAN_RTR_DATA;	//
	TxMessage.DLC=len;				//
	for(counting=0;counting<len;counting++)
	{
		TxMessage.Data[counting]=data[counting];
	}

	if(CAN_Transmit(CAN1, &TxMessage) == CAN_TxStatus_NoMailBox)
		ret = FALSE;
	else
		ret = TRUE;
    return ret;
}

void CAN1_IRQ(CanRxMsg* RxMessage)
{
    USARTCAN_Recv[CAN_CHN].lenth=RxMessage->DLC+2;//多出来的2表示canid
    USARTCAN_Recv[CAN_CHN].datatype = USARTCAN.can[canDatatype];
    USARTCAN_Recv[CAN_CHN].databuf[0] = (RxMessage->StdId)>>8;
    USARTCAN_Recv[CAN_CHN].databuf[1] = (RxMessage->StdId)&0xff;
    memcpy(&USARTCAN_Recv[CAN_CHN].databuf[2],RxMessage->Data,RxMessage->DLC);
    USARTCAN_Recv[CAN_CHN].newupd=ON;
}


void DrCAN_Init(void)
{
    if(USARTCAN.can[EnCAN]==ON)
    {
        DrCAN1_Init();
    }
}
