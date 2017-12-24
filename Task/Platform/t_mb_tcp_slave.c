#include "main.h"
#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"	 
#include "m_spi_mb_tcp_slave.h"	  
#include "m_spi_mb_tcp_slave_cfg.h"
#include "tcp_slave_mb.h"
#include "m_w5500.h"

#define REG_HOLDING_START 1000
#define REG_HOLDING_NREGS 10
vu32 TimeDelayCount;
static unsigned short usRegHoldingStart = REG_HOLDING_START;
static unsigned short usRegHoldingBuf[REG_HOLDING_NREGS]={0xaa,0x55,0x5599,0x5588};		
/* ----------------------- input register Defines ------------------------------------------*/
#define REG_INPUT_START 1000
#define REG_INPUT_NREGS 10	 
/* ----------------------- Static variables ---------------------------------*/
static unsigned short usRegInputStart = REG_INPUT_START;
static unsigned short usRegInputBuf[REG_INPUT_NREGS]={0,0,0x5599,0x5588};	   
/* ----------------------- coils register Defines ------------------------------------------*/
#define REG_COILS_START   1000
#define REG_COILS_SIZE    16		 
/* ----------------------- discrete register Defines ------------------------------------------*/
#define REG_DISC_START   1000
#define REG_DISC_SIZE    16	 
u8 ucTCPSlaveIntFlag;
		 
/* W5500 configuration */

