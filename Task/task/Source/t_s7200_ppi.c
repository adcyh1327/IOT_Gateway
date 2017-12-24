#include <string.h>
#include <stddef.h>
#include "main.h"
#include "t_list_impl.h"

#define BUG()       __breakpoint(0)
/*
enum {
    READ_WAITRESPOND = 0x1,
    READ_RECVSUCCESS = 0x2,
    WRITE_WAITRESPOND = 0x1,
    WRITE_RECVSUCCESS = 0x2,
    Err_MBcmd = 0x4,
};*/
static u8 S7200_ppi=0xff;
static u8 S7200_ppi_Enable=OFF;


void GW_UpdateReadStatus_PPI(uint8_t _status) {}
void GW_UpdateWriteStatus_PPI(uint8_t _status) {}

static void PPI_Fetch(struct PPI_Ctx* _ppi_ctx, uint8_t _plc_addr);
static void PPI_DiscreteRead(struct PPI_Ctx* _ppi_ctx, uint8_t _plc_addr, Data_Entry_Info_t** _infos, uint8_t _entry_num);
static void PPI_Write(struct PPI_Ctx* _ppi_ctx, uint8_t _plc_addr, Data_Entry_Info_t* _info);
static void PPI_HandleReply(struct PPI_Ctx* _ppi_ctx);

const uint8_t ppi_templ[] = {
    0x68, 
    0x1B, 0x1B,         // PDU length +2
    0x68,
    0x02,               // PLC Address +4
    0x00,               // local Address +5
    0x6C,
    0x32,
    0x01,               // req:01, res:03
    0x00, 0x00, 
    0x00, 0x00,         // Counter +12
    0x00, 0x0E,         // 请求信息的长度  +14
    0x00, 0x00,         // 待入写的字节长度 +16
    0x04,               // 操作类型: read:04, write:05 +17
    0x01,               // 请求的数据块个数 +18
    0x12, 0x0A,         // 请求帧头
    0x10,               // +21
    0x04,               // Data Type +22
    0x00, 0x01,         // Data Number +24
    0x00,
    0x01, 0x84,         // Memory Type + 27
    0x00, 0x00, 0x00,   // offset +30
                        // End - Read
    0x00, 0x04,         // bit:03, other:04 +32
    0x00, 0x10,         // Bit Length +34
    0x00, 0x00          // the value be writen
};

const uint8_t ppi_fetch[] = {0x10, 0x02, 0x00, 0x5C, 0x00, 0x16};
const uint8_t ppi_data_type_bit[] = {3, 0, 3, 3, 4, 3, 5, 3};

enum {
    WRITE_TEMPL_LEN = sizeof(ppi_templ),
    READ_TEMPL_LEN = WRITE_TEMPL_LEN - 6,
};

enum {
    TEMPL_OFF_PDU_LEN = 2,
    TEMPL_OFF_PLC_ADDR = 4,
    TEMPL_OFF_CTL_C = 12,
    TEMPL_OFF_CTL_REQ_LEN = 14,
    TEMPL_OFF_CTL_WR_LEN = 16,
    TEMPL_OFF_CTL_MODE = 17,
    TEMPL_OFF_CTL_BLK_NUM = 18,
    TEMPL_OFF_BLK_HDR = 21,
    TEMPL_OFF_DATA_TYPE = 22,
    TEMPL_OFF_DATA_NUM = 24,
    TEMPL_OFF_MEM_TYPE = 27,
    TEMPL_OFF_MEM_OFF = 30,
    TEMPL_OFF_WR_TYPE = 32,
    TEMPL_OFF_WR_BIT_LEN = 34
};

enum {
    _OFF_BLK_HDR = 2,
    _OFF_DATA_TYPE = 3,
    _OFF_DATA_NUM = 5,
    _OFF_MEM_TYPE = 8,
    _OFF_MEM_OFF = 11,
    _OFF_TYPE = 13,
    _OFF_BIT_LEN = 15,
    _OFF_DATA = 16
};

#define RS_485_RX_EN()      GPIO_ResetBits(GPIOD , GPIO_Pin_10)
#define RS_485_TX_EN()      GPIO_SetBits(GPIOD , GPIO_Pin_10)

struct PPI_Ctx ppi_ctx;

