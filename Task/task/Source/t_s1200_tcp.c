#include "main.h"
#include "t_s1200_tcp.h"
#include "m_w5500.h"




#define S1200TCP_READ_TIMEOUT           250
#define S1200TCP_WRITE_TIMEOUT          250	   
#define S1200TCP_CONNECT_TIMEOUT        1000 //8位的


typedef struct 
{
    unsigned char ChkOngo; 
    unsigned char ChkType;
    unsigned  int ChkLen;  
    unsigned char ChkSn;

}StrCheckCtr;


/* S7200TCP */
// static unsigned char  g_ArrS7300TCP_COTPreq[22]    = {0x03,0x00,0x00,0x16,0x11,0xe0,0x00,0x00,0x00,0x09,0x00,0xc1,0x02,0x4d,0x57,0xc2,
//                                                0x02,0x4d,0x57,0xc0,0x01,0x0a};//22  
// static unsigned char  g_ArrS7300TCP_COTPres[22]    = {0x03,0x00,0x00,0x16,0x11,0xd0,0x00,0x09,0x53,0x38,0x00,0xc0,0x01,0x09,0xc1,0x02,
//                                                0x4d,0x57,0xc2,0x02,0x4d,0x57};//22									                        
// static unsigned char  g_ArrS7300TCP_T125req[25]    = {0x03,0x00,0x00,0x19,0x02,0xf0,0x80,0x32,0x01,0x00,0x00,0xcc,0xc1,0x00,0x08,0x00,
//                                                0x00,0xf0,0x00,0x00,0x01,0x00,0x01,0x03,0xc0}; ///25
// static unsigned char  g_ArrS7300TCP_T125res[27]    = {0x03,0x00,0x00,0x1b,0x02,0xf0,0x80,0x32,0x03,0x00,0x00,0xcc,0xc1,0x00,0x08,0x00,
//                                                0x00,0x00,0x00,0xf0,0x01,0x00,0x01,0x00,0x01,0x00,0xf0};//27
/* S7300TCP */
// unsigned char g_ArrS7300TCP_COTPreq[22] = {0x03,0x00,0x00,0x16,0x11,0xe0,0x00,0x00,0x00,0x01,0x00,0xc1,0x02,
//                                            0x01,0x00,0xc2,0x02,0x01,0x02,0xc0,0x01,0x09};/* 22 */ 
// unsigned char g_ArrS7300TCP_COTPres[22] = {0x03,0x00,0x00,0x16,0x11,0xd0,0x00,0x01,0x44,0x31,0x00,0xc0,0x01,
//                                            0x09,0xc1,0x02,0x01,0x00,0xc2,0x02,0x01,0x02};/* 22 */
// unsigned char g_ArrS7300TCP_T125req[25] = {0x03,0x00,0x00,0x19,0x02,0xf0,0x80,0x32,0x01,0x00,0x00,0xcc,0xc1,
//                                            0x00,0x08,0x00,0x00,0xf0,0x00,0x00,0x01,0x00,0x01,0x03,0xc0};/* 25 */
// unsigned char g_ArrS7300TCP_T125res[27] = {0x03,0x00,0x00,0x1b,0x02,0xf0,0x80,0x32,0x03,0x00,0x00,0xcc,0xc1,
//                                            0x00,0x08,0x00,0x00,0x00,0x00,0xf0,0x00,0x00,0x01,0x00,0x01,0x00,
//                                            0xf0};/* 27 */
/* S7400TCP */
// static unsigned char  g_ArrS7300TCP_COTPreq[22]    = {0x03,0x00,0x00,0x16,0x11,0xe0,0x00,0x00,0x00,0x01,0x00,0xc1,0x02,0x02,0x00,0xc2,
//                                                0x02,0x02,0x03,0xc0,0x01,0x0a};//22  
// static unsigned char  g_ArrS7300TCP_COTPres[22]    = {0x03,0x00,0x00,0x16,0x11,0xd0,0x00,0x01,0x00,0x06,0x00,0xc0,0x01,0x0a,0xc1,0x02,
//                                                0x02,0x00,0xc2,0x02,0x02,0x03};//22									                        
// static unsigned char  g_ArrS7300TCP_T125req[25]    = {0x03,0x00,0x00,0x19,0x02,0xf0,0x80,0x32,0x01,0x00,0x00,0xcc,0xc1,0x00,0x08,0x00,
//                                                0x00,0xf0,0x00,0x00,0x01,0x00,0x01,0x03,0xc0}; ///25
// static unsigned char  g_ArrS7300TCP_T125res[27]    = {0x03,0x00,0x00,0x1b,0x02,0xf0,0x80,0x32,0x03,0x00,0x00,0xcc,0xc1,0x00,0x08,0x00,
//                                                0x00,0x00,0x00,0xf0,0x00,0x00,0x01,0x00,0x01,0x01,0xe0};//27

 /* S1200TCP */
 static unsigned char  g_ArrS1200TCP_COTPreq[22]    = {0x03,0x00,0x00,0x16,0x11,0xe0,0x00,0x00,0x00,0x01,0x00,0xc1,0x02,0x01,0x00,0xc2,
                                                0x02,0x01,0x01,0xc0,0x01,0x09};//22  
 static unsigned char  g_ArrS1200TCP_COTPres[22]    = {0x03,0x00,0x00,0x16,0x11,0xd0,0x00,0x01,0x00,0x06,0x00,0xc0,0x01,0x09,0xc1,0x02,
                                                0x01,0x00,0xc2,0x02,0x01,0x01};//22									                        
 static unsigned char  g_ArrS1200TCP_T125req[25]    = {0x03,0x00,0x00,0x19,0x02,0xf0,0x80,0x32,0x01,0x00,0x00,0xff,0xff,0x00,0x08,0x00,
                                                0x00,0xf0,0x00,0x00,0x01,0x00,0x01,0x07,0x80}; ///25
 static unsigned char  g_ArrS1200TCP_T125res[27]    = {0x03,0x00,0x00,0x1b,0x02,0xf0,0x80,0x32,0x03,0x00,0x00,0xff,0xff,0x00,0x08,0x00,
                                                0x00,0x00,0x00,0xf0,0x00,0x00,0x01,0x00,0x01,0x00,0xf0};//27
                                                
