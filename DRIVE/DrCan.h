#ifndef __DR_CAN_H
#define __DR_CAN_H

#include "main.h"

#define CAN_BAUDCFG_NUM       14U

struct CAN_BS
{
	u16 brp;
	u8 bs1;
	u8 bs2;
};

typedef struct CAN_BS CAN_BS_t;

struct CAN_CFG
{
	u16 baudrate;
	CAN_BS_t bs_cfg;
};

u8 CAN1_SendData(u16 id, u8 *data,u8 len);
void CAN1_IRQ(CanRxMsg* RxMessage);
void DrCAN_Init(void);

#endif


