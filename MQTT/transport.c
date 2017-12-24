/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - "commonalization" from prior samples and/or documentation extension
 *******************************************************************************/

// #include <sys/types.h>

#if !defined(SOCKET_ERROR)
	/** error in socket operation */
	#define SOCKET_ERROR -1
#endif

#if defined(WIN32)
/* default on Windows is 64 - increase to make Linux and Windows the same */
#define FD_SETSIZE 1024
#include <winsock2.h>
#include <ws2tcpip.h>
#define MAXHOSTNAMELEN 256
#define EAGAIN WSAEWOULDBLOCK
#define EINTR WSAEINTR
#define EINVAL WSAEINVAL
#define EINPROGRESS WSAEINPROGRESS
#define EWOULDBLOCK WSAEWOULDBLOCK
#define ENOTCONN WSAENOTCONN
#define ECONNRESET WSAECONNRESET
#define ioctl ioctlsocket
#define socklen_t int
#else
#define INVALID_SOCKET SOCKET_ERROR
// #include <sys/socket.h>
// #include <sys/param.h>
// #include <sys/time.h>
// #include <netinet/in.h>
// #include <netinet/tcp.h>
// #include <arpa/inet.h>
// #include <netdb.h>
// #include <stdio.h>
// #include <unistd.h>
// #include <errno.h>
// #include <fcntl.h>
// #include <string.h>
// #include <stdlib.h>
#endif

#if defined(WIN32)
#include <Iphlpapi.h>
#else
// #include <sys/ioctl.h>
// #include <net/if.h>
#endif

#include "MQTTPacket.h"
#include "httputil.h"
#include "main.h"
#include "transport.h"
#include "wizchip_conf.h"

/**
This simple low-level implementation assumes a single connection for a single thread. Thus, a static
variable is used for that connection.
On other scenarios, the user must solve this by taking into account that the current implementation of
MQTTPacket_read() has a function pointer for a function call to get the data to a buffer, but no provisions
to know the caller or other indicator (the socket id): int (*getfn)(unsigned char*, int)
*/

int transport_getdata(unsigned char* buf, int count)
{
//     uint16_t len=0;
// 					// 数据回环测试程序：数据从上位机服务器发给W5500，W5500接收到数据后再回给服务器
// 					len=getSn_RX_RSR(0);										// len=Socket0接收缓存中已接收和保存的数据大小
// 					if(len>0)
// 					{
return						recv(gWIZNETINFO.tcpsocket,buf,count);										
// 					}    
//  return recv(SOCK_MQTT,buf,count);
}

int8_t transport_close(u8 sn)
{
  close(sn);
  return 0;
}

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
int MQTT_Connect(u8 sn)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	int len;
	int rc = 0;
	unsigned char buf[200];
	int buflen = sizeof(buf);

	data.clientID = MQTT_CFG_Connect.Client_id;
// 	data.username.cstring = MQTT_CFG_Connect.UserName;
// 	data.password.cstring = MQTT_CFG_Connect.PassWard;	
    data.will.qos = MQTT_CFG_Connect.con_qos;
	data.keepAliveInterval = MQTT_CFG_Connect.kpalivespace;	
	data.cleansession = 1;
 	
	len = MQTTSerialize_connect(buf, buflen, &data);	
	rc  = Ethernet_SendBuffer(sn,buf, len);
	if(rc != len){
		return 0;
  }

	if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK)
	{
		unsigned char sessionPresent, connack_rc;
       //*将提供的数据与请求数据并行化
		if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
			return 0;
		else return 1;
    }
	return 0;
}

/**
  * @brief  向代理（服务器）发送一个消息
  * @param  pTopic 消息主题
  * @param  pMessage 消息内容
  * @retval 小于0表示发送失败
  */
u8 Pubbuf[4096];
u8 Subbuf[4096];

int mqtt_publish(u8 sn,struct MQTT_Topic_Info_t* pubmsg)
{	
    int32_t len;
    int ret;
    int buflen = sizeof(Pubbuf);
    len = MQTTSerialize_publish(Pubbuf, buflen, 0, pubmsg->qos, 0,pubmsg->packid, pubmsg->topic_name, pubmsg->msgdata, pubmsg->msglen); /* 2 */
  
    //通过W5500数据发送接口发送数据
    ret = Ethernet_SendBuffer(sn, Pubbuf, len);
	if(ret != len) {
		ret = 0;
    } else {
		ret = 1;
    }
    return ret;
}



int mqtt_subscrib(u8 sn,struct MQTT_Topic_Info_t* submsg,u8 topic_num)
{
	int len,rc;
	unsigned int buflen = sizeof(Subbuf);
    int req_qos;
    
    req_qos = submsg->qos;
    /* subscribe */
	len = MQTTSerialize_subscribe(Subbuf, buflen, 0, submsg->packid, 1, &submsg->topic_name, &req_qos);    
	//通过W5500数据发送接口发送数据
    rc = Ethernet_SendBuffer(sn, Subbuf, len);
    if(rc != len)
    {
        //prinf("connect transport_sendPacketBuffer error\n\r");
        rc = 0;  
    }										
    else
    {
        rc = 1;
    }
    return rc;
}

int mqtt_pingreq(uint8_t sn)
{
	int32_t len,rc;
    unsigned char ret;
	unsigned char buf[10];
	int buflen = sizeof(buf);
	len = MQTTSerialize_pingreq(buf, buflen); /* 1 */
    rc = Ethernet_SendBuffer(sn, buf, len);
    if(rc == len)
    { 	 
        ret=1;
	}
	ret = MQTTPacket_read(buf, buflen, transport_getdata); 
	if (buf[0]  == 0xD0)
    {   
        ret=1;
    }	
    else
    {   
        ret = 0;
        transport_close(sn);  //mqtt断开	
    }
       	 
	  return ret;
}


