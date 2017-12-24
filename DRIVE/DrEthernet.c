#include "main.h"
#include "spi.h"
#include "httputil.h"
#include "wizchip_conf.h"



void Ethernet_Init(void)
{
    uint8_t tmp;
	uint8_t tmpstr[6];
    //设置接收缓存和发送缓存都为 2KB
    uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
    // First of all, Should register SPI callback functions implemented by user for accessing WIZCHIP
    /* Critical section callback */
    reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);	// 注册临界区函数
    /* Chip selection call back */
#if   _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_
    reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);  // 注册SPI片选信号函数
#elif _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_
    reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);  // CS must be tried with LOW.
#else
#if (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SIP_) != _WIZCHIP_IO_MODE_SIP_
#error "Unknown _WIZCHIP_IO_MODE_"
#else
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
#endif
#endif
    /* SPI Read & Write callback function */
    reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);	// 注册读写函数

    /* WIZCHIP SOCKET Buffer initialize */ //设置接收缓存和发送缓存都为 2KB
    if(ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1)
    {
#ifdef serial_dug
        ///printf("WIZCHIP Initialized fail.\r\n");
#endif
        while(1);
    }

    /* PHY link status check */
    do
    {
        if(ctlwizchip(CW_GET_PHYLINK, (void *)&tmp) == -1)
        {
#ifdef serial_dug
            ///printf("Unknown PHY Link stauts.\r\n");
#endif
            while(1);
        }
    }
    while(tmp == PHY_LINK_OFF);
		
		
		
    ctlnetwork(CN_SET_NETINFO, (void *)&gWIZNETINFO);
    

    // Display Network Information  //显示网卡芯片ID
    ctlwizchip(CW_GET_ID, (void *)tmpstr);
    
    #ifndef DEBUG_ENABLE
    ctlnetwork(CN_SET_TIMEOUT, (void *)&gWIZNetTimeout);
    setSn_KPALVTR(gWIZNETINFO.tcpsocket, 0x01);
    #endif
    
}

/*
 * 函数名：W5500_StatusDetect
 * 描述  ：W5500通过SPI读取mac/ip/sn/gw数据，判断是否和预期值一致，防止中途芯片自己复位，如果不同，则重新初始化芯片
 * 输入  ：无
 * 输出  ：无
 * 说明  ：无
 */
void W5500_StatusDetect(void)
{
    u8 i,j;
    wiz_NetInfo tempstr;
    #ifdef W5500_ENABLE
        ctlnetwork(CN_GET_NETINFO, (void *)&tempstr);
        for(i=0;i<4;i++)
        {//任何一个不匹配均跳出
            if(tempstr.iplocal[i]!=gWIZNETINFO.iplocal[i])
            {
                break;
            }
            if(tempstr.sn[i]!=gWIZNETINFO.sn[i])
            {
                break;
            }
            if(tempstr.gw[i]!=gWIZNETINFO.gw[i])
            {
                break;
            }
        }
        for(j=0;j<6;j++)
        {
            if(tempstr.mac[j]!=gWIZNETINFO.mac[j])
            {
                break;
            }
        }
        if((i<4)||(j<6))
        {
            if(gWIZNETINFO.conn_status)
            {
                gWIZNETINFO.conn_status = OFF;
            }
            W5500_SPI_Config();//相关IO及SPI初始化，包括硬件reset
            Ethernet_Init();//相关接口注册以及网络参数写入等
        }
    #endif
}


/*
 * 函数名：Ethernet_Getdata
 * 描述  ：W5500从指定的套接字端口获取数据
 * 输入  ：无
 * 输出  ：无
 * 说明  ：无
 */
int32_t Ethernet_Getdata(int sock,  unsigned char* buf, int count)
{
    return recv(sock,buf,count);	
}


/*
 * 函数名：Ethernet_SendBuffer
 * 描述  ：将数据通过W5500发送到指定套接字端口
 * 输入  ：无
 * 输出  ：无
 * 说明  ：无
 */
int32_t Ethernet_SendBuffer(int sock, unsigned char* buf, int buflen)
{
    return send(sock,buf,buflen);
}

/*
 * 函数名：Ethernet_OpenClient
 * 描述  ：创建客户端套接字，并连接目标服务器
 * @param：sn套接字编号、port本地端口、portgoal目标端口、ipgoal目标IP地址
 * 输出  ：无
 * 说明  ：无
 */
int8_t Ethernet_OpenClient(uint8_t sn, uint16_t port, uint8_t* ipgoal, uint16_t portgoal)
{
    int8_t ret;
	
    //新建一个Socket并绑定指定端口
    ret = socket(sn,Sn_MR_TCP,port,0x00);
    if(ret != sn){
        //prinf("%d:Socket Error\r\n",SOCK_TCPS);
        while(1);
    }
	
	//连接TCP服务器
//	delay_ms(1000);
//    OSTimeDlyHMSM(0, 0, 0, 500);
    ret = connect(sn,ipgoal,portgoal);
    if(ret != SOCK_OK){
        while(1);
    }
	return ret;
}

/*
 * 函数名：Ethernet_Close
 * 描述  ：关闭指定套接字端口
 * 输入  ：无
 * 输出  ：无
 * 说明  ：无
 */
int8_t Ethernet_Close(int sock)
{
    return close(sock);
}