///////////////测试样例/////////////////////
enum {READ_ITEM_NUM = 1,WRITE_ITEM_NUM = 1};
uint8_t QXX[32] = {0x3,};
uint32_t I0;
uint32_t Q0;
uint32_t V0;
uint32_t VD4;

Data_Entry_Info_t write_QXX = {
    DT_WORD,
    0,      // bit offset
    0x01,
    MT_Q,
    0,
    (uint8_t*)&QXX[0]
};

Data_Entry_Info_t read_I0 = {
    DT_BIT,
    0,      // bit offset
    0x1,
    MT_I,
    0,
    (uint8_t*)&I0,
};

Data_Entry_Info_t read_Q0 = {
    DT_BYTE,
    0,      // bit offset      
    0x3,
    MT_Q,
    0,
    (uint8_t*)&Q0,
};

Data_Entry_Info_t read_V0 = {
    DT_WORD,
    0,      // bit offset
    0x1,
    MT_V,
    0,
    (uint8_t*)&V0,
};

Data_Entry_Info_t read_VD4 = {
    DT_DWORD,
    0,      // bit offset
    0x1,
    MT_V,
    0,
    (uint8_t*)&VD4,
};

Data_Entry_Info_t read_reg = {
    DT_BYTE,
    0,      // bit offset
    0x1,
    MT_V,
    0,
    (uint8_t*)&V0,
};

Data_Entry_Info_t* write_grp[WRITE_ITEM_NUM] = {
    &write_QXX,
};

Data_Entry_Info_t* read_grp[READ_ITEM_NUM] = {
    &read_reg,
    //&read_Q0,
    //&read_V0,
    //&read_VD4
};

struct ppi_request_block req_read_grp = {
    LIST_HEAD_INIT(req_read_grp.entry),
    OP_READ,
    0,
    RF_ONCE,
    READ_ITEM_NUM,
    read_grp
};

struct ppi_request_block req_write_QXX = {
    LIST_HEAD_INIT(req_write_QXX.entry),
    OP_WRITE,
    0,
    RF_ONCE,
    0x1,
    (Data_Entry_Info_t**)&write_QXX
};

void BSP_Ser_StartSend(void)
{
    //RS_485_TX_EN();
    //USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
    DAQ_UartSend(ppi_ctx.buff,ppi_ctx.len,CHN_UART_CFG);
    ppi_ctx.timer = ppi_ctx.guard_time;
    ppi_ctx.rxLen = 0xffff;
    ppi_ctx.rxIndex = 0;
}

void PPI_AddReqBlock_Front(struct PPI_Ctx* _ppi_ctx, struct ppi_request_block* _req)
{
    __disable_irq();
    if (_req->opType == OP_WRITE)
        _req->infoNum = 1;
    list_add(&_req->entry, &_ppi_ctx->head);
    __enable_irq();
}

void PPI_AddReqBlock(struct PPI_Ctx* _ppi_ctx, struct ppi_request_block* _req)
{
    __disable_irq();
    if (_req->opType == OP_WRITE)
        _req->infoNum = 1;
    list_add_tail(&_req->entry, &_ppi_ctx->head);
    __enable_irq();
}

static struct ppi_request_block* PPI_DelReqBlock(struct PPI_Ctx* _ppi_ctx)
{
    struct ppi_request_block* req = 0;
    
    __disable_irq();
    if (!list_empty(&_ppi_ctx->head))
    {
        req = (struct ppi_request_block*)_ppi_ctx->head.next;
			  if (req)
						list_del(_ppi_ctx->head.next);
    }
    __enable_irq();
    
    return req;
}

void PPI_Init(void)
{
    RS_485_RX_EN();
    
    memset(&ppi_ctx, 0, sizeof(ppi_ctx));
    ppi_ctx.event = EV_RX_OK;
    ppi_ctx.addr = 0;
    ppi_ctx.fsm_state = FSM_REQ;
    ppi_ctx.guard_time = 100;
    ppi_ctx.delay_time = 10;
    ppi_ctx.interval_time = 50;
    
    // 
    INIT_LIST_HEAD(&ppi_ctx.head);
    //PPI_AddReqBlock(&ppi_ctx, &req_read_grp);
   // PPI_AddReqBlock(&ppi_ctx, &req_write_QXX);
}


void PPI_LED_Indicate(void)
{
    static uint8_t flip;
    if (flip & 0x1)
    {
        //LED1_ON;
    }
    else
    {
        //LED1_OFF;
    }
        
    flip ^= 0x1;
}

