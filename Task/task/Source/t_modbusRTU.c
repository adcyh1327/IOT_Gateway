#include "main.h"
#include "stm32f10x_crc.h" 

#define MODBUSRTU_SEND_LENGTH             120          //发送缓冲区大小
#define MODBUSRTU_RECE_LENGTH             120         //接受缓冲区大小

static u8 modbusrtu_sendbuf[MODBUSRTU_SEND_LENGTH];
u8 middle_array[MODBUSRTU_RECE_LENGTH]     ={0};
u8 FH_Receive            = 0;		//帧头标志
u8 Receive_Count         = 0;
u8 Recevie_End           = 0;
//u8 Recevie_OK          = 0;
u8 Verify_OK             = 0;
u8 FH_One                = 1;
u8 Time6_Ok              = 0;
u8 Num_a				 = 0;
u8 FH_Flage				 = 0;            
volatile u8 TaiDa_Adder = 0;
u8 ModbusRTU_485=0xff;
u8 ModbusRTU_Enable=OFF;
//-------------------------------------/..地址..//..长度..//..校验../---------
u8 modbusrtu_sendbuf[MODBUSRTU_SEND_LENGTH] = {0x01,0x03,0x01,0x42,0x00,0x02,0x00,0x00};
//CRC高位字节值表 
const u8 auchCRCHi[] = { 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40 
}; 
//CRC 低位字节值表 
const u8 auchCRCLo[]={ 
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 
0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 
0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 
0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 
0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 
0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 
0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 
0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 
0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 
0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 
0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 
0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 
0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 
0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 
0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 
0x43, 0x83, 0x41, 0x81, 0x80, 0x40 
}; 

void ModbusRTU_Init(u8 nodeID)
{
//  middle_array[500]     ={0};
//  electricity_meter[500]={0};
  FH_Receive            = 0;		//帧头标志
  Receive_Count         = 0;
  Recevie_End           = 0;
  Verify_OK             = 0;
  FH_One                = 1;
  Time6_Ok              = 0;
  Num_a				    = 0;
  FH_Flage				= 0;            
  TaiDa_Adder           = 0;
  //r485_Flage            = 0;	  
  ModbusRTU_485=nodeID;
	ModbusRTU_Enable=ON;
}

