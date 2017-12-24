#include "main.h"
#include "spi.h"
#include "httputil.h"
#include "wizchip_conf.h"



void Ethernet_Init(void)
{
    uint8_t tmp;
	uint8_t tmpstr[6];
    //���ý��ջ���ͷ��ͻ��涼Ϊ 2KB
    uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
    // First of all, Should register SPI callback functions implemented by user for accessing WIZCHIP
    /* Critical section callback */
    reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);	// ע���ٽ�������
    /* Chip selection call back */
#if   _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_
    reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);  // ע��SPIƬѡ�źź���
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
    reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);	// ע���д����

    /* WIZCHIP SOCKET Buffer initialize */ //���ý��ջ���ͷ��ͻ��涼Ϊ 2KB
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
    

    // Display Network Information  //��ʾ����оƬID
    ctlwizchip(CW_GET_ID, (void *)tmpstr);
    
    #ifndef DEBUG_ENABLE
    ctlnetwork(CN_SET_TIMEOUT, (void *)&gWIZNetTimeout);
    setSn_KPALVTR(gWIZNETINFO.tcpsocket, 0x01);
    #endif
    
}

/*
 * ��������W5500_StatusDetect
 * ����  ��W5500ͨ��SPI��ȡmac/ip/sn/gw���ݣ��ж��Ƿ��Ԥ��ֵһ�£���ֹ��;оƬ�Լ���λ�������ͬ�������³�ʼ��оƬ
 * ����  ����
 * ���  ����
 * ˵��  ����
 */
void W5500_StatusDetect(void)
{
    u8 i,j;
    wiz_NetInfo tempstr;
    #ifdef W5500_ENABLE
        ctlnetwork(CN_GET_NETINFO, (void *)&tempstr);
        for(i=0;i<4;i++)
        {//�κ�һ����ƥ�������
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
            W5500_SPI_Config();//���IO��SPI��ʼ��������Ӳ��reset
            Ethernet_Init();//��ؽӿ�ע���Լ��������д���
        }
    #endif
}


/*
 * ��������Ethernet_Getdata
 * ����  ��W5500��ָ�����׽��ֶ˿ڻ�ȡ����
 * ����  ����
 * ���  ����
 * ˵��  ����
 */
int32_t Ethernet_Getdata(int sock,  unsigned char* buf, int count)
{
    return recv(sock,buf,count);	
}


/*
 * ��������Ethernet_SendBuffer
 * ����  ��������ͨ��W5500���͵�ָ���׽��ֶ˿�
 * ����  ����
 * ���  ����
 * ˵��  ����
 */
int32_t Ethernet_SendBuffer(int sock, unsigned char* buf, int buflen)
{
    return send(sock,buf,buflen);
}

/*
 * ��������Ethernet_OpenClient
 * ����  �������ͻ����׽��֣�������Ŀ�������
 * @param��sn�׽��ֱ�š�port���ض˿ڡ�portgoalĿ��˿ڡ�ipgoalĿ��IP��ַ
 * ���  ����
 * ˵��  ����
 */
int8_t Ethernet_OpenClient(uint8_t sn, uint16_t port, uint8_t* ipgoal, uint16_t portgoal)
{
    int8_t ret;
	
    //�½�һ��Socket����ָ���˿�
    ret = socket(sn,Sn_MR_TCP,port,0x00);
    if(ret != sn){
        //prinf("%d:Socket Error\r\n",SOCK_TCPS);
        while(1);
    }
	
	//����TCP������
//	delay_ms(1000);
//    OSTimeDlyHMSM(0, 0, 0, 500);
    ret = connect(sn,ipgoal,portgoal);
    if(ret != SOCK_OK){
        while(1);
    }
	return ret;
}

/*
 * ��������Ethernet_Close
 * ����  ���ر�ָ���׽��ֶ˿�
 * ����  ����
 * ���  ����
 * ˵��  ����
 */
int8_t Ethernet_Close(int sock)
{
    return close(sock);
}