void PPI_CyclicTask(void)
{
    if (ppi_ctx.event == EV_NONE)
        return;

    switch (ppi_ctx.fsm_state)
    {
    default:
    case FSM_REQ:
				ppi_ctx.curr = PPI_DelReqBlock(&ppi_ctx);
				if (ppi_ctx.curr == 0)
            return;
        
        ppi_ctx.curr->result = 0;
        if (ppi_ctx.curr->opType == OP_WRITE)
        {
            //Data_Entry_Info_t* write_req = ppi_ctx.curr->infos[0]; 
            //QXX[0] ^= 0x2;
            PPI_Write(&ppi_ctx, RegisterCfgBuff[S7200_ppi][g_u8_RespondID].plcaddrstation, &write_QXX);
            //GW_UpdateWriteStatus_PPI(WRITE_WAITRESPOND);
        }
        else
        {
            PPI_DiscreteRead(&ppi_ctx, RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].plcaddrstation,
														ppi_ctx.curr->infos, ppi_ctx.curr->infoNum);
            //GW_UpdateReadStatus_PPI(READ_WAITRESPOND);
        }
        
        PPI_LED_Indicate();

        BSP_Ser_StartSend();
        ppi_ctx.fsm_next_state = FSM_WAIT_ACK;
        break;
    
    case FSM_WAIT_ACK:
        if ((ppi_ctx.event != EV_TO) && (ppi_ctx.rxLen == 1) && (ppi_ctx.rxBuff[0] == 0xE5))
        {
            ppi_ctx.fsm_next_state = FSM_FETCH;
            ppi_ctx.timer = ppi_ctx.delay_time;
        }
        else
        {
            ppi_ctx.curr->result = 0x80;
            ppi_ctx.fsm_next_state = FSM_REQ;
            // 请求超时
            ppi_ctx.timer = 1000;
            if (ppi_ctx.curr->flags & RF_SOLID)
                PPI_AddReqBlock(&ppi_ctx, ppi_ctx.curr);
        }
        
        break;
    
    case FSM_FETCH:
        PPI_Fetch(&ppi_ctx, RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].plcaddrstation);
        BSP_Ser_StartSend();
        ppi_ctx.fsm_next_state = FSM_WAIT_DATA;
        break;
    
    case FSM_WAIT_DATA:
        if ((ppi_ctx.event != EV_TO) && (ppi_ctx.rxLen <= 261))
        {
            PPI_HandleReply(&ppi_ctx);
        }
        else
            ppi_ctx.curr->result = 0x80;
            
        if (ppi_ctx.curr->flags & RF_SOLID)
            PPI_AddReqBlock(&ppi_ctx, ppi_ctx.curr);
        ppi_ctx.timer = ppi_ctx.interval_time;
        ppi_ctx.fsm_next_state = FSM_REQ;
        break;
    }
    
    // 错误处理
    if (ppi_ctx.curr->result)
        NegativeResponse(Err_MBcmd);
    
    ppi_ctx.fsm_state = ppi_ctx.fsm_next_state;
    ppi_ctx.event = EV_NONE;
}

uint8_t grace_count = 0;
// 定时器需要每隔1毫秒调用该函数一次
void PPI_TimerHandler(void)
{
    // 和比它优先级高的中断共用一个变量，需要保护
    __disable_irq();
    if (ppi_ctx.timer && (!--ppi_ctx.timer))
    {
			  // 若接收缓冲区中已经有一些数据，再宽限100个毫秒
			  if ((ppi_ctx.rxIndex) && (grace_count == 0)) 
				{
					  grace_count++;
					  ppi_ctx.timer = 100;
        }
				else
				{
            ppi_ctx.event = EV_TO;
					  grace_count = 0;
				}
    }
    __enable_irq();
}

static uint8_t sum_up(uint8_t* _buff, uint16_t _len)
{
    uint16_t i;
    uint8_t sum = 0;
    
    for (i =0; i < _len; i++) sum += _buff[i];
    
    return sum;
}

void PPI_Fetch(struct PPI_Ctx* _ppi_ctx, uint8_t _plc_addr)
{
    memcpy(&_ppi_ctx->buff[0], &ppi_fetch[0], sizeof(ppi_fetch));
    _ppi_ctx->buff[4] = _ppi_ctx->buff[1] + _ppi_ctx->buff[2] + _ppi_ctx->buff[3];
    _ppi_ctx->len = sizeof(ppi_fetch);
    _ppi_ctx->index = 0;
}

