#include "main.h"
#include "t_console.h"
#include "stm32f10x_flash.h"   
#include "stm32f10x_rcc.h"	
#include "stm32f10x_usart.h"
#include "stdio.h"	  

/* uart */			
       u8    receive232[40];
       u16	 serial_Rxed_DataLen; 	
       u16   g_us_Timer1s=0;
       u8    g_uc_OKflag;	
       u8    programFlg=0;	 
       u8    SciReceiveFlag;						/*=1串口收到8个数据,＝0未收到8个数据*/
       u8    serial_Rxed_aPack; 					/*=1串口正在接收一帧数据，=0串口已接收完一帧数据*/
       u8    tail_Rxed;
       u16   tail_len;
       u8    Rx_headRecved;
       u8    Rx_transMeanNeed;     
	   u8    chksum;

/* flash */	   			
const  u32   APPL_CRC __attribute__((at(APP_CRC_ADDR)))={0xA1A2A3A4}; 
/* fbl control */
        u8   applValidFlg=kApplInvalid;/* appl is valid */  
		u8   time5s;                   /* stay in fbl just in 10s */                   
        u8   McuAddr;										   

/* console */
        u8   consoleMech=0;	  
        u8   consoleBuf[300];
        u16  consoleBufIdx=0; 
        u8   consoleCommaNum=0;	  
struct MesloopSt mesLoop[500]; 

#define CONSOLE_IDLE    0	
#define CONSOLE_RESET   1	 
#define CONSOLE_WR_RCD  2 	 
#define CONSOLE_RD_RCD  3    
#define CONSOLE_WR_FILE 4   
#define CONSOLE_RD_FILE 5
#define COMMA_WR_RCD    7

//Print("\r\n运行程序...");	  
void NVIC_GenerateSystemReset(void)
{
  SCB->AIRCR = AIRCR_VECTKEY_MASK | (u32)0x04;
}

void NVIC_GenerateCoreReset(void)
{
  SCB->AIRCR = AIRCR_VECTKEY_MASK | (u32)0x01;
}


void U0C1_ASC_vSendData(u8 value)
{
  UART4_Send_Data(&value,1);
}
  		  
u16 calcVal(u8 val,u16 *start,u16 len)
{
  u16 i,j;
  u16 calcRes=0;
  i=j=*start;
  for(;i<len;i++)
  {
    if(val==consoleBuf[i])
	{
	  for(;j<i;j++)
	  {
	    calcRes=calcRes*10+(consoleBuf[j]-'0');
	  }
	  break;
	}
  }
  *start=i+1;
  return calcRes;
}
void strcat(u8 s[],u8 t[])
{    
  int i=0,j=0;    
  while(s[i]!='\0')         /*找到串s的尾*/       
  i++;    
  while((s[i]=t[j])!='\0')
  {       /*拷贝t[j]到s[i]*/       
    i++;       
    j++;       
  }
}

u8 *updateStr(u8 str1[],u16 val)
{
  u8 str2[10];
  if(val>9999)
  {	
    str2[0]=(val/10000)+0x30;  
    str2[1]=(val%10000/1000)+0x30;	 
    str2[2]=(val%1000/100)+0x30;  	 
    str2[3]=(val%100/10)+0x30;  
    str2[4]=(val%10)+0x30;	  
    str2[5]=0;
  }	 
  else if(val>999)
  {
    str2[0]=(val/1000)+0x30;  
    str2[1]=(val%1000/100)+0x30;	 
    str2[2]=(val%100/10)+0x30;  	 
    str2[3]=(val%10)+0x30;    
    str2[4]=0;
  }	   
  else if(val>99)
  {
    str2[0]=(val/100)+0x30;  
    str2[1]=(val%100/10)+0x30;	 
    str2[2]=(val%10)+0x30;   
    str2[3]=0;
  }
  else if(val>9)
  {
    str2[0]=(val/10)+0x30;  
    str2[1]=(val%10)+0x30;	 
    str2[2]=0;
  }
  else 
  {
    str2[0]=(val%10)+0x30;	 
    str2[1]=0;
  }
  strcat(str1,str2);
  return str1;
}

