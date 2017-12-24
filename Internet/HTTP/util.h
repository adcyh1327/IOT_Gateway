/*
*
@file		util.h
@brief	
*/

#ifndef _UTIL_H
#define _UTIL_H

void Delay_us(unsigned char time_us);
void Delay_ms(unsigned int time_ms);


unsigned int ATOI(char* str,unsigned int base); 			/* Convert a string to integer number */
unsigned long ATOI32(char* str,unsigned int base); 			/* Convert a string to integer number */
void itoa(unsigned int n,unsigned char* str, unsigned char len);
int ValidATOI(char* str, int base, int* ret); 		/* Verify character string and Convert it to (hexa-)decimal. */
char C2D(unsigned char c); 					/* Convert a character to HEX */

unsigned int swaps(unsigned int i);
unsigned long swapl(unsigned long l);

void replacetochar(char * str, char oldchar, char newchar);

void mid( char* src,  char* s1,  char* s2,  char* sub);
void inet_addr_(unsigned char* addr,unsigned char *ip);
#endif