void PPI_DiscreteRead(struct PPI_Ctx* _ppi_ctx, 
                      uint8_t _plc_addr, 
                      Data_Entry_Info_t** _infos, uint8_t _entry_num)
{
    uint32_t bit_off;
    uint16_t off;
    uint8_t i;
    
    memcpy(&_ppi_ctx->buff[0], &ppi_templ[0], TEMPL_OFF_DATA_TYPE);
    _ppi_ctx->buff[TEMPL_OFF_PLC_ADDR]    = _plc_addr;
    _ppi_ctx->buff[TEMPL_OFF_CTL_C - 1]   = _ppi_ctx->counter;
    _ppi_ctx->buff[TEMPL_OFF_CTL_C]       = _ppi_ctx->counter++;
    _ppi_ctx->buff[TEMPL_OFF_CTL_REQ_LEN] = 2 + 12 * _entry_num;
    _ppi_ctx->buff[TEMPL_OFF_CTL_BLK_NUM] = _entry_num;
    off = TEMPL_OFF_BLK_HDR - 2;
    for (i = 0; i < _entry_num; i++)
    {
        bit_off = _infos[i]->memOff << 3;
        if (_infos[i]->dataType == DT_BIT)
        {
            bit_off += _infos[i]->bitOff;
        }
        
        _ppi_ctx->buff[off + _OFF_BLK_HDR - 2]  = 0x12;
        _ppi_ctx->buff[off + _OFF_BLK_HDR - 1]  = 0x0A;
        _ppi_ctx->buff[off + _OFF_BLK_HDR]      = 0x10;
        
        _ppi_ctx->buff[off + _OFF_DATA_TYPE]    = _infos[i]->dataType;
        
        _ppi_ctx->buff[off + _OFF_DATA_NUM - 1] = 0;
        _ppi_ctx->buff[off + _OFF_DATA_NUM]     = _infos[i]->dataNum;
        
        _ppi_ctx->buff[off + _OFF_MEM_TYPE - 2] = 0;
        _ppi_ctx->buff[off + _OFF_MEM_TYPE - 1] = _infos[i]->memType >> 8;
        _ppi_ctx->buff[off + _OFF_MEM_TYPE]     = _infos[i]->memType;
        
        _ppi_ctx->buff[off + _OFF_MEM_OFF - 2]  = bit_off >> 16;
        _ppi_ctx->buff[off + _OFF_MEM_OFF - 1]  = bit_off >> 8;
        _ppi_ctx->buff[off + _OFF_MEM_OFF]      = bit_off;
        
        off += 12;
    }
    
    _ppi_ctx->buff[TEMPL_OFF_PDU_LEN - 1] = off - 4;
    _ppi_ctx->buff[TEMPL_OFF_PDU_LEN]     = _ppi_ctx->buff[TEMPL_OFF_PDU_LEN - 1];
    _ppi_ctx->buff[off] = sum_up(&_ppi_ctx->buff[4], off - 4);
    _ppi_ctx->buff[off + 1] = 0x16;
    _ppi_ctx->len = off + 2;
    _ppi_ctx->index = 0;
}