/* 200 smart */
//  static unsigned char  g_ArrS7300TCP_COTPreq[22]    = {0x03,0x00,0x00,0x16,0x11,0xe0,0x00,0x00,0x00,0x09,0x00,0xc1,0x02,0x02,0x00,0xc2,
//                                                 0x02,0x02,0x00,0xc0,0x01,0x09};//22  
//  static unsigned char  g_ArrS7300TCP_COTPres[22]    = {0x03,0x00,0x00,0x16,0x11,0xd0,0x00,0x09,0x00,0x06,0x00,0xc0,0x01,0x09,0xc1,0x02,
//                                                 0x02,0x00,0xc2,0x02,0x02,0x00};//22									                        
//  static unsigned char  g_ArrS7300TCP_T125req[25]    = {0x03,0x00,0x00,0x19,0x02,0xf0,0x80,0x32,0x01,0x00,0x00,0xcc,0xc1,0x00,0x08,0x00,
//                                                 0x00,0xf0,0x00,0x00,0x01,0x00,0x01,0x03,0xc0}; ///25
//  static unsigned char  g_ArrS7300TCP_T125res[27]    = {0x03,0x00,0x00,0x1b,0x02,0xf0,0x80,0x32,0x03,0x00,0x00,0xcc,0xc1,0x00,0x08,0x00,
//                                                 0x00,0x00,0x00,0xf0,0x00,0x00,0x01,0x00,0x01,0x00,0xf0};//27


static unsigned char g_ArrS1200TCPBuff_R[31]   = {0x03,0x00,0x00,0x1F,0x02,0xF0,0x80,0x32,0x01,0x00,0x00,0x00,0xA3,
                                           0x00,0x0E,0x00,0x00,0x04,0x01,0x12,0x0A,0x10,0x02,0x00,0x01,0x00,
                                           0x00,0x81,0x00,0x00,0x00};
/* 写操作一共有35个字节的开销,当前仅支持连续100个字的写入操作，因此这里面的缓冲区大小是：235=35+200 */
static unsigned char g_ArrS1200TCPBuff_W[235]  = {0x03,0x00,0x00,0x25,0x02,0xf0,0x80,0x32,0x01,0x00,0x00,0x00,0x0c,
                                           0x00,0x0e,0x00,0x06,0x05,0x01,0x12,0x0a,0x10,0x04,0x00,0x01,0x00,
                                           0x01,0x84,0x00,0x00,0x00,0x00,0x04,0x00,0x10,0x00,0x63};