eMBErrorCode eTCP_SlaveMBRegHoldingCB( u8 * pucRegBuffer, u16 usAddress, u16 usNRegs,eMBRegisterMode eMode )
{
  eMBErrorCode  eStatus = MB_ENOERR;
  int         iRegIndex;  
  if( ( usAddress >= REG_HOLDING_START ) &&( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
  {
    iRegIndex = ( int )( usAddress - usRegHoldingStart );
    switch ( eMode )
    {
      /* Pass current register values to the protocol stack. */
      case MB_REG_READ:
        while( usNRegs > 0 )
        {
          *pucRegBuffer++ =( unsigned char )( usRegHoldingBuf[iRegIndex] >> 8 );
          *pucRegBuffer++ =( unsigned char )( usRegHoldingBuf[iRegIndex] &0xFF );
          iRegIndex++;
          usNRegs--;
        }
      break;
	  /* Update current register values with new values from the * protocol stack. */
      case MB_REG_WRITE:
        while( usNRegs > 0 )
        {
          usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
          usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
          iRegIndex++;
          usNRegs--;
        } 
      break;
    }
  }
  else
  {
    eStatus = MB_ENOREG;
  }
  return eStatus;
}

eMBErrorCode eTCP_SlaveMBRegInputCB( u8 * pucRegBuffer, u16 usAddress, u16 usNRegs )
{
  eMBErrorCode  eStatus = MB_ENOERR;
  int         iRegIndex;  
  if( ( usAddress >= REG_INPUT_START )&& ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
  {
    iRegIndex = ( int )( usAddress - usRegInputStart );
    while( usNRegs > 0 )
    {
      *pucRegBuffer++ =( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
      *pucRegBuffer++ =( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
      iRegIndex++;
      usNRegs--;
    }
  }
  else
  {
    eStatus = MB_ENOREG;
  }	
  return eStatus;
}

/********************* EXTI4_IRQHandler**********************************/
void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line0);
		ucTCPSlaveIntFlag=1;
	}
}

void f_mb_tcp_slave_task(void)
{    
  u8 i,*p;

  if(g_u16_ETH_Timeout >= 3000)
  {
    g_u16_ETH_Timeout=0;
    SlaveWrite_SOCK_1_Byte(0,Sn_CR,CLOSE);
  }
  
	switch(SlaveRead_SOCK_1_Byte(0,Sn_SR))
	{
		case SOCK_CLOSED:
			 SlaveWrite_SOCK_1_Byte(0,Sn_CR,OPEN);	  
			 while(SlaveRead_SOCK_1_Byte(0,Sn_CR));
		break;

		case SOCK_INIT:
			SlaveSocket_Listen(0);
		break;

		case SOCK_LISTEN:
		break;

		case SOCK_ESTABLISHED:
		   g_u16_ETH_Timeout=0;
			 eTCP_SlaveMBPoll(0);
		break;
		
		case SOCK_CLOSE_WAIT:
			 SlaveWrite_SOCK_1_Byte(0,Sn_CR,CLOSE);
		     while(SlaveRead_SOCK_1_Byte(0,Sn_CR));
		break;
		
		case SOCK_UDP:
		break;

		case SOCK_MACRAW:
 
 		break;
		case SOCK_SYNSEND:
		break;
		
		case SOCK_SYNRECV:
		break;
		
		case SOCK_FIN_WAI:
		break;

		case SOCK_CLOSING:
		break;
		
		case SOCK_TIME_WAIT:
		break;

		case SOCK_LAST_ACK:
		break;

		default:
		break;
	}
	switch(SlaveRead_SOCK_1_Byte(1,Sn_SR))
	{
		case SOCK_CLOSED:
			 SlaveWrite_SOCK_1_Byte(1,Sn_CR,OPEN);	  
			 while(SlaveRead_SOCK_1_Byte(1,Sn_CR));
		break;

		case SOCK_INIT:
			SlaveSocket_Listen(1);
		break;

		case SOCK_LISTEN:
		break;

		case SOCK_ESTABLISHED:
			 eTCP_SlaveMBPoll(1);
		break;
		
		case SOCK_CLOSE_WAIT:
			 SlaveWrite_SOCK_1_Byte(1,Sn_CR,CLOSE);
		     while(SlaveRead_SOCK_1_Byte(1,Sn_CR));
		break;
		
		case SOCK_UDP:
		break;

		case SOCK_MACRAW:
 
 		break;
		case SOCK_SYNSEND:
		break;
		
		case SOCK_SYNRECV:
		break;
		
		case SOCK_FIN_WAI:
		break;

		case SOCK_CLOSING:
		break;
		
		case SOCK_TIME_WAIT:
		break;

		case SOCK_LAST_ACK:
		break;

		default:
		break;
	}
	switch(SlaveRead_SOCK_1_Byte(6,Sn_SR))
	{
		case SOCK_CLOSED:
			 SlaveWrite_SOCK_1_Byte(6,Sn_CR,OPEN);	  
			 while(SlaveRead_SOCK_1_Byte(6,Sn_CR));
		break;

		case SOCK_INIT:
			SlaveSocket_Listen(6);
		break;

		case SOCK_LISTEN:
		break;

		case SOCK_ESTABLISHED:
			 eTCP_SlaveMBPoll(6);
		break;
		
		case SOCK_CLOSE_WAIT:
			 SlaveWrite_SOCK_1_Byte(6,Sn_CR,CLOSE);
		     while(SlaveRead_SOCK_1_Byte(6,Sn_CR));
		break;
		
		case SOCK_UDP:
		break;

		case SOCK_MACRAW:
 
 		break;
		case SOCK_SYNSEND:
		break;
		
		case SOCK_SYNRECV:
		break;
		
		case SOCK_FIN_WAI:
		break;

		case SOCK_CLOSING:
		break;
		
		case SOCK_TIME_WAIT:
		break;

		case SOCK_LAST_ACK:
		break;

		default:
		break;
	}
	switch(SlaveRead_SOCK_1_Byte(7,Sn_SR))
	{
		case SOCK_CLOSED:
			 SlaveWrite_SOCK_1_Byte(7,Sn_CR,OPEN);	  
			 while(SlaveRead_SOCK_1_Byte(7,Sn_CR));
		break;

		case SOCK_INIT:
			SlaveSocket_Listen(7);
		break;

		case SOCK_LISTEN:
		break;

		case SOCK_ESTABLISHED:
			 eTCP_SlaveMBPoll(7);
		break;
		
		case SOCK_CLOSE_WAIT:
			 SlaveWrite_SOCK_1_Byte(7,Sn_CR,CLOSE);
		     while(SlaveRead_SOCK_1_Byte(7,Sn_CR));
		break;
		
		case SOCK_UDP:
		  i=SlaveRead_1_Byte(SIR);

    	if(i&0x80)
    	{
    		i=SlaveRead_SOCK_1_Byte(7,Sn_IR);	//读W5500中断
    		SlaveWrite_SOCK_1_Byte(7,Sn_IR,i);
    		if(i&IR_RECV)
				{
// 					p=(u8 *)MODBUS_CONFIG_BASEADDR;
// 					usTCPBufLen	= SlaveRead_SOCK_Data_Buffer(6, aucTCPBuf);
	//         for(i=0;i<24;i++)
	//         {
	//           g_u8_EthTransData[8+i]=(u8)(*(u16 *)p);
	//           p++;
	//         }
// 					memcpy(g_u8_EthTransData, p, 24);	
// 					memcpy(g_u8_EthTransData+8, p, 24);			
// 					//Write_SOCK_Data_Buffer(s, Tx_Buffer, size);
// 					SlaveWrite_SOCK_Data_Buffer(7,g_u8_EthTransData,24);
				}
      }
		  break;

		case SOCK_MACRAW:
 
 		break;
		case SOCK_SYNSEND:
		break;
		
		case SOCK_SYNRECV:
		break;
		
		case SOCK_FIN_WAI:
		break;

		case SOCK_CLOSING:
		break;
		
		case SOCK_TIME_WAIT:
		break;

		case SOCK_LAST_ACK:
		break;

		default:
		break;
	}
}
void Delay1ms(uint32_t nTime)
{
  TimeDelayCount = nTime;
  //使能系统滴答定时器
  while(TimeDelayCount>0);
}