void PPI_Write(struct PPI_Ctx* _ppi_ctx, uint8_t _plc_addr, Data_Entry_Info_t* _info)
{
    uint32_t bit_off = _info->memOff << 3;
    uint16_t bit_len = _info->dataNum << ppi_data_type_bit[_info->dataType & 0x7];
    uint16_t len = 4 + ((bit_len + 7) >> 3);  // 所有的长度对齐到一个字节
    
    if (_info->dataType == DT_BIT)
    {
        bit_off += _info->bitOff;
    }
    
    memcpy(&_ppi_ctx->buff[0], &ppi_templ[0], WRITE_TEMPL_LEN);
    _ppi_ctx->buff[TEMPL_OFF_PLC_ADDR]  = _plc_addr;
    _ppi_ctx->buff[TEMPL_OFF_CTL_C - 1] = _ppi_ctx->counter;
    _ppi_ctx->buff[TEMPL_OFF_CTL_C]     = _ppi_ctx->counter++;
    
    _ppi_ctx->buff[TEMPL_OFF_CTL_WR_LEN - 1] = len >> 8;
    _ppi_ctx->buff[TEMPL_OFF_CTL_WR_LEN]     = len;
    // 写操作
    _ppi_ctx->buff[TEMPL_OFF_CTL_MODE]     = OP_WRITE;
    
    _ppi_ctx->buff[TEMPL_OFF_DATA_TYPE]    = _info->dataType;
    _ppi_ctx->buff[TEMPL_OFF_DATA_NUM]     = _info->dataNum;
    _ppi_ctx->buff[TEMPL_OFF_MEM_TYPE - 1] = _info->memType >> 8;
    _ppi_ctx->buff[TEMPL_OFF_MEM_TYPE]     = _info->memType;
    _ppi_ctx->buff[TEMPL_OFF_MEM_OFF - 2]  = bit_off >> 16;
    _ppi_ctx->buff[TEMPL_OFF_MEM_OFF - 1]  = bit_off >> 8;
    _ppi_ctx->buff[TEMPL_OFF_MEM_OFF]      = bit_off;
    // 数据类型冗余
    _ppi_ctx->buff[TEMPL_OFF_WR_TYPE -1]   = 0;
    if (_info->dataType == DT_BIT)
    {
        _ppi_ctx->buff[TEMPL_OFF_WR_TYPE] = 0x03;
    }
    else
    {
        _ppi_ctx->buff[TEMPL_OFF_WR_TYPE] = 0x04;
    }
    // 所写位的总长度
    _ppi_ctx->buff[TEMPL_OFF_WR_BIT_LEN - 1] = bit_len >> 8;
    _ppi_ctx->buff[TEMPL_OFF_WR_BIT_LEN] = bit_len;
    
    len = (bit_len + 7) >> 3;
    
    _ppi_ctx->buff[TEMPL_OFF_PDU_LEN - 1] = TEMPL_OFF_WR_BIT_LEN + 1 + len - 4;
    _ppi_ctx->buff[TEMPL_OFF_PDU_LEN]     = _ppi_ctx->buff[TEMPL_OFF_PDU_LEN - 1];
    // 复制要写的数据
    memcpy(&_ppi_ctx->buff[TEMPL_OFF_WR_BIT_LEN+1], _info->mappedBuff, len);
    len += TEMPL_OFF_WR_BIT_LEN + 1;
    _ppi_ctx->buff[len] = sum_up(&_ppi_ctx->buff[4], len - 4);
    _ppi_ctx->buff[len + 1] = 0x16;
    _ppi_ctx->len = len + 2;
    _ppi_ctx->index = 0;
}

// PPI不支持一次写多个区域!!! -- 2015/9/30
void PPI_HandleReadReply(struct PPI_Ctx* _ppi_ctx, uint8_t _num)
{
    uint8_t i;
    uint8_t err = 0;
    uint16_t byte_len;
    uint8_t* buff = &_ppi_ctx->rxBuff[21];
    struct ppi_request_block* req = _ppi_ctx->curr;
    
    // 若数据长度不够，就不要更新数据
    if (buff[-2] < 4)
        return;

    for (i = 0; i < _num; i++)
    {
        switch (buff[0])
        {
        case 0xff:
            byte_len = ((buff[2] << 8) + buff[3] + 7) >> 3;
            memcpy(req->infos[i]->mappedBuff, &buff[4], byte_len);
            if((RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regtype=='I')||
                (RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regtype=='Q')||
                (RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regtype=='S')||
                (RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regtype=='M'))
            {
              req->infos[i]->mappedBuff[1]=req->infos[i]->mappedBuff[0];
              req->infos[i]->mappedBuff[0]=0x00;
            }
            byte_len = (byte_len + 1) & ~0x1;
            buff += byte_len;
            break;
            
        default:
        case 0x03:
            req->result |= 1 << i;
            err = 1;
						break;
        }
        buff += 4;
    }
    
    if (!err)
    {
        _ppi_ctx->curr->result = 0;
        GW_ReadStatus(S7200_ppi)=(READ_RECVSUCCESS);
        g_u16_RecvTransLen[S7200_ppi][g_u8_threadIdx[S7200_ppi]]=
        RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].datalenth * 2;
    }
    else
    {
      g_u16_RecvTransLen[S7200_ppi][g_u8_threadIdx[S7200_ppi]]=0;
    }
}

