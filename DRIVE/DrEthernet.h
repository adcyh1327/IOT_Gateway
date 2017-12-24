#ifndef __DR_ETHERNET_H
#define __DR_ETHERNET_H	  

#include "stm32f10x.h"

void Ethernet_Init(void);
int32_t Ethernet_Getdata(int sock,  unsigned char* buf, int count);
int32_t Ethernet_SendBuffer(int sock, unsigned char* buf, int buflen);
int8_t Ethernet_OpenClient(uint8_t sn, uint16_t port, uint8_t* ipgoal, uint16_t portgoal);
int8_t Ethernet_Close(int sock);
void W5500_StatusDetect(void);//×´Ì¬¼à²â

#endif




