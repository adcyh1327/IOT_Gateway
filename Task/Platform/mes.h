#include "main.h"

/*modbus tcp slave*/
extern u8  g_u8TcpSlaveLclIP[4]; 
extern const u8  g_u8TcpSlaveLclMac[6];	 
extern const u8  g_u8TcpSlaveRmtIP[4];	
extern const u8  g_u8TcpSlaveGw[4];						 	
extern const u8  g_u8TcpSlaveSub[4];
extern const u16 g_u16TcpSlaveRmtPort ;	  
extern  u16 g_u16TcpSlaveLclPort[8];	
extern const u16 g_u16TcpSlaveLclPortDwn;				  
/*modbus tcp master*/
extern  u8  g_u8TcpMasterLclIP[4];
extern  u8  g_u8TcpMasterRmtIP[4];	   
extern const u8  g_u8TcpMasterGw[4];
extern const u8  g_u8TcpMasterSub[4]; 
extern  u16 g_u16TcpMasterRmtPort[8];  
extern  u16 g_u16TcpMasterLclPort[8];