static unsigned char g_ArrS1200MasterRxBuff[200];/* TCP接受测缓冲区 */
static unsigned char g_u8FlagS1200TCPShake     = 0;/* 套接字状态-eg有没有收到数据 */
static unsigned  int g_u16FlagS1200TCPAPPShakeTime = 0;/* 应用层连接时间，交互计时超时*/
static unsigned char g_u8FlagS1200TCPAPPLink   = 0;/* 应用层连接状态 3-正常数据交互 */
static unsigned char g_u8FlagS1200TCPAPPrw   = 0;/* 应用层读写状态 0-IDLE，1-读，2-写 */
static unsigned char g_u8FlagS1200TCPSocketLink = 0;/* 套接字连接状态0-无，1-Init，2-Connected */
static unsigned char g_u8FlagS1200TCPSocketData = 0;/* 套接字发送接收数据状态 */
static unsigned  int g_u8TimerS1200TCPCommu    = 0;/* 应用层读写超时管理 */
static unsigned  int g_u16TimerCount1ms      = 0;
static StrCheckCtr g_StrS1200TCPChkLoop_R;/*读*/
static StrCheckCtr g_StrS1200TCPChkLoop_W;/*写*/
static unsigned char g_u8S1200TCPsn         = 0;/*与S7-300PLC的SN序号*/
static u8 S1200_tcp=0xff;;
static u8 S1200_tcp_Enable=OFF;
static u16 S1200_CommTimer;

void S1200TCP_Init(u8 nodeID)
{
  S1200_tcp = nodeID;
  S1200_tcp_Enable=ON;
}
    
