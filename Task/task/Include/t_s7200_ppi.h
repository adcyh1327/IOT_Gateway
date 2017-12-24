#ifndef __T_PPI_H
#define __T_PPI_H

#include <stdint.h>

#include "t_list.h"

enum Data_Type_t {
    DT_BIT = 0x1,
    DT_BYTE = 0x2,
    DT_WORD = 0x4,
    DT_DWORD = 0x6
};

enum Mem_Type_t {
    MT_S = 0x4,
    MT_SM = 0x5,
    MT_AI = 0x6,
    MT_AQ = 0x7,
    MT_C = 0x1E,
    MT_I = 0x81,
    MT_Q = 0x82,
    MT_M = 0x83,
    MT_V = 0x0184,
    MT_T = 0x1F,
};

enum OP_Type {
    OP_READ = 0x4,
    OP_WRITE = 0x5,
};

struct Data_Entry_Info {
    enum Data_Type_t    dataType;
    uint8_t             bitOff;
    uint16_t            dataNum;
    enum Mem_Type_t     memType;
    uint32_t            memOff;
    uint8_t*            mappedBuff;
};

typedef struct Data_Entry_Info Data_Entry_Info_t;

enum {
    RF_ONCE = 0x00,
    RF_SOLID = 0x10,
};

struct ppi_request_block {
    struct list_head entry;
    enum OP_Type opType;
    uint8_t result;
    uint8_t flags;
    uint8_t infoNum;
    Data_Entry_Info_t** infos;
};

enum PPI_FSM {
    FSM_REQ,
    FSM_WAIT_ACK,
    FSM_FETCH,
    FSM_WAIT_DATA
};

enum PPI_EVENT {
    EV_NONE,
    EV_RX_OK,
    EV_TO,
	  EV_REQ
};

enum {PPI_BUFF_LEN = 272};

struct PPI_Ctx {
    volatile enum PPI_EVENT event;
    uint8_t addr;
    volatile enum PPI_FSM fsm_state;
    volatile enum PPI_FSM fsm_next_state;
    //
    struct list_head head;
    struct ppi_request_block* curr;
    
    uint8_t counter;
    volatile uint16_t timer;
    
    uint16_t guard_time;
    uint16_t delay_time;
    uint16_t interval_time;
    
    uint16_t len;
    uint16_t index; 
    uint8_t  buff[PPI_BUFF_LEN];
    
    uint16_t rxLen;
    uint16_t rxIndex;
    uint8_t  rxBuff[PPI_BUFF_LEN];
};

void S7200PPI_Init(u8 nodeID);
void TIM_1ms_S7200PPI(void);
void UART_S7200ppi_Recv(u8 data);
void f_s7200ppi_task(void);

void PPI_AddReqBlock(struct PPI_Ctx* _ppi_ctx, struct ppi_request_block* _req);
void PPI_AddReqBlock_Front(struct PPI_Ctx* _ppi_ctx, struct ppi_request_block* _req);

#endif // __T_PPI_H

