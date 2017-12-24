#ifndef DIAG_H_
#define DIAG_H_	 
#include "stm32f10x.h"  
#define AIRCR_VECTKEY_MASK        ((u32)0x05FA0000) 
#define IO_E_OK                   ((u8)0x00u)
#define IO_E_NOT_OK               ((u8)0x01u)		
#define kFblOk                    0
#define kFblFailed                1
#define kProgRequest              0x01u
#define kNoProgRequest            0xFEu
#define kApplValid                0x01u /* Application is fully programmed */
#define kApplInvalid              0x00u /* Operational software is missing */
#define HandShakeFailed           0x00u
#define HandShakeSuccess          0x01u

/* transmit control */
#define PACK_HDR                  1			 
#define PACK_END                  2
#define ESC                       250
#define ESC_HDR                   251
#define ESC_END                   252
#define ESC_ESC                   253

/* flash allocate */
//#define APP_CRC_BASE              0x8024000 
//#define APP_CRC_ADDR              0x8024000
#define ApplicationMsp            0x8004800 
#define ApplicationVect           (ApplicationMsp+4) 
#define PAGE_SIZE                 (0x800)    /* 2 Kbytes */
#define FLASH_SIZE                (0x80000)  /* 512 KBytes */	  
#define ApplicationSize           0x20000   
#define ADU_LENGTH                0x400  

typedef  void (*FunVoidType)(void);
typedef u32 (*FunWriteType)(u8*, u32, u16);	 
void U0C1_ASC_vSendData(u8 value); 

/* Exported constants --------------------------------------------------------*/  
void initPara(void);															  
void CommonInit(void);
void JumpToApp(void);	
void f_debug_jump(void);


/* fbl control */
#define  f_GenSoftwareReset 	  NVIC_GenerateSystemReset 
struct MesloopSt
{
  u8 En;/* 1 for enable,other for disable */
  u8 Type;/* 1 for TCP; 2 for RTU; 3 for ASCII; 4 for IO; 5 for ADC; 6 for RS232; 7 for RS485; 9-255 reserve*/
  u8 IPAddr[4];/* IP address */
  u8 StAddr;/* station address */
  u16 Port;/* port */
  u8 cycle;/* cycle*500ms */
};

extern struct                  MesloopSt mesLoop[500];
extern  u16                    g_us_Timer1s;
extern  u32                    g_ul_ReadLen;
extern  u8                     *g_uc_FlashPointer;
extern  u8                     g_uc_FlashReadFlg;
extern  u8                     g_uc_ExpectedTxSn; /* Block sequence counter */
extern  u32                    g_ul_AlreadyTxLen;
extern  u8                     g_uc_ErrorStatus;
extern  u8                     g_uc_linRxFifo[8];
extern  u8                     g_uc_CmdProcessing;
extern  u8                     applValidFlg;  
extern  u8                     McuAddr;
extern  u8                     time5s;

/* export functions */
void    f_PowerUpForHandShake(void);
u8      f_ProReq(void);
u8      f_AppValidChk(void);
void    f_JumpAppl(void);
void    f_console_Task(void);
void    SetFblMode(u8 state);	
void    Print(u8 *str);	
void    secrial_recv(u8 tst);

#endif