/* S7-300 TCP写操作 */	     
void S1200TCP_Write(unsigned char l_u8RegType,    /*寄存器类型*/
                    unsigned  int l_u16DBIndex,   /*DB块号*/
                    unsigned  int l_u16RegOffset, /*寄存器偏移量*/
                    unsigned  int l_u16WriteLen)  /*写数据的字数*/
{
	
	unsigned  int l_u16FrameNum ;/*帧长度+35（字节）*/
  unsigned  int l_u16DataNum ;/*数据长度+4（字节）*/
	unsigned  int l_u16WrNum    = l_u16WriteLen;/*写入个数*/
	unsigned long l_u32Offset ;/*偏移量是位数量*/
	unsigned long l_u32WrbitNum = l_u16WriteLen*16;/*被写入的字符的位数量*/
	unsigned  int l_u16Index;
    
  g_u8TimerS1200TCPCommu          = S1200TCP_WRITE_TIMEOUT;

	if (3 == g_u8FlagS1200TCPAPPLink)
	{					 
		g_u8S1200TCPsn++;/*update sn*/
		g_StrS1200TCPChkLoop_W.ChkSn    = g_u8S1200TCPsn;
		g_ArrS1200TCPBuff_W[12]         = g_u8S1200TCPsn;

	  if (('Q' == l_u8RegType)||('q' == l_u8RegType))
		{   /*如Q1写入时偏移量8bit，长度8bit*/	
		    /*如Q0写入时偏移量0bit，长度8bit*/
        //    l_u32Offset     = l_u32Offset / 2;
		  //  l_u32WrbitNum   = l_u32WrbitNum / 2;
      l_u16WrNum = l_u16WrNum*2;
            l_u32Offset= ((l_u16RegOffset>>8) * 8) ;
			l_u16FrameNum = l_u16WriteLen*2+35;
			g_ArrS1200TCPBuff_W[22]     = 4;
			g_ArrS1200TCPBuff_W[25]     = l_u16DBIndex / 256;
			g_ArrS1200TCPBuff_W[26]     = l_u16DBIndex % 256;
			g_ArrS1200TCPBuff_W[27]     = 0x82;	
			g_ArrS1200TCPBuff_W[32]     = 0x04;	
			for(l_u16Index=0; l_u16Index<l_u16WrNum; l_u16Index++) 
			{
				 g_ArrS1200TCPBuff_W[l_u16Index+35]  = g_u8_Writedata[l_u16Index];
			}
    }
		else if (('M' == l_u8RegType)||('m' == l_u8RegType))
		{	
      l_u16WrNum = l_u16WrNum*2;
            l_u32Offset= ((l_u16RegOffset>>8) * 8) ;
			l_u16FrameNum = l_u16WriteLen*2+35;
			g_ArrS1200TCPBuff_W[22]     = 4;
			g_ArrS1200TCPBuff_W[25]     = l_u16DBIndex / 256;
			g_ArrS1200TCPBuff_W[26]     = l_u16DBIndex % 256;
			g_ArrS1200TCPBuff_W[27]     = 0x83;
			g_ArrS1200TCPBuff_W[32]     = 0x04;	
			for(l_u16Index=0; l_u16Index<l_u16WrNum; l_u16Index++) 
			{
				 g_ArrS1200TCPBuff_W[l_u16Index+35]  = g_u8_Writedata[l_u16Index];
			}
		}
		else if (('V' == l_u8RegType)||('v' == l_u8RegType))
		{
			l_u16WrNum = l_u16WrNum*2;
            l_u32Offset= l_u16RegOffset * 8;
			l_u16FrameNum = l_u16WriteLen*2+35;
			g_ArrS1200TCPBuff_W[22]     = 4;
			g_ArrS1200TCPBuff_W[25]     = l_u16DBIndex / 256;
			g_ArrS1200TCPBuff_W[26]     = l_u16DBIndex % 256;
			g_ArrS1200TCPBuff_W[27]     = 0x84;	
			g_ArrS1200TCPBuff_W[32]     = 0x04;	
			for(l_u16Index=0; l_u16Index<l_u16WrNum; l_u16Index++) 
			{
				 g_ArrS1200TCPBuff_W[l_u16Index+35]  = g_u8_Writedata[l_u16Index];
			}
		}
		else
		{
				return;
		}
       
        g_StrS1200TCPChkLoop_W.ChkType  = l_u8RegType;
		g_StrS1200TCPChkLoop_W.ChkLen   = 0;
		g_StrS1200TCPChkLoop_W.ChkOngo  = 1;
		l_u16DataNum  = l_u16WrNum+4;
		
		g_ArrS1200TCPBuff_W[ 2]     = l_u16FrameNum / 256;/*帧长度高字节*/
		g_ArrS1200TCPBuff_W[ 3]     = l_u16FrameNum % 256;/*帧长度低字节*/
		g_ArrS1200TCPBuff_W[15]     = l_u16DataNum / 256;/*数据长度高字节*/
		g_ArrS1200TCPBuff_W[16]     = l_u16DataNum % 256;/*数据长度低字节*/ 
		
		g_ArrS1200TCPBuff_W[23]     = 0;
		g_ArrS1200TCPBuff_W[24]     = l_u16WriteLen % 256;
           
		g_ArrS1200TCPBuff_W[28]     = (l_u32Offset>>16)%256;	
		g_ArrS1200TCPBuff_W[29]     = (l_u32Offset>>8)%256;	
		g_ArrS1200TCPBuff_W[30]     = (l_u32Offset%256);
		g_ArrS1200TCPBuff_W[33]     = l_u32WrbitNum / 256;	
		g_ArrS1200TCPBuff_W[34]     = l_u32WrbitNum % 256;
		
		DAQ_EthSend(0, g_ArrS1200TCPBuff_W, l_u16Index+35);
	}
	else
	{
	}
}