//======================================================================
//获得CRC16值 
//puchMsg:要校验的数组 
//usDataLen:数组长度 
//======================================================================
u16 Get_Crc16(u8 *puchMsg,u16 usDataLen) 
{ 
u8 uchCRCHi=0xFF; 		//高CRC 字节初始化 
u8 uchCRCLo=0xFF; 		//低CRC 字节初始化  
u32 uIndex; 	 		//CRC 循环中的索引 
	while(usDataLen--) 	//传输消息缓冲区 
	{ 
		uIndex=uchCRCHi^*puchMsg++; //计算CRC  
		uchCRCHi=uchCRCLo^auchCRCHi[uIndex]; 
		uchCRCLo=auchCRCLo[uIndex]; 
	} 
	return (uchCRCHi<<8|uchCRCLo); 
} 
//======================================================================
//CRC8校验 
//ptr:要校验的数组 
//len:数组长度 
//返回值:CRC8码 
//======================================================================
u8 Get_Crc8(u8 *ptr,u16 len) 
{ 
	u8 crc; 
	u8 i; 
	crc=0; 
	while(len--) 
	{ 
		crc^=*ptr++; 
		for(i=0;i<8;i++) 
		{ 
			if(crc&0x01)
			{
				crc=(crc>>1)^0x8C; 
			}
			else crc >>= 1; 
		} 
	} 
	return crc; 
} 
//=====================================================================
void UART_ModbusRTU_Recv(unsigned char  l_u8ReceData)
{
	u16 CrcCheck;
	u8 i,j;
	//接收
	if((RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].plcaddrstation == l_u8ReceData) && (FH_One == 1))
	{	
		FH_One = 0;
		Receive_Count = 0;
		middle_array[Receive_Count] = l_u8ReceData;
		FH_Receive = 1;
	}

	else if(FH_Receive == 1)
	{
		middle_array[Receive_Count] = l_u8ReceData;	

	}
	else
	{
		FH_One    = 1;
		FH_Receive = 0;
		Receive_Count = 0;
	}

	//读操作
	if(GW_ReadStatus(ModbusRTU_485)==READ_WAITRESPOND)	
	{
		Recevie_End = 0;

		if((middle_array[2]+5)==(Receive_Count+1))
		{
			CrcCheck = Get_Crc16(middle_array,(middle_array[2]+3));
			if(((CrcCheck>>8) == middle_array[Receive_Count-1]) && ((CrcCheck%256) == middle_array[Receive_Count]))
			{
	      if((middle_array[1]&FUNC_RESP_NEG)!=FUNC_RESP_NEG)
	      {
          if((RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].specfuncode==FUNC_RD_COILSTATUS)||
              (RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].specfuncode==FUNC_RD_INPUTSTATUS))
          {
            for(i=0;i<middle_array[2];i++)
            {
              for(j=0;j<8;j++)
              {
                g_u8_EthRespData[ModbusRTU_485][(g_u16_StartAddr[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]]*2)+(8*2*i)+(2*j)]=0x00;
                g_u8_EthRespData[ModbusRTU_485][(g_u16_StartAddr[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]]*2)+(8*2*i)+(2*j)+1]=
                  (middle_array[3+i]>>j)&0x01;
              }
            }
            
  	  			GW_ReadStatus(ModbusRTU_485)=READ_RECVSUCCESS;
  	        g_u16_RecvTransLen[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]]= 
  	          RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].datalenth*2;
          }
          else
          {
  	  			memcpy(&g_u8_EthRespData[ModbusRTU_485][g_u16_StartAddr[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]]*2],
  	  							&middle_array[3],RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].datalenth*2);
  	  			GW_ReadStatus(ModbusRTU_485)=READ_RECVSUCCESS;
  	        g_u16_RecvTransLen[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]]= 
  	          RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].datalenth*2;
  	      }
	      }
	      else
	      {
	        GW_ReadStatus(ModbusRTU_485)=READ_RECVSUCCESS;
	        g_u16_RecvTransLen[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]]= 0;
	      }
			}
			FH_One    = 1;
			FH_Receive = 0;
			Receive_Count = 0;
			return;
		}
		else if((middle_array[2]+5)<(Receive_Count+1))
		{
			FH_One    = 1;
			FH_Receive = 0;
			Receive_Count = 0;
			return;
		}
		else
		{
      
		}
	}
	else if((Receive_Count==7)&&(GW_WriteStatus(ModbusRTU_485)==WRITE_WAITRESPOND))
	{//写操作
		if((middle_array[1]&FUNC_RESP_NEG)!=FUNC_RESP_NEG)
		{
			GW_WriteStatus(ModbusRTU_485)=WRITE_RECVSUCCESS;
			//if(Write_FuncCode == FUNC_WR_MULREG)
      {
        memcpy(&g_u8_EthRespData[ModbusRTU_485][(g_u16_StartAddr[ModbusRTU_485][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
      }
      //else if(Write_FuncCode == FUNC_WR_SGCOIL)
      //{
      //  g_u8_EthRespData[ModbusRTU_485][(g_u16_StartAddr[ModbusRTU_485][g_u8_RespondID]+
			//	        g_u8_WriteAddrOffset)*2]=0x00;
			//	g_u8_EthRespData[ModbusRTU_485][(g_u16_StartAddr[ModbusRTU_485][g_u8_RespondID]+
			//	        g_u8_WriteAddrOffset)*2+1]=g_u8_Writedata[1]&0x01;
      //}
      //else
      //{

      //}
		}
		else
		{
			NegativeResponse(middle_array[2]);
		}
		FH_One    = 1;
		Receive_Count = 0;
		return;
	}
	else
	{
	    
		//Recevie_End = 0;
  }


	Receive_Count++;
// 	if (Receive_Count > 9)
// 	{
// 		FH_One    = 1;
// 		Receive_Count = 0;	
// 	}
}

//=====================================================================
void ModbusRTU_Read(u8 spec_code,u8 lenth)
{
	u16 CrcCheck;
	u8 func_code,index=0;
	func_code=((spec_code&0x0f)==0)? FUNC_RD_HOLDREG : (spec_code&0x0f) ;
	modbusrtu_sendbuf[index++] =  RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].plcaddrstation;
	modbusrtu_sendbuf[index++] =  func_code;
	modbusrtu_sendbuf[index++] =  RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].regaddr[6];
	modbusrtu_sendbuf[index++] =  RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].regaddr[7];
  modbusrtu_sendbuf[index++] =  lenth/256;
	modbusrtu_sendbuf[index++] =  lenth%256;
  /*计算Crc校验*/
	CrcCheck = Get_Crc16(modbusrtu_sendbuf,index);
	modbusrtu_sendbuf[index++] =  CrcCheck>>8;
	modbusrtu_sendbuf[index++] =  CrcCheck;
  DAQ_UartSend(modbusrtu_sendbuf,index,spec_code>>4);//高4位用来指定串口通道，实现不同端口用同个协议
}