static f_console_check(u8 val)
{
  u16 i;
  u16 loopIdx; 
  u16 loopEn; 
  u16 loopType;
   u8 loopIPAddr[4];
  u16 loopStAddr;
  u16 loopPort;
   u8 loopCycle;
   u8 mStr[1000];
  /* 判断当前是否是启动一次对话 */
  if('?'==val)
  {		 
    mStr[0]=0;
    consoleMech=CONSOLE_IDLE;
    consoleBufIdx=0;  
    strcat(mStr,"\r\n*************Shanghai MJ Intelligent Systems Co.,Ltd.***************");  	
	strcat(mStr,"\r\n**                    MES Console V1.0                            **"); 	 
    strcat(mStr,"\r\n** 1.Restart MES Device.                                          **");
	strcat(mStr,"\r\n** 2.Write one channel.                                           **");	  
	strcat(mStr,"\r\n** 3.Read one channel.                                            **");
	strcat(mStr,"\r\n** 4.Write configuration file.                                    **");
	strcat(mStr,"\r\n** 5.Read configuration file.                                     **");  
	strcat(mStr,"\r\n** 6.Reserve.                                                     **");	 
	strcat(mStr,"\r\n********************************************************************");
	Print(mStr);	  //1  							 
	Print("\r\n");	
	return;
  }

  switch(consoleMech)
  {
    case CONSOLE_IDLE:									   
	{
	  switch(val)
	  {
	    case 0x30:
		  consoleMech=val-'0';
		  consoleBufIdx=0;
		break; 
		  
		case 0x32:
		  consoleMech=val-'0';
		  consoleBufIdx=0;
		  consoleCommaNum=0;
          mStr[0]=0;	 
          strcat(mStr,"\r\nWrite channel Format：Idx,En,Type,IP,StAddr,Port,Cycle,"); 
		  strcat(mStr,"\r\nIdx: 0-499");		  
		  strcat(mStr,"\r\nEn:1 enable;0 disable");
		  strcat(mStr,"\r\nType: 1 TCP; 2 RTU; 3 ASCII; 4 IO; 5 ADC; 6 RS232; 7 RS485; 9-255 reserve");
		  strcat(mStr,"\r\nIPAddr[4]:IP address");
		  strcat(mStr,"\r\nStAddr:station address");
		  strcat(mStr,"\r\nPort:port");
		  strcat(mStr,"\r\ncycle:cycle*500ms");
          strcat(mStr,"\r\neg.config channel 8 as modbus TCP, IP 192.168.1.100, station 44, port 65535, cycle 2*500ms"); 
          strcat(mStr,"\r\nSend:8,1,1,192.168.1.100,44,65535,2,"); 
		  Print(mStr);	  //2	 
	      Print("\r\n");	
		break; 

		case 0x33:
		  consoleMech=val-'0';
		  consoleBufIdx=0;
		  consoleCommaNum=0;
		  mStr[0]=0;	 
          strcat(mStr,"\r\nRead channel Format：channel index,");
		  strcat(mStr,"\r\neg. Read channel 128 Send:128,"); 
		  Print(mStr);	  //3 
	      Print("\r\n");	
        break;

		case 0x34:
		  consoleMech=0;
		  consoleBufIdx=0;
		  consoleCommaNum=0;  		  	  
		  mStr[0]=0;
		  strcat(mStr,"\r\nwriting success");     
          Print(mStr);	  //12  	
	      Print("\r\n");		    
		break;

		case 0x35:
		  consoleMech=val-'0';
		  consoleBufIdx=0;	
		  mStr[0]=0;	 	
          strcat(mStr,"\r\nRead n channel Format：channel num,");
		  strcat(mStr,"\r\neg. Read first 10 channels Send:10,"); 
		  Print(mStr);	  //5  
	      Print("\r\n");	
		break; 
			
		case 0x36:
		  consoleMech=0;
		  consoleBufIdx=0;	
		  mStr[0]=0;	 	
          strcat(mStr,"\r\n This cmd is not supported,please check!");
		  Print(mStr);	  //14 
	      Print("\r\n");	
		break;

		case 0x31:		
		  mStr[0]=0;    
          strcat(mStr,"\r\nMES device Restart......"); 
		  Print(mStr);	  //6		  
	      Print("\r\n");	
		  NVIC_GenerateCoreReset();
		break;

		default:
		break;
	  }
	}
	break;

	case CONSOLE_WR_RCD:
	{ 
	  consoleBuf[consoleBufIdx++]=val;
	  if(','==val)
	  {
	    consoleCommaNum++;
		if(COMMA_WR_RCD==consoleCommaNum)
		{ 
		  i=0;
          /* loopNum */
		  loopIdx=calcVal(',',&i,consoleBufIdx);
		  if(loopIdx>499)
		  {	   				 
		    mStr[0]=0;    
            strcat(mStr,"\r\n Cmd received!");	   
            strcat(mStr,"\r\n Channel not exist!");	  	 
		    Print(mStr);	  //7	
	        Print("\r\n");	
		    consoleBufIdx=0;
		    consoleCommaNum=0;	
			return;
		  }
		  /* En */
		  loopEn=calcVal(',',&i,consoleBufIdx);
		  if(loopEn>1)
		  {	 		 		 
		    mStr[0]=0;  
            strcat(mStr,"\r\n Cmd received!");	  
            strcat(mStr,"\r\n Enable Error");	  
			Print(mStr);	  //8 		 
	        Print("\r\n");	
		    consoleBufIdx=0;
		    consoleCommaNum=0;
			return;
		  }
		  /*Type*/	
		  loopType=calcVal(',',&i,consoleBufIdx);
		  if((loopType>7)|(loopType==0))
		  {	 				 
		    mStr[0]=0;  
            strcat(mStr,"\r\n Cmd received!");	  //COMMA_WR_RCD 
            strcat(mStr,"\r\n Type Error");	  //COMMA_WR_RCD  
			Print(mStr);	  //9 	 
	        Print("\r\n");	
		    consoleBufIdx=0;
		    consoleCommaNum=0;
			return;
		  }
		  /*IP*/	
		  loopIPAddr[0]=loopIPAddr[1]=loopIPAddr[2]=loopIPAddr[3]=0;
		  loopIPAddr[0]=calcVal('.',&i,consoleBufIdx); 
		  loopIPAddr[1]=calcVal('.',&i,consoleBufIdx);
		  loopIPAddr[2]=calcVal('.',&i,consoleBufIdx);
		  loopIPAddr[3]=calcVal(',',&i,consoleBufIdx);
		  /* StAddr */
		  loopStAddr=calcVal(',',&i,consoleBufIdx);
		  /* Port */
		  loopPort=calcVal(',',&i,consoleBufIdx);  
		  /* cycle */
		  loopCycle=calcVal(',',&i,consoleBufIdx);	
		  mesLoop[loopIdx].En=loopEn;/* 1 for enable,other for disable */
		  mesLoop[loopIdx].Type=loopType;/* 1 for TCP; 2 for RTU; 3 for ASCII; 4 for IO; 5 for ADC; 6 for RS232; 7 for RS485; 9-255 reserve*/
		  mesLoop[loopIdx].IPAddr[0]=loopIPAddr[0];/* IP address */
		  mesLoop[loopIdx].IPAddr[1]=loopIPAddr[1];/* IP address */
		  mesLoop[loopIdx].IPAddr[2]=loopIPAddr[2];/* IP address */
		  mesLoop[loopIdx].IPAddr[3]=loopIPAddr[3];/* IP address */
		  mesLoop[loopIdx].StAddr=loopStAddr;/* station address */
		  mesLoop[loopIdx].Port=loopPort;/* port */
		  mesLoop[loopIdx].cycle=loopCycle;/* cycle*500ms */		 
		  mStr[0]=0;  
          strcat(mStr,"\r\nsetting sucess!");	  
		  Print(mStr);	  //10 	    
	      Print("\r\n");
		  consoleMech=0;
		  consoleBufIdx=0;
		  consoleCommaNum=0;	  
		}
	  }		  
	}
	break;

    case CONSOLE_RD_RCD:
	{  
	  consoleBuf[consoleBufIdx++]=val;
	  if(','==val)
	  {
	    i=0;					   
        /* loopNum */	
		loopIdx=calcVal(',',&i,consoleBufIdx);
		mStr[0]=0;
		strcat(mStr,"Channel "); 
	    updateStr(mStr,loopIdx);  
		   strcat(mStr,":,En:");  
		updateStr(mStr,mesLoop[loopIdx].En);  
		   strcat(mStr,",Type:"); 
		updateStr(mStr,mesLoop[loopIdx].Type);  
		   strcat(mStr,",IP:"); 
		updateStr(mStr,mesLoop[loopIdx].IPAddr[0]); 
		   strcat(mStr,"."); 	
		updateStr(mStr,mesLoop[loopIdx].IPAddr[1]); 
		   strcat(mStr,".");  	
		updateStr(mStr,mesLoop[loopIdx].IPAddr[2]); 
		   strcat(mStr,"."); 	
		updateStr(mStr,mesLoop[loopIdx].IPAddr[3]); 
		   strcat(mStr,",StAddr:");  
		updateStr(mStr,mesLoop[loopIdx].StAddr); 
		   strcat(mStr,",Port:");  
		updateStr(mStr,mesLoop[loopIdx].Port); 
		   strcat(mStr,",cycle:");  
		updateStr(mStr,mesLoop[loopIdx].cycle);   
        Print(mStr);	  //11   
	    consoleMech=0;
		consoleBufIdx=0;
		consoleCommaNum=0;
	   Print("\r\n");	
	  }	
	}
    break;

	case CONSOLE_WR_FILE: 		 
	break;	  
	
	case CONSOLE_RD_FILE: 
	  consoleBuf[consoleBufIdx++]=val;
	  if(','==val)
	  {
        /* loopNum */
		i=0;	
		loopIdx=calcVal(',',&i,consoleBufIdx);	
        if(loopIdx>500)
		{  		  		  		 
		  mStr[0]=0;  
          strcat(mStr,"\r\n channel is not valid");	  //COMMA_WR_RCD 
		  Print(mStr);	  //9 	 
	      Print("\r\n");	
		}

	    for(i=0;i<loopIdx;i++)
		{	
		     mStr[0]=0;
		     strcat(mStr,"\r\nChannel "); 
	      updateStr(mStr,i);  
		     strcat(mStr,":,En:");  
		  updateStr(mStr,mesLoop[i].En);  
		     strcat(mStr,",Type:"); 
		  updateStr(mStr,mesLoop[i].Type);  
		     strcat(mStr,",IP:"); 
	   	  updateStr(mStr,mesLoop[i].IPAddr[0]); 
		     strcat(mStr,"."); 	
		  updateStr(mStr,mesLoop[i].IPAddr[1]); 
		     strcat(mStr,".");  	
		  updateStr(mStr,mesLoop[i].IPAddr[2]); 
		     strcat(mStr,"."); 	
		  updateStr(mStr,mesLoop[i].IPAddr[3]); 
		     strcat(mStr,",StAddr:");  
		  updateStr(mStr,mesLoop[i].StAddr); 
		     strcat(mStr,",Port:");  
		  updateStr(mStr,mesLoop[i].Port); 
		     strcat(mStr,",cycle:");  
		  updateStr(mStr,mesLoop[i].cycle);   
              Print(mStr);	  //13 
		}  
		Print("\r\n");
	    consoleMech=0;
	    consoleBufIdx=0;
	    consoleCommaNum=0;   
	  } 
	break;
	
	default:
	break;
  }
}
#if 0
static u16 f_EraseMem(u32 size, u32 address)
{	 
  u32 pagenumber = 0x0;
  u32 i;
  u16 ioRc =IO_E_OK;
  /*calc how many pages to erase*/
  if ((size % PAGE_SIZE) != 0)
  {
    pagenumber = (size / PAGE_SIZE) + 1;
  }
  else
  {
    pagenumber = size / PAGE_SIZE;
  }
  /* unlock */
  FLASH_Unlock();
  /* erase */
  for(i=0;i<pagenumber;i++)
  {
    if(FLASH_COMPLETE!=FLASH_ErasePage(address + (PAGE_SIZE * i)))
	{
	  ioRc=65535-IO_E_OK;
	}
  }
  return ioRc;
}
#endif