/* S7-300 TCP读操作 */	     
void S1200TCP_Read(unsigned char l_u8RegType,    /*寄存器类型*/
                   unsigned  int l_u16DBIndex,   /*DB块号*/
                   unsigned  int l_u16RegOffset, /*寄存器偏移量*/
                   unsigned  int l_u16ReadLen)  /*写数据的字数*/
{
    
    unsigned long l_u32Offset;
		   
    
    g_u8TimerS1200TCPCommu          = S1200TCP_WRITE_TIMEOUT;

	if (3 == g_u8FlagS1200TCPAPPLink)
	{    
        
        g_u8S1200TCPsn++;/*update sn*/
		g_StrS1200TCPChkLoop_R.ChkSn    = g_u8S1200TCPsn;
		g_ArrS1200TCPBuff_R[12]         = g_u8S1200TCPsn;
        
		if (('I'==l_u8RegType)||('i'==l_u8RegType))
		{	
            l_u32Offset= ((l_u16RegOffset>>8) * 8);
			g_ArrS1200TCPBuff_R[25]     = l_u16DBIndex / 256;
            g_ArrS1200TCPBuff_R[26]     = l_u16DBIndex % 256;
            g_ArrS1200TCPBuff_R[27]     = 0x81;
		}
		else if (('Q'==l_u8RegType)||('q'==l_u8RegType))
		{
			l_u32Offset= ((l_u16RegOffset>>8) * 8);
            g_ArrS1200TCPBuff_R[25]     = l_u16DBIndex / 256;
            g_ArrS1200TCPBuff_R[26]     = l_u16DBIndex % 256;
            g_ArrS1200TCPBuff_R[27]     = 0x82;

		}
		else if (('M'==l_u8RegType)||('m'==l_u8RegType))
		{
            l_u32Offset= ((l_u16RegOffset>>8) * 8);
			g_ArrS1200TCPBuff_R[25]     = l_u16DBIndex / 256;
            g_ArrS1200TCPBuff_R[26]     = l_u16DBIndex % 256;
            g_ArrS1200TCPBuff_R[27]     = 0x83;

		}
		else if (('V'==l_u8RegType)||('v'==l_u8RegType))
		{			  
            l_u32Offset= l_u16RegOffset * 8;
            g_ArrS1200TCPBuff_R[25]     = l_u16DBIndex / 256;
            g_ArrS1200TCPBuff_R[26]     = l_u16DBIndex % 256;
            g_ArrS1200TCPBuff_R[27]     = 0x84;
		}
        
        g_StrS1200TCPChkLoop_R.ChkType  = l_u8RegType;
		g_StrS1200TCPChkLoop_R.ChkLen   = l_u16ReadLen * 2;
		g_StrS1200TCPChkLoop_R.ChkOngo  = 1;
		g_ArrS1200TCPBuff_R[24]     = g_StrS1200TCPChkLoop_R.ChkLen % 256;
        g_ArrS1200TCPBuff_R[28]     = (l_u32Offset>>16)%256;	
        g_ArrS1200TCPBuff_R[29]     = (l_u32Offset>>8)%256;	
        g_ArrS1200TCPBuff_R[30]     = (l_u32Offset%256);

						
		DAQ_EthSend(0, g_ArrS1200TCPBuff_R, 31);
        
	}
	else if(5 == l_u8RegType)/* 建立连接，第一拍 发送cotp,之后等待 第二拍,第三拍 */
	{								
		g_u8TimerS1200TCPCommu = S1200TCP_CONNECT_TIMEOUT;
		DAQ_EthSend(0, g_ArrS1200TCP_COTPreq, 22); 
	} 
	else if(6 == l_u8RegType)/* 建立连接，第三拍收到之后 发送第四拍(T.125),之后等待第五拍 */
	{
		g_u8TimerS1200TCPCommu = S1200TCP_CONNECT_TIMEOUT;
		DAQ_EthSend(0, g_ArrS1200TCP_T125req, 25);
	}
	else
	{
	}
}


