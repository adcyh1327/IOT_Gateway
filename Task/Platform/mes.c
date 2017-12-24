#include "main.h"

/*modbus tcp slave*/
u8  g_u8TcpSlaveLclIP[4]   ={192,168,  0, 60}; 
const u8  g_u8TcpSlaveLclMac[6]  ={0x48,0x53,0x00,0x57,0x00,0x01};	 
const u8  g_u8TcpSlaveRmtIP[4]   ={192,168,  0,251};	
const u8  g_u8TcpSlaveGw[4]      ={192,168,  0,  1};						 	
const u8  g_u8TcpSlaveSub[4]     ={255,255,255,  0};
const u16 g_u16TcpSlaveRmtPort   =502;	  
 u16 g_u16TcpSlaveLclPort[8]   ={502,503,504,505,506,507,508,60172};	
const u16 g_u16TcpSlaveLclPortDwn=503;				  
/*modbus tcp master*/
u8  g_u8TcpMasterLclIP[4] ={192,168,  1,66};
u8  g_u8TcpMasterRmtIP[4] ={192,168,  1,5};	   
const u8  g_u8TcpMasterGw[4]    ={192,168,  1,  1};
const u8  g_u8TcpMasterSub[4]   ={255,255,255,  0}; 
u16 g_u16TcpMasterRmtPort[8] ={502};  
u16 g_u16TcpMasterLclPort[8] ={502};