u32 FlashDriver_RWriteSync(u8 *pData, u16 size, u32 addr)
{
  vu16 *pDataTemp = (vu16 *)pData;
  vu32 temp = addr;
  for (; temp < (addr + size); pDataTemp++, temp += 2)
  {
    FLASH_ProgramHalfWord(temp, *pDataTemp);
    if (*pDataTemp != *(vu16 *)temp)
    {
      return FALSE;
    }
  }
  return TRUE;
}

void f_console_Task(void)
{
  u8 chksum;
  if(1==serial_Rxed_aPack)
  {
    time5s=0;
	serial_Rxed_aPack=0;
	/* CMD: 6 handshake to reset*/
	if((6==receive232[0])&&(McuAddr==receive232[1]))
	{
	  g_uc_OKflag=0x55;
	  chksum=0;
	  U0C1_ASC_vSendData(3);
	  U0C1_ASC_vSendData(6);
	  U0C1_ASC_vSendData(McuAddr);
	  U0C1_ASC_vSendData(4);
	  chksum=3+6+McuAddr+4;
	  if((chksum/16)<=9)
	  {
        U0C1_ASC_vSendData((chksum/16)+48);
	  }
	  else if(((chksum/16)<=15))
	  {
	    U0C1_ASC_vSendData((chksum/16)+55);
	  }
	  if((chksum%16)<=9)
	  {
        U0C1_ASC_vSendData((chksum%16)+48);
	  }
	  else if(((chksum%16)<=15))
	  {
	    U0C1_ASC_vSendData((chksum%16)+55);
	  }
	  U0C1_ASC_vSendData(4);
      f_GenSoftwareReset();
	}
  }
}
chksum_tst(u8 tst)
{
  u8 chkH=0;
  u8 chkL=0;
  if(tail_Rxed)
  {
	 if((tail_len+2)==serial_Rxed_DataLen)
	 {
	   if(((receive232[serial_Rxed_DataLen-2])>='0')&&((receive232[serial_Rxed_DataLen-2])<='9'))
	   {
		 chkH=receive232[serial_Rxed_DataLen-2]-48;
	   }
	   else if(((receive232[serial_Rxed_DataLen-2])>='A')&&((receive232[serial_Rxed_DataLen-2])<='F'))
	   {
		 chkH=receive232[serial_Rxed_DataLen-2]-55;
	   }
	   else
	   {
	   }

	   if(((receive232[serial_Rxed_DataLen-1])>='0')&&((receive232[serial_Rxed_DataLen-1])<='9'))
	   {
		 chkL=receive232[serial_Rxed_DataLen-1]-48;
	   }
	   else if(((receive232[serial_Rxed_DataLen-1])>='A')&&((receive232[serial_Rxed_DataLen-1])<='F'))
	   {
		 chkL=receive232[serial_Rxed_DataLen-1]-55;
	   }
	   else
	   {
	   }
	   if(((chksum/16)==chkH)&&((chksum%16)==chkL))
	   {
		  Rx_headRecved = 0;
		  serial_Rxed_aPack = 1;
	   }
	   else
	   {
		  Rx_headRecved = 0;
	   }
	 }
  }
  else
  {
	chksum+=tst;
  }
}


