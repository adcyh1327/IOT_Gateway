#ifndef __DRFLASH_H
#define __DRFLASH_H	  



/* ²Áflash */
unsigned char FlashErase(unsigned long l_u32StartAddr, unsigned char l_u32PageNum);
/* Ð´flash */
u8 FlashWriteData(u32 startaddr, u8 *data,u16 bytenum);
/* ¶Áflash */
u8 FlashReadData(u32 startaddr,u8 *target,u16 bytenum);

#endif


