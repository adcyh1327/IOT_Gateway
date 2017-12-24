/******************** (C) COPYRIGHT 2013 **************************
 * 文件名  ：mb.c
 * 描述    ：STM32 Modbus 从机程序         
 * 实验平台：STM32F103VCT6+W5500 开发板
 * 库版本  ：ST3.5.0
 *
 * 作者    ：Zhangsz
 * 编写日期：2013-12-06
**********************************************************************************/

/* ----------------------- System includes ----------------------------------*/
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "MJ_stm32_platform.h"
/* ----------------------- Modbus includes ----------------------------------*/	
#include "tcp_slave_mb.h"
#include "tcp_slave_mbconfig.h"
#include "tcp_slave_mbframe.h"
#include "tcp_slave_mbfunc.h"
#include "tcp_slave_mbport.h"
#include "m_spi_mb_tcp_master.h"

#include "m_w5500.h"

#if MB_RTU_ENABLED == 1
#include "mbrtu.h"
#endif
#if MB_ASCII_ENABLED == 1
#include "mbascii_slave.h"
#endif
#if MB_TCP_ENABLED == 1
#include "tcp_slave_mbtcp.h"
#endif



#ifndef MB_PORT_HAS_CLOSE
#define MB_PORT_HAS_CLOSE 0
#endif

/* Functions pointer which are initialized in eMBInit( ). Depending on the
 * mode (RTU or ASCII) the are set to the correct implementations.
 */

       u8    aucTCPBuf[263]; 
	   u16   usTCPBufLen;
	   u8    needSendLen;	 
	   u16    needSendAddr;
/* Callback functions required by the porting layer. They are called when
 * an external event has happend which includes a timeout or the reception
 * or transmission of a character.
 */
u8( *pxMBFrameCBByteReceived ) ( void );
u8( *pxMBFrameCBTransmitterEmpty ) ( void );
u8( *pxMBPortCBTimerExpired ) ( void );

u8( *pxMBFrameCBReceiveFSMCur ) ( void );
u8( *pxMBFrameCBTransmitFSMCur ) ( void );

/* An array of Modbus functions handlers which associates Modbus function
 * codes with implementing functions.
 */

eMBErrorCode eTCP_SlaveMBPoll( u8 socket )
{
	u8 i;
	u16 usTCPDataLen;
	i=SlaveRead_1_Byte(SIR);
	if(i&0x01)
	{
		i=SlaveRead_SOCK_1_Byte(0,Sn_IR);	//读W5500中断
		SlaveWrite_SOCK_1_Byte(0,Sn_IR,i);
		if(i&IR_RECV)
		{															  
			usTCPBufLen	= SlaveRead_SOCK_Data_Buffer(0, aucTCPBuf);	//读新数据放到aucTCPBuf中
			if(usTCPBufLen==0)//如果接受数据长度为0，则返回
			{ 			 			  
        return FALSE;
			}
			else//正常流程
			{
        if(aucTCPBuf[6] == GatewayStationAddr)
        {
          usTCPDataLen=(aucTCPBuf[4]<<8)+aucTCPBuf[5];
          TCPcount[0]=aucTCPBuf[0];
          TCPcount[1]=aucTCPBuf[1];
          for(i=0;i<usTCPDataLen;i++)
          {
            g_u8_EthRecvData[i] = aucTCPBuf[6+i];
          }
          TCP_Recv_EthData(socket,g_u8_EthRecvData,usTCPDataLen);
        }
        else
        {
          
        }
			}
		}	
    if(i&IR_DISCON)		/* TCP Disconnect */
    {
      SlaveSocket_Listen(0);		//重新监听SOCK 0
   	}			               
		if(i&IR_TIMEOUT)		   //超时
		{
      SlaveSocket_Listen(0);		//重新监听SOCK 0
		}
  }
  else if(i&0x02)
  {
		i=SlaveRead_SOCK_1_Byte(1,Sn_IR);	//读W5500中断
		SlaveWrite_SOCK_1_Byte(1,Sn_IR,i);
		if(i&IR_RECV)
		{															  
			usTCPBufLen	= SlaveRead_SOCK_Data_Buffer(1, aucTCPBuf);	//读新数据放到aucTCPBuf中
			if(usTCPBufLen==0)//如果接受数据长度为0，则返回
			{ 			 			  
        return FALSE;
			}
			else//正常流程
			{
        if(aucTCPBuf[6] == GatewayStationAddr)
        {
          usTCPDataLen=(aucTCPBuf[4]<<8)+aucTCPBuf[5];
          TCPcount[0]=aucTCPBuf[0];
          TCPcount[1]=aucTCPBuf[1];
          for(i=0;i<usTCPDataLen;i++)
          {
            g_u8_EthRecvData[i] = aucTCPBuf[6+i];
          }
          TCP_Recv_EthData(socket,g_u8_EthRecvData,usTCPDataLen);
        }
        else
        {
          
        }
			}
		}	
    if(i&IR_DISCON)		/* TCP Disconnect */
    {
      SlaveSocket_Listen(1);		//重新监听SOCK 0
   	}			               
		if(i&IR_TIMEOUT)		   //超时
		{
      SlaveSocket_Listen(1);		//重新监听SOCK 0
		}
  }
	else if(i&0x40)//bootloader
  {
		i=SlaveRead_SOCK_1_Byte(6,Sn_IR);	//读W5500中断
		SlaveWrite_SOCK_1_Byte(6,Sn_IR,i);
		if(i&IR_RECV)
		{															  
			usTCPBufLen	= SlaveRead_SOCK_Data_Buffer(6, aucTCPBuf);	//读新数据放到aucTCPBuf中
			if(usTCPBufLen==0)//如果接受数据长度为0，则返回
			{ 			 			  
				return FALSE;
			}
			else//正常流程
			{
				if((0x00==aucTCPBuf[4])&&(0x0B==aucTCPBuf[5])&&(0x10==aucTCPBuf[7])&& (0x55==aucTCPBuf[8])&&(0x55==aucTCPBuf[9])&&  
						(0x02==aucTCPBuf[11])&&(0x04==aucTCPBuf[12])&&(0x55==aucTCPBuf[13])&& (0x55==aucTCPBuf[14])&&(0x55==aucTCPBuf[15])&&(0x55==aucTCPBuf[16]))
				{			    
					SlaveWrite_SOCK_Data_Buffer(6,aucTCPBuf,usTCPBufLen);		
					if(Flash_Erase(APP_CRC_ADDR,1) != FLASH_COMPLETE)
					{
						return 0;
					}
					f_GenSoftwareReset();
				}	       
			}
		}	
		if(i&IR_DISCON)		/* TCP Disconnect */
		{
			SlaveSocket_Listen(6);		//重新监听SOCK 0
		}			               
		if(i&IR_TIMEOUT)		   //超时
		{
			SlaveSocket_Listen(6);		//重新监听SOCK 0
		}
	}
  else
  {

  }
  return i;
}