// secrial_recv();
/* ****************************************************************
** 函 数 名: secrial_recv()
** 功能描述: 低优先级中断子程序：232接收发送中断子程序
*************************************************************** */
void secrial_recv(u8 tst)
{
  if (tst == PACK_HDR)
  {
    serial_Rxed_aPack = 0;
	serial_Rxed_DataLen = 0;
	Rx_headRecved = 1;
	Rx_transMeanNeed = 0;
	chksum=0;
	tail_len=0;
	tail_Rxed=0;
	//receive232[serial_Rxed_DataLen] = PACK_HDR;
	//serial_Rxed_DataLen++;
  }
  else
  {		
#if 1
	if(Rx_headRecved == 1)
	{
      switch(tst)
	  {
		case ESC:
		{
	      Rx_transMeanNeed = 1;
		  break;
		}
		case ESC_HDR:
		{
		  if(Rx_transMeanNeed)
		  {
			receive232[serial_Rxed_DataLen] = PACK_HDR;
			serial_Rxed_DataLen++;
			Rx_transMeanNeed = 0;
			chksum_tst(1);
		  } 
		  else
		  {
			receive232[serial_Rxed_DataLen] = ESC_HDR;
			serial_Rxed_DataLen++;
			Rx_transMeanNeed = 0;
			chksum_tst(tst);
		  }
		  break;
		}
		case ESC_END:
		{
		  if(Rx_transMeanNeed)
		  {
			receive232[serial_Rxed_DataLen] = PACK_END;
			serial_Rxed_DataLen++;
			Rx_transMeanNeed = 0;
			chksum_tst(2);
		  }
		  else
		  {
			receive232[serial_Rxed_DataLen] = ESC_END;
			serial_Rxed_DataLen++;
		    Rx_transMeanNeed = 0;
			chksum_tst(tst);
		  }
		  break;
		}
		case ESC_ESC:
		{
		  if(Rx_transMeanNeed)
		  {
			receive232[serial_Rxed_DataLen] = ESC;
			serial_Rxed_DataLen++;
			Rx_transMeanNeed = 0;
			chksum_tst(250);
		  }
		  else
		  {
			receive232[serial_Rxed_DataLen] = ESC_ESC;
			serial_Rxed_DataLen++;
			Rx_transMeanNeed = 0;
			chksum_tst(tst);
		  }
		  break;
		}
		case PACK_HDR:
		{
		  serial_Rxed_aPack = 0;
		  serial_Rxed_DataLen = 0;
		  Rx_headRecved = 1;
		  tail_len=0;
		  tail_Rxed=0;
		  Rx_transMeanNeed = 0;
          chksum=0;
		  break;
		}
		case PACK_END:
		{
		  //serial_Rxed_aPack = 1;
		  tail_Rxed=1;
		  tail_len=serial_Rxed_DataLen;
		  chksum+=PACK_END;
		  Rx_transMeanNeed = 0;
		  break;
		}
		default:
		{
		  receive232[serial_Rxed_DataLen] =(u8)tst;
		  serial_Rxed_DataLen++;
		  if(tst==1)
		  {
			tst=0;
		  }
		  chksum_tst(tst);
		  break;
		}//default:
	  }//end switch(SBUF)
	}
	else//
	{
	  serial_Rxed_aPack = 0;
	  serial_Rxed_DataLen = 0;
      Rx_transMeanNeed = 0;
	  Rx_headRecved = 0;
	  f_console_check(tst);
	}	 
  #endif
  }// end if(serial_Rxed_DataLen < MAX_PACK_SIZE)
  /* end serial_rev() */
}