/* 1ms定时器 */
void TIM_1ms_S1200TCP(void)
{
  unsigned char l_u8FlagLink     = 0;
  if (g_u16TimerCount1ms > 0)
  {
      g_u16TimerCount1ms--;
  }
  
  if(S1200_CommTimer<=2000)
  {
    S1200_CommTimer++;
  }
  else
  {
    S1200_CommTimer =0;
    g_u8FlagS1200TCPAPPLink=0;
  }
  
  if (2 != (g_u8FlagS1200TCPSocketLink&2))/* 如果socket0 未建立连接，则不握手监听 */
  {
      l_u8FlagLink    = 1;
  }

  if (0 == l_u8FlagLink)
  {
    /* g_u8FlagS1200TCPAPPLink，3连接成功 */
    if (3 != g_u8FlagS1200TCPAPPLink)
    {
      g_u16FlagS1200TCPAPPShakeTime++;
      //建立连接过程
      if (0 == g_u8FlagS1200TCPAPPLink)
      {	 
        g_u8FlagS1200TCPAPPLink = 1;
        g_u16FlagS1200TCPAPPShakeTime=0;
        S1200TCP_Read(5,0,0,0);/* 1I;2Q;3M;4;V */
      } 
      else
      {
        if (g_u16FlagS1200TCPAPPShakeTime > S1200TCP_CONNECT_TIMEOUT)	  	
        {/* S1200TCP_CONNECT_TIMEOUTms时间没连上，重连 */
            g_u8FlagS1200TCPAPPLink     = 0;
            g_u16FlagS1200TCPAPPShakeTime   = 0;
        }
      }
    } 
    else /* if (3 == g_u8FlagS1200TCPAPPLink) */										  
    {
      if (g_u8TimerS1200TCPCommu > 0)
      {/* 正常通信的计数 */
          g_u8TimerS1200TCPCommu--;
      }
    }
  }
    
    /* 超时等状态处理 */
  if(S1200_tcp_Enable==ON)
  {
    g_u16_SwitchTimer[S1200_tcp]++;
  }
  if((ON==MB_NeedWrite(S1200_tcp))&&(READ_IDLE==GW_ReadStatus(S1200_tcp)))
	{ 	
    if(GW_WriteStatus(S1200_tcp)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[S1200_tcp]>1000)
	     {
	     		g_u16_SwitchTimer[S1200_tcp]=0;
					GW_WriteStatus(S1200_tcp)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(S1200_tcp)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[S1200_tcp]>=WriteTime))
	     {	
		   		GW_WriteStatus(S1200_tcp)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[S1200_tcp]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(S1200_tcp)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(S1200_tcp)=WRITE_DELAY;
      g_u16_SwitchTimer[S1200_tcp]=0;
	  }
	  else if(g_u16_SwitchTimer[S1200_tcp]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[S1200_tcp]=0;
      GW_WriteStatus(S1200_tcp)=WRITE_IDLE;
	    MB_NeedWrite(S1200_tcp)=OFF;
	    g_u8_threadIdx[S1200_tcp]++;
		  ThreadNew(S1200_tcp)=ON;
	  }
	  else{

	  }
	}
	else
	{
		if((g_u16_SwitchTimer[S1200_tcp]>UpdateCycle[S1200_tcp])&&
		    (g_u8_ProtocalNum[S1200_tcp][READ]!=0))
		{	  
		  g_u16_SwitchTimer[S1200_tcp]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(S1200_tcp))
		  {
		    GW_ReadStatus(S1200_tcp)=READ_IDLE;
		    g_u16_TimeoutCnt[S1200_tcp][g_u8_threadIdx[S1200_tcp]]=0;
		    if(OFF==MB_NeedWrite(S1200_tcp))
		    {
          g_u8_threadIdx[S1200_tcp]++;
          while(RegisterCfgBuff[S1200_tcp][g_u8_threadIdx[S1200_tcp]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[S1200_tcp]++;
            if(g_u8_ProtocalNum[S1200_tcp][READ]<=g_u8_threadIdx[S1200_tcp])
      			{			  
      	      g_u8_threadIdx[S1200_tcp]=0;
      			}
          }
		      ThreadNew(S1200_tcp)=ON;
		    }
		    else
		    {
          
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[S1200_tcp][g_u8_threadIdx[S1200_tcp]]++;
        if(g_u16_TimeoutCnt[S1200_tcp][g_u8_threadIdx[S1200_tcp]]>
          (THRES_TIMOUTCNT/UpdateCycle[S1200_tcp]))
        {
          g_u16_RecvTransLen[S1200_tcp][g_u8_threadIdx[S1200_tcp]]=0;
          g_u16_TimeoutCnt[S1200_tcp][g_u8_threadIdx[S1200_tcp]]=0;
          if(OFF==MB_NeedWrite(S1200_tcp))
  		    {
            g_u8_threadIdx[S1200_tcp]++;
            while(RegisterCfgBuff[S1200_tcp][g_u8_threadIdx[S1200_tcp]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[S1200_tcp]++;
              if(g_u8_ProtocalNum[S1200_tcp][READ]<=g_u8_threadIdx[S1200_tcp])
        			{			  
        	      g_u8_threadIdx[S1200_tcp]=0;
        			}
            }
  		      ThreadNew(S1200_tcp)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(S1200_tcp)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[S1200_tcp][READ]<=g_u8_threadIdx[S1200_tcp])
	{			  
		g_u8_threadIdx[S1200_tcp]=0;
	}
}


void S1200TCP_RecDataType(unsigned char regtype, 
                          unsigned char *sourcebuf, 
                          unsigned char *targetbuf, 
                          unsigned char datalen)
{
    
    unsigned char l_u8Index;
    
    if((regtype=='I')||(regtype=='Q')||(regtype=='M')||(regtype=='S'))
    {
      targetbuf[0]=0x00;
      targetbuf[1]= (sourcebuf[0]>>(RegisterCfgBuff[S1200_tcp][g_u8_threadIdx[S1200_tcp]].regaddr[7]))&0x01;
    }
    else if((regtype=='V')||(regtype=='C')||(regtype=='T'))
    {
			memcpy(targetbuf,sourcebuf,datalen);
    }
    else
    {

    }
}


//接收数据TCP的子task
void f_S1200TCP_Task(void)
{
    
    unsigned  int l_u16MasterRecvLen    = 0;
    unsigned  int l_u16Index            = 0;
    unsigned char l_u8DataType;
    unsigned int l_u32DataAddr;
    unsigned char l_u8ReStateSIR        = 0;
    unsigned char l_u8ReStateSnIR       = 0;
    
    /* 读 */
    if (0 != ThreadNew(S1200_tcp))
    {
        ThreadNew(S1200_tcp)    = 0;
        GW_ReadStatus(S1200_tcp)    = READ_WAITRESPOND;
        l_u32DataAddr    = (RegisterCfgBuff[S1200_tcp][g_u8_threadIdx[S1200_tcp]].regaddr[5]<<16)
                        +  (RegisterCfgBuff[S1200_tcp][g_u8_threadIdx[S1200_tcp]].regaddr[6]<<8)
                        +  RegisterCfgBuff[S1200_tcp][g_u8_threadIdx[S1200_tcp]].regaddr[7];
        
        S1200TCP_Read(RegisterCfgBuff[S1200_tcp][g_u8_threadIdx[S1200_tcp]].regtype,
                    ((RegisterCfgBuff[S1200_tcp][g_u8_threadIdx[S1200_tcp]].regaddr[2]<<8)+RegisterCfgBuff[S1200_tcp][g_u8_threadIdx[S1200_tcp]].regaddr[3]),
                      l_u32DataAddr,
                      RegisterCfgBuff[S1200_tcp][g_u8_threadIdx[S1200_tcp]].datalenth);

    }
    /* 写 */
    else if (WRITE_PRESEND == GW_WriteStatus(S1200_tcp))
    {		   
        GW_WriteStatus(S1200_tcp)    = WRITE_WAITRESPOND;
        l_u32DataAddr    = (g_u8_WriteAddr[5]<<16)+(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
        S1200TCP_Write(RegisterCfgBuff[S1200_tcp][g_u8_RespondID].regtype,
                     ((g_u8_WriteAddr[2]<<8)+g_u8_WriteAddr[3]),
                       l_u32DataAddr,
                       g_u16_WriteLen);
    
    }
    else
    {
    }
    
    if (0 == g_u16TimerCount1ms)
    {
        g_u16TimerCount1ms  = 10;
    }
    else
    {
        return;
    }
    /* 断开重连 */
    if (g_u8TimerS1200TCPCommu > 0)/* 正常通信中，定时读时判断状态 */
 	{
        
        if(0x02 != (g_u8FlagS1200TCPSocketLink & 0x02))/* 端口0初始化配置 */
        {
            if (1 == mb_tcp_master_Socket_Connect(0))
            {	   
                g_u8FlagS1200TCPSocketLink = 1;
            }
            else
            {
                g_u8FlagS1200TCPSocketLink = 0;
            }
        }
		Delay1ms(20);
    }
    
    /* Socket的连接 和 接收数据 */
    l_u8ReStateSIR   = MasterRead_1_Byte(SIR);/* 判断哪个socket中断 */	
	if (0x01 == (l_u8ReStateSIR & 0x01))/* Socket0事件处理 */
	{
		l_u8ReStateSnIR   = MasterRead_SOCK_1_Byte(0, Sn_IR);/* 读取Socket0中断标志寄存器 */
		MasterWrite_SOCK_1_Byte(0, Sn_IR, l_u8ReStateSnIR);
        
		if (l_u8ReStateSnIR & IR_CON)/* 在TCP模式下,Socket0成功连接 */ 
		{
			g_u8FlagS1200TCPSocketLink   |= 0x02;/* 网络连接状态0x02,端口完成连接，可以正常传输数据 */
		}
		if (l_u8ReStateSnIR & IR_DISCON)//在TCP模式下Socket断开连接处理
		{
			MasterWrite_SOCK_1_Byte(0, Sn_CR, CLOSE);//关闭端口,等待重新打开连接
            mb_tcp_master_Socket_Init(0);            
			g_u8FlagS1200TCPSocketLink   = 0;//网络连接状态0x00,端口连接失败
		}
		if (l_u8ReStateSnIR & IR_SEND_OK)//Socket0数据发送完成,可以再次启动S_tx_process()函数发送数据 
		{
			g_u8FlagS1200TCPSocketData  = 2;//端口发送一个数据包完成 
		}
		if (l_u8ReStateSnIR & IR_RECV)//Socket接收到数据,可以启动S_rx_process()函数 
		{
			g_u8FlagS1200TCPSocketData  = 1;//端口接收到一个数据包
		}
		if (l_u8ReStateSnIR & IR_TIMEOUT)//Socket连接或数据传输超时处理 
		{
			MasterWrite_SOCK_1_Byte(0, Sn_CR, CLOSE);// 关闭端口,等待重新打开连接 			
			g_u8FlagS1200TCPSocketLink   = 0;//网络连接状态0x00,端口连接失败
		}
	}
    
    if (1 == g_u8FlagS1200TCPSocketData)//如果Socket0接收到数据
    {
        g_u8FlagS1200TCPSocketData  = 0;
        S1200_CommTimer=0;
        l_u16MasterRecvLen  = MasterRead_SOCK_Data_Buffer(0, g_ArrS1200MasterRxBuff);  
        /* 发送第一拍COTP 等待回复 */
        if (1 == g_u8FlagS1200TCPAPPLink) 
        {
            if (22 == l_u16MasterRecvLen)
            {
                g_u8FlagS1200TCPAPPLink     = 2;
                g_u16FlagS1200TCPAPPShakeTime   = 0;
                S1200TCP_Read(6,0,0,0);/* 1I;2Q;3M;4;D */
            }
        }  
        /* 发送第二拍T125 等待回复 */
        else if (2 == g_u8FlagS1200TCPAPPLink)
        {
           if (27 == l_u16MasterRecvLen)
           {	
              g_u8FlagS1200TCPAPPLink   = 3;
              g_u16FlagS1200TCPAPPShakeTime=0;
              g_u8TimerS1200TCPCommu    = 0;
              g_u8FlagS1200TCPAPPrw     = 0;
           } 
        }
        /* 连接成功后的数据交互 */
        else if (3 == g_u8FlagS1200TCPAPPLink)
        {	
            /* 读 */
            if (1 == g_StrS1200TCPChkLoop_R.ChkOngo)
            {
                /* 校验长度 */
                if (l_u16MasterRecvLen == (g_StrS1200TCPChkLoop_R.ChkLen+25))
                {
                    /* 校验序号 */
                    if(g_ArrS1200MasterRxBuff[12]== g_StrS1200TCPChkLoop_R.ChkSn) 
                    {
                        g_u8FlagS1200TCPAPPrw   = 0;
                        g_u8TimerS1200TCPCommu  = 0; 
                        g_StrS1200TCPChkLoop_R.ChkOngo=0;	 
												S1200TCP_RecDataType(RegisterCfgBuff[S1200_tcp][g_u8_threadIdx[S1200_tcp]].regtype,
													&g_ArrS1200MasterRxBuff[25],
													&g_u8_EthRespData[S1200_tcp][g_u16_StartAddr[S1200_tcp][g_u8_threadIdx[S1200_tcp]]*2],
													g_StrS1200TCPChkLoop_R.ChkLen);
                        g_u16_RecvTransLen[S1200_tcp][g_u8_threadIdx[S1200_tcp]] =
                                 RegisterCfgBuff[S1200_tcp][g_u8_threadIdx[S1200_tcp]].datalenth*2;
                        GW_ReadStatus(S1200_tcp)   = READ_RECVSUCCESS;
                    }
                }
            }
            
            /* 写 */
            if (1 == g_StrS1200TCPChkLoop_W.ChkOngo)
            {
                /* 校验长度 */
                if (l_u16MasterRecvLen == (g_StrS1200TCPChkLoop_W.ChkLen+22))
                {
                    /* 校验序号 */
                    if (g_ArrS1200MasterRxBuff[12] == g_StrS1200TCPChkLoop_W.ChkSn) 
                    {														
                        g_u8FlagS1200TCPAPPrw   = 0;
                        g_u8TimerS1200TCPCommu  = 0; 
                        g_StrS1200TCPChkLoop_W.ChkOngo  = 0;
                        
                        GW_WriteStatus(S1200_tcp)    = WRITE_RECVSUCCESS;
                        memcpy(&g_u8_EthRespData[S1200_tcp][(g_u16_StartAddr[S1200_tcp][g_u8_RespondID]+
				                  g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
                    }
                }
            }
        }
    }
    else
    {	    
    }
    
}








