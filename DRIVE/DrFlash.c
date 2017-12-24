#include "main.h" 




/* 读flash */
unsigned char FlashErase(unsigned long l_u32StartAddr, unsigned char l_u32PageNum)
{
    
    volatile FLASH_Status FLASHStatus;
    unsigned long EraseCounter  = 0x0;
    
    OS_ENTER_CRITICAL();
    FLASH_Unlock();/* 解锁函数 */	
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);/* 擦页 */	
	FLASHStatus = FLASH_COMPLETE;
    
	for (EraseCounter=0; (EraseCounter<l_u32PageNum)&&(FLASHStatus==FLASH_COMPLETE); EraseCounter++)	
	{/* 开始写数据 */							
		FLASHStatus = FLASH_ErasePage(l_u32StartAddr+(FLASH_PAGE_SIZE*EraseCounter));		
	}
    OS_EXIT_CRITICAL();
    
	return FLASHStatus;
}

/* 写flash */
unsigned char FlashWrite(unsigned long l_u32StartAddr, unsigned long *lp_u32Data, unsigned int l_u16Size)
{
    
    volatile FLASH_Status FLASHStatus;			
	unsigned long l_u32Address = 0x0;	
	unsigned long i = 0;
    
    OS_ENTER_CRITICAL();
	FLASH_Unlock();/* 解锁函数 */	
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);/* 擦页	*/
	FLASHStatus     = FLASH_COMPLETE;
	l_u32Address    = l_u32StartAddr;
    
	while((l_u32Address<(l_u32StartAddr+l_u16Size))&&(FLASHStatus==FLASH_COMPLETE))		
	{/* 检验数据是否出错	*/	
		FLASHStatus     = FLASH_ProgramWord(l_u32Address, lp_u32Data[i++]);		
		l_u32Address    = l_u32Address + 4;	
	}
    OS_EXIT_CRITICAL();

	return FLASHStatus;
    
}


u32 STMFLASH_ReadWord(u32 faddr)
{
	return *(vu8*)faddr; 
} 

u8 FlashWriteData(u32 startaddr, u8 *data,u16 bytenum)
{
    u8 ret;
    unsigned long i,temp[(bytenum/4)+1];
    u16 writenum;
    if(bytenum%4==0)
    {
        writenum=bytenum/4;
    }
    else
    {
        writenum=(bytenum/4)+1;
    }
    for(i=0;i<writenum;i++)
    {
      temp[i]=(u32)(data[4*i+3]<<24) + (u32)(data[4*i+2]<<16) + (u32)(data[4*i+1]<<8) + (u32)data[4*i+0];
    }
    ret=FlashWrite(startaddr,temp,bytenum);
    return ret;
}


u8 FlashReadData(u32 startaddr,u8 *target,u16 bytenum)
{
    u8 ret=1;
    u32 i;
    if(startaddr%4==0)
    {
        for(i=0;i<bytenum;i++)
        {
            target[i]=STMFLASH_ReadWord(startaddr);//
            startaddr++;//	
        }
        ret=0;
    }
    else
    {
        ret=1;
    }
    return ret;
}