void ModbusRTU_Write(u8 spec_code,u16 addr,u8 lenth)
{
	u16 CrcCheck;
	u8 func_code,index=0,i,temp;
  func_code=((spec_code&0x0f)==0)? FUNC_WR_MULREG : (spec_code&0x0f) ;
	modbusrtu_sendbuf[index++] =  RegisterCfgBuff[ModbusRTU_485][g_u8_RespondID].plcaddrstation;
	modbusrtu_sendbuf[index++] =  func_code;
	modbusrtu_sendbuf[index++] =  addr/256;
	modbusrtu_sendbuf[index++] =  addr%256;
	
  if(func_code == FUNC_WR_SGCOIL)
  {
    temp=2;
    g_u8_Writedata[0]=(g_u8_Writedata[1]==0x01)?0xff:0x00;
    g_u8_Writedata[1]=0x00;
  }
	else if(func_code == FUNC_WR_MULCOIL)
  {
    temp = (lenth/8)+((lenth%8)==0?0:1);
  }
  else if(func_code == FUNC_WR_MULREG)
  {
    modbusrtu_sendbuf[index++] =  lenth/256;
    modbusrtu_sendbuf[index++] =  lenth%256;
    temp = lenth * 2;
    modbusrtu_sendbuf[index++] = temp;
  }
  else
  {
    temp = lenth * 2;
  }
  for(i=0;i<temp;i++)
	{
    modbusrtu_sendbuf[index++] =  g_u8_Writedata[i];
	}
	
  /*计算Crc校验*/
	CrcCheck = Get_Crc16(modbusrtu_sendbuf,index);
	modbusrtu_sendbuf[index++] =  CrcCheck>>8;
	modbusrtu_sendbuf[index++] =  CrcCheck;
  DAQ_UartSend(modbusrtu_sendbuf,index,spec_code>>4);
}