void PPI_HandleWriteReply(struct PPI_Ctx* _ppi_ctx)
{
    if (_ppi_ctx->rxBuff[21] == 0xff)
    {
        _ppi_ctx->curr->result = 0;
        GW_WriteStatus(S7200_ppi)=WRITE_RECVSUCCESS;
        memcpy(&g_u8_EthRespData[S7200_ppi][(g_u16_StartAddr[S7200_ppi][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
    }
}

void PPI_HandleReply(struct PPI_Ctx* _ppi_ctx)
{
    // 默认情况下，响应的报文存放在rxBuff中
    uint8_t cksum;
    uint8_t* rx_buff = _ppi_ctx->rxBuff;
    
    _ppi_ctx->curr->result = 0x80;
    
    if (_ppi_ctx->rxLen < 6)
    {
        return;
    }
    
    cksum = sum_up(&rx_buff[4], _ppi_ctx->rxLen - 6);
    if (cksum != rx_buff[_ppi_ctx->rxLen - 2])
    {
        return;
    }
    
    // 检验和通过
    if (rx_buff[8] != 0x03)
    {
        return;
    }
    
    // 是响应
    if (rx_buff[19] == 0x04)
    {
        PPI_HandleReadReply(_ppi_ctx, rx_buff[20]);
    }
    else if (rx_buff[19] == 0x05)
    {
        PPI_HandleWriteReply(_ppi_ctx);
    }
}

void _USART3_IRQHandler(void)
{
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    ppi_ctx.rxBuff[ppi_ctx.rxIndex] = USART3->DR;
    ppi_ctx.rxIndex++;
    
    //ppi_ctx.timer = 0; 		// 2016-3-12 删除
    // 若出现仅仅发几个字节的情况怎么办?
    if ((ppi_ctx.rxIndex == 1) && (ppi_ctx.rxBuff[0] == 0xE5))
    {
        ppi_ctx.event = EV_RX_OK;
			  ppi_ctx.rxLen = 1;
				ppi_ctx.timer = 0;	// 2016-3-12 增加
    }
    else if (ppi_ctx.rxIndex == 3)
    {
        ppi_ctx.rxLen = ppi_ctx.rxBuff[1] + 6;
    }
    else if (ppi_ctx.rxIndex >= ppi_ctx.rxLen)
    {
			  grace_count = 0;
        ppi_ctx.event = EV_RX_OK;
				ppi_ctx.timer = 0;	// 2016-3-12 增加
    }
    else
    {

    }
	}
    
	if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET) 
	{
        USART_ClearITPendingBit(USART3, USART_IT_TXE);
        if (ppi_ctx.index < ppi_ctx.len)
        {
            USART3->DR = ppi_ctx.buff[ppi_ctx.index];
            ppi_ctx.index++;
        }
        else
        {
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
            USART_ITConfig(USART3, USART_IT_TC, ENABLE);
        }
    }
    
    if (USART_GetITStatus(USART3, USART_IT_TC) != RESET)
    {
        USART_ITConfig(USART3, USART_IT_TC, DISABLE);
        RS_485_RX_EN();
        
        ppi_ctx.timer = ppi_ctx.guard_time;
        ppi_ctx.rxLen = 0xffff;
        ppi_ctx.rxIndex = 0;
    }
}

void S7200PPI_Init(u8 nodeID)
{
  S7200_ppi=nodeID;
  PPI_Init();
  S7200_ppi_Enable=ON;
}

void UART_S7200ppi_Recv(u8 data)
{
  ppi_ctx.rxBuff[ppi_ctx.rxIndex] = data;
  ppi_ctx.rxIndex++;
  
  //ppi_ctx.timer = 0; 		// 2016-3-12 删除
  // 若出现仅仅发几个字节的情况怎么办?
  if ((ppi_ctx.rxIndex == 1) && (ppi_ctx.rxBuff[0] == 0xE5))
  {
      ppi_ctx.event = EV_RX_OK;
			ppi_ctx.rxLen = 1;
			ppi_ctx.timer = 0;	// 2016-3-12 增加
  }
  else if (ppi_ctx.rxIndex == 3)
  {
      ppi_ctx.rxLen = ppi_ctx.rxBuff[1] + 6;
  }
  else if (ppi_ctx.rxIndex >= ppi_ctx.rxLen)
  {
      ppi_ctx.event = EV_RX_OK;
			ppi_ctx.timer = 0;	// 2016-3-12 增加
  }
  else
  {

  }
}

void TIM_1ms_S7200PPI(void)
{
  PPI_TimerHandler();
  if(S7200_ppi_Enable==ON)
  {
 	 g_u16_SwitchTimer[S7200_ppi]++; 
  }
  if((ON==MB_NeedWrite(S7200_ppi))&&(READ_IDLE==GW_ReadStatus(S7200_ppi)))
	{	
    if(GW_WriteStatus(S7200_ppi)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[S7200_ppi]>1000)
	     {
	     		g_u16_SwitchTimer[S7200_ppi]=0;
					GW_WriteStatus(S7200_ppi)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(S7200_ppi)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[S7200_ppi]>WriteTime))
	     {	
		   		GW_WriteStatus(S7200_ppi)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[S7200_ppi]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(S7200_ppi)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(S7200_ppi)=WRITE_DELAY;
      g_u16_SwitchTimer[S7200_ppi]=0;
	  }
	  else if(g_u16_SwitchTimer[S7200_ppi]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[S7200_ppi]=0;
      GW_WriteStatus(S7200_ppi)=WRITE_IDLE;
	    MB_NeedWrite(S7200_ppi)=OFF;
	    g_u8_threadIdx[S7200_ppi]++;
		  ThreadNew(S7200_ppi)=ON;
	  }
	  else{

	  }
	}
	else
	{	
		if((g_u16_SwitchTimer[S7200_ppi]>=UpdateCycle[S7200_ppi])&&
		    (g_u8_ProtocalNum[S7200_ppi][READ]!=0))
		{	  
		  g_u16_SwitchTimer[S7200_ppi]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(S7200_ppi))
		  {
		    GW_ReadStatus(S7200_ppi)=READ_IDLE;
		    g_u16_TimeoutCnt[S7200_ppi][g_u8_threadIdx[S7200_ppi]]=0;
		    if(OFF==MB_NeedWrite(S7200_ppi))
		    {
          g_u8_threadIdx[S7200_ppi]++;
          while(RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[S7200_ppi]++;
            if(g_u8_ProtocalNum[S7200_ppi][READ]<=g_u8_threadIdx[S7200_ppi])
      			{			  
      	      g_u8_threadIdx[S7200_ppi]=0;
      			}
          }
		      ThreadNew(S7200_ppi)=ON;
		    }
		    else
		    {
          
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[S7200_ppi][g_u8_threadIdx[S7200_ppi]]++;
        if(g_u16_TimeoutCnt[S7200_ppi][g_u8_threadIdx[S7200_ppi]]>=
          (THRES_TIMOUTCNT/UpdateCycle[S7200_ppi]))
        {
          g_u16_RecvTransLen[S7200_ppi][g_u8_threadIdx[S7200_ppi]]=0;
          g_u16_TimeoutCnt[S7200_ppi][g_u8_threadIdx[S7200_ppi]]=0;
          if(OFF==MB_NeedWrite(S7200_ppi))
  		    {
            g_u8_threadIdx[S7200_ppi]++;
            while(RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[S7200_ppi]++;
              if(g_u8_ProtocalNum[S7200_ppi][READ]<=g_u8_threadIdx[S7200_ppi])
        			{			  
        	      g_u8_threadIdx[S7200_ppi]=0;
        			}
            }
  		      ThreadNew(S7200_ppi)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(S7200_ppi)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[S7200_ppi][READ]<=g_u8_threadIdx[S7200_ppi])
	{			  
		g_u8_threadIdx[S7200_ppi]=0;
	}
}

void f_s7200ppi_task(void)
{
  u8 datatyppe;
  u16 dataaddr;
  #if 1
  if(0!=ThreadNew(S7200_ppi))
  {
    ThreadNew(S7200_ppi)=0;
    ppi_ctx.fsm_state = FSM_REQ;
    GW_ReadStatus(S7200_ppi)=READ_WAITRESPOND;
    switch(RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regtype)
    {
      case 'I':
        read_reg.memType=MT_I;
        read_reg.dataType=DT_BIT;
        read_reg.memOff=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[6];
        read_reg.bitOff=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[7];
        read_reg.dataNum=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].datalenth;
        break;
      case 'Q':
        read_reg.memType=MT_Q;
        read_reg.dataType=DT_BIT;
        read_reg.memOff=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[6];
        read_reg.bitOff=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[7];
        read_reg.dataNum=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].datalenth;
        break;
      case 'M':
        read_reg.memType=MT_M;
        read_reg.dataType=DT_BIT;
        read_reg.memOff=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[6];
        read_reg.bitOff=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[7];
        read_reg.dataNum=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].datalenth;
        break;
      case 'S':
        read_reg.memType=MT_S;
        read_reg.dataType=DT_BIT;
        read_reg.memOff=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[6];
        read_reg.bitOff=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[7];
        read_reg.dataNum=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].datalenth;
        break;
      case 'V':
        read_reg.memType=MT_V;
        read_reg.dataType=DT_BYTE;
        read_reg.memOff=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[6]*256+
                        RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[7];
        read_reg.dataNum=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].datalenth*2;
        break;
      case 'C':
        read_reg.memType=MT_C;
        read_reg.dataType=DT_BYTE;
        read_reg.memOff=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[6]*256+
                        RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[7];
        read_reg.dataNum=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].datalenth*2;
        break;
      case 'T':
        read_reg.memType=MT_T;
        read_reg.dataType=DT_BYTE;
        read_reg.memOff=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[6]*256+
                        RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].regaddr[7];
        read_reg.dataNum=RegisterCfgBuff[S7200_ppi][g_u8_threadIdx[S7200_ppi]].datalenth*2;
        break;
      default:break;
    }
    read_reg.mappedBuff=&g_u8_EthRespData[S7200_ppi][g_u16_StartAddr[S7200_ppi][g_u8_threadIdx[S7200_ppi]]*2];
    PPI_AddReqBlock(&ppi_ctx, &req_read_grp);
  }
  else if(WRITE_PRESEND==GW_WriteStatus(S7200_ppi))
  {		   
    GW_WriteStatus(S7200_ppi)=WRITE_WAITRESPOND;
    //PositiveResponse();
    //f_MBTCP_Transmit(data);
    ppi_ctx.fsm_state = FSM_REQ;
    switch(RegisterCfgBuff[S7200_ppi][g_u8_RespondID].regtype)
    {
      case 'I':
        write_QXX.memType=MT_I;
        write_QXX.dataType=DT_BIT;
        write_QXX.memOff=g_u8_WriteAddr[6];
        write_QXX.bitOff=g_u8_WriteAddr[7];
        write_QXX.dataNum=g_u16_WriteLen;
        g_u8_Writedata[0]=g_u8_Writedata[1];
        break;
      case 'Q':
        write_QXX.memType=MT_Q;
        write_QXX.dataType=DT_BIT;
        write_QXX.memOff=g_u8_WriteAddr[6];
        write_QXX.bitOff=g_u8_WriteAddr[7];
        write_QXX.dataNum=g_u16_WriteLen;
        g_u8_Writedata[0]=g_u8_Writedata[1];
        break;
      case 'M':
        write_QXX.memType=MT_M;
        write_QXX.dataType=DT_BIT;
        write_QXX.memOff=g_u8_WriteAddr[6];
        write_QXX.bitOff=g_u8_WriteAddr[7];
        write_QXX.dataNum=g_u16_WriteLen;
        g_u8_Writedata[0]=g_u8_Writedata[1];
        break;
      case 'S':
        write_QXX.memType=MT_S;
        write_QXX.dataType=DT_BIT;
        write_QXX.memOff=g_u8_WriteAddr[6];
        write_QXX.bitOff=g_u8_WriteAddr[7];
        write_QXX.dataNum=g_u16_WriteLen;
        g_u8_Writedata[0]=g_u8_Writedata[1];
        break;
      case 'V':
        write_QXX.memType=MT_V;
        write_QXX.dataType=DT_BYTE;
        write_QXX.memOff=(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
        write_QXX.dataNum=g_u16_WriteLen*2;
        break;
      case 'C':
        write_QXX.memType=MT_C;
        write_QXX.dataType=DT_BYTE;
        write_QXX.memOff=(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
        write_QXX.dataNum=g_u16_WriteLen*2;
        break;
      case 'T':
        write_QXX.memType=MT_T;
        write_QXX.dataType=DT_BYTE;
        write_QXX.memOff=(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
        write_QXX.dataNum=g_u16_WriteLen*2;
        break;
      default:break;
    }
    memcpy(&QXX,&g_u8_Writedata,write_QXX.dataNum);
    PPI_AddReqBlock(&ppi_ctx, &req_write_QXX);
  }
  else
  {

  }
  #endif
  PPI_CyclicTask();
  
}