void TIM_1ms_ModbusRTU(void)
{
	if(ModbusRTU_Enable==ON)
	{
		g_u16_SwitchTimer[ModbusRTU_485]++; 
	}
  if((ON==MB_NeedWrite(ModbusRTU_485))&&(READ_IDLE==GW_ReadStatus(ModbusRTU_485)))
	{
    g_u16_SwitchTimer[ModbusRTU_485]++; 	
    if(GW_WriteStatus(ModbusRTU_485)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[ModbusRTU_485]>1000)
	     {
	     		g_u16_SwitchTimer[ModbusRTU_485]=0;
					GW_WriteStatus(ModbusRTU_485)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(ModbusRTU_485)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[ModbusRTU_485]>WriteTime))
	     {	
		   		GW_WriteStatus(ModbusRTU_485)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[ModbusRTU_485]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(ModbusRTU_485)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(ModbusRTU_485)=WRITE_DELAY;
      g_u16_SwitchTimer[ModbusRTU_485]=0;
	  }
	  else if(g_u16_SwitchTimer[ModbusRTU_485]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[ModbusRTU_485]=0;
      GW_WriteStatus(ModbusRTU_485)=WRITE_IDLE;
	    MB_NeedWrite(ModbusRTU_485)=OFF;
			ThreadNew(ModbusRTU_485)=ON;
	  }
	  else{

	  }
	}
	else
	{	
		if((g_u16_SwitchTimer[ModbusRTU_485]>UpdateCycle[ModbusRTU_485])&&
		    (g_u8_ProtocalNum[ModbusRTU_485][READ]!=0))
		{	  
		  g_u16_SwitchTimer[ModbusRTU_485]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(ModbusRTU_485))
		  {
		    GW_ReadStatus(ModbusRTU_485)=READ_IDLE;
		    g_u16_TimeoutCnt[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]]=0;
		    if(OFF==MB_NeedWrite(ModbusRTU_485))
		    {
          g_u8_threadIdx[ModbusRTU_485]++;
          while(RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[ModbusRTU_485]++;
            if(g_u8_ProtocalNum[ModbusRTU_485][READ]<=g_u8_threadIdx[ModbusRTU_485])
      			{			  
      	      g_u8_threadIdx[ModbusRTU_485]=0;
      			}
          }
		      ThreadNew(ModbusRTU_485)=ON;
		    }
		    else
		    {
          
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]]++;
        if(g_u16_TimeoutCnt[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]]>
					(THRES_TIMOUTCNT/UpdateCycle[ModbusRTU_485]))
        {
          g_u16_RecvTransLen[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]]=0;
          g_u16_TimeoutCnt[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]]=0;
          if(OFF==MB_NeedWrite(ModbusRTU_485))
  		    {
            g_u8_threadIdx[ModbusRTU_485]++;
            while(RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[ModbusRTU_485]++;
              if(g_u8_ProtocalNum[ModbusRTU_485][READ]<=g_u8_threadIdx[ModbusRTU_485])
        			{			  
        	      g_u8_threadIdx[ModbusRTU_485]=0;
        			}
            }
  		      ThreadNew(ModbusRTU_485)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(ModbusRTU_485)=READ_IDLE;
  		    }
        }
		  }
		  
		}
	}
	if(g_u8_ProtocalNum[ModbusRTU_485][READ]<=g_u8_threadIdx[ModbusRTU_485])
	{			  
		g_u8_threadIdx[ModbusRTU_485]=0;
	}
}


void f_ModbusRTU_task(void)
{				
  u8 datatyppe;
  u16 dataaddr;
  if(0!=ThreadNew(ModbusRTU_485))
  {
    ThreadNew(ModbusRTU_485)=0;
    GW_ReadStatus(ModbusRTU_485)=READ_WAITRESPOND;
    /*dataaddr=(RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].regaddr[6]<<8)
      + RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].regaddr[7];*/
    ModbusRTU_Read(RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].specfuncode,
                   RegisterCfgBuff[ModbusRTU_485][g_u8_threadIdx[ModbusRTU_485]].datalenth);
  }
  else if(WRITE_PRESEND==GW_WriteStatus(ModbusRTU_485))
  {		   
    GW_WriteStatus(ModbusRTU_485)=WRITE_WAITRESPOND;
    dataaddr=(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
    switch(RegisterCfgBuff[ModbusRTU_485][g_u8_RespondID].specfuncode)
    {
      case FUNC_RD_COILSTATUS:  
            ModbusRTU_Write(FUNC_WR_SGCOIL,dataaddr,g_u16_WriteLen);
            break;
      case FUNC_RD_HOLDREG:  
            ModbusRTU_Write(FUNC_WR_MULREG,dataaddr,g_u16_WriteLen);
            break;
      default:  
            ModbusRTU_Write(FUNC_WR_MULREG,dataaddr,g_u16_WriteLen);
            break;
    }
    
  }
  else
  {

  }
}

