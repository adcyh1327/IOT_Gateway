#include "Task_ModbusTCP.h"
#include "socket.h"	// Just include one header for WIZCHIP
#include "dhcp.h"
#include "main.h"
#include "wizchip_conf.h"
#include "Task_LED.h"
/* -------------------------------- web --------------------------------------*/
#include "httputil.h"
#define MBDATA_BUF_SIZE                  280//mb�����Ķ����Ϊ127���֣���֡����ֽ��������ܳ�����ֵ��������������

#define TCPSEND_IDLE                         0u
#define TCPSEND_WAIT_ACK                     1u

#define TCPSEND_TIMEOUT                    5000//��ʱʱ��Ϊ5s��һ�������򱨾�һ��

#define STKSIZE_MBTCP_SEND            STK_SIZE_384

OS_STK STK_MBTCP_SEND[STKSIZE_MBTCP_SEND];

static uint8_t aucTCPBuf[MBDATA_BUF_SIZE];	 //�������ݻ���
static uint8_t TCP_TR_data[MBDATA_BUF_SIZE]; //���ͻ�����
static u16 MBTCP_Count; //֡�����������������û��һ֡������1
static u8 usartcan_chn;//ͨ����
static u16 Polltimer;//poll��ʱ������clientʱ�谴������ʱ�����ڶ�server�ļĴ���
static u16 TO_Timer;//��ʱ������

volatile Tdef_Byte _ModbusTCP_Status;
#define ModbusTCP_Status                 _ModbusTCP_Status.Byte
#define MBTCP_SendStatus                 _ModbusTCP_Status.Bits.bit0  //����״̬��0-����  1-���������ݣ��ȴ��Է���Ӧ

OS_EVENT *EV_MBTCPsendbuf;           //��̫������

void MBTCP_Send_task(void *arg);

void MBTCP_Init(void)
{
    EV_MBTCPsendbuf = OSMboxCreate((void*)0);
    MBTCP_Count = 0x0001;
    usartcan_chn = 0;
    Polltimer = gWIZNETINFO.polltime;
}

void MBTCP_Timer1ms(void)
{
    if((gWIZNETINFO.conn_status)&&(gWIZNETINFO.session_mode==S_mb_client))
    {
        if(MBTCP_SendStatus == TCPSEND_IDLE)
        {
            if(Polltimer>0)//�ñ�����Ӧ���и�ֵ��һ��Ϊ��flash�ڶ�ȡ������
            {
                Polltimer--;
            }
        }
        else
        {
            if(TO_Timer>0)
            {
                TO_Timer--;
            }
            else
            {
                MBTCP_SendStatus = TCPSEND_IDLE;
                Polltimer = gWIZNETINFO.polltime;
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
        }
    }
    
}

void Modbus_Server(u8 *aucTCPBuf,u16 usRecBufLen)
{
    u16 usTxLen,cnt;
    u8 chn;
    unsigned short int usStartAddr,usNumber;
   
    if(usRecBufLen > 0) //����������ݳ���Ϊ0.�򷵻�
    {
        usStartAddr = (u16)(aucTCPBuf[8]<<8);
        usStartAddr +=(u16)(aucTCPBuf[9]);
        
        cnt = ((aucTCPBuf[0])<<8)+aucTCPBuf[1];//��ȡ�յ���֡������
        if(cnt==MBTCP_Count)
        {
            //��˵�±ߵĳ�����Ӧ�÷��ڴ˴��ˣ���ֹ�ֳ�Э��ƫ����ɲ��������������Ƚ��������ɴ���
        }
        else
        {
             LED_BLINK_ONCE(S_FAULT);
            //errorcode
        }
        MBTCP_Count = (++cnt);

        if(0x03 == aucTCPBuf[7]) //���Ĵ�������
        {    
            usNumber = (u16)(aucTCPBuf[10]<<8);
            usNumber +=(u16)(aucTCPBuf[11]);//��ȡ�����������
            usTxLen=usNumber<<1;
            if(usStartAddr<USARTCAN.addr)//��ȡ��ַ�Ƿ���������Χ��
            {
                aucTCPBuf[4] = 0;
                aucTCPBuf[5] = 3;
                aucTCPBuf[7] = aucTCPBuf[7] | 0x80;
                aucTCPBuf[8] = 0x02;
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
            else if((usStartAddr+usNumber)>(USARTCAN.addr+(USARTCAN.datalen*NUM_UARTCAN)))//����һ���ֳ�����Χ
            { 
                aucTCPBuf[4] = 0;
                aucTCPBuf[5] = 3;
                aucTCPBuf[7] = aucTCPBuf[7] | 0x80;
                aucTCPBuf[8] = 0x03;
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
            else
            {                
                memcpy(&aucTCPBuf[9],&TCP_TR_data[(usStartAddr-USARTCAN.addr)*2],usNumber*2);//�������
			
                aucTCPBuf[8]=(u8)usTxLen;
                usTxLen += 3;	 
                aucTCPBuf[4]=(u8)(usTxLen>>8);	 
                aucTCPBuf[5]=(u8)usTxLen;
            }
        }
        else if(0x06 == aucTCPBuf[7])//д�����Ĵ�������Ҫ���ڴ��ڻ�CANת��̫��ʱ�����ݸ��º�client������д�ñ�־λΪ0
        {	
            usNumber = 1;			
            if(usStartAddr<USARTCAN.addr)
            {
                aucTCPBuf[4] = 0;
                aucTCPBuf[5] = 3;
                aucTCPBuf[7] = aucTCPBuf[7] | 0x80;
                aucTCPBuf[8] = 0x02;
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
            else if((usStartAddr+usNumber)>(USARTCAN.addr+(USARTCAN.datalen*NUM_UARTCAN)))//
            {
                aucTCPBuf[4] = 0;
                aucTCPBuf[5] = 3;
                aucTCPBuf[7] = aucTCPBuf[7] | 0x80;
                aucTCPBuf[8] = 0x03;
                usTxLen = 9;
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
            else
            {
                aucTCPBuf[4] = 0;
                aucTCPBuf[5] = 7;
                aucTCPBuf[7] = aucTCPBuf[7];  	
                memcpy(&TCP_TR_data[(usStartAddr-USARTCAN.addr)*2],&aucTCPBuf[10],usNumber*2);
            }
        }
        else if(0x10 == aucTCPBuf[7])//д����Ĵ�������Ҫ����client������ת���ɴ��ڻ�CAN
        {	
            usNumber = (u16)(aucTCPBuf[10]<<8);
            usNumber +=(u16)(aucTCPBuf[11]);
            if(usStartAddr<USARTCAN.addr)
            {
                aucTCPBuf[4] = 0;
                aucTCPBuf[5] = 3;
                aucTCPBuf[7] = aucTCPBuf[7] | 0x80;
                aucTCPBuf[8] = 0x02;
                LED_BLINK_ONCE(S_FAULT);
                //errorcode

            }
            else if((usStartAddr+usNumber)>(USARTCAN.addr+(USARTCAN.datalen*NUM_UARTCAN)))
            {
                aucTCPBuf[4] = 0;
                aucTCPBuf[5] = 3;
                aucTCPBuf[7] = aucTCPBuf[7] | 0x80;
                aucTCPBuf[8] = 0x03;
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
            else
            {
                aucTCPBuf[4] = 0;
                aucTCPBuf[5] = 6;
                aucTCPBuf[7] = aucTCPBuf[7];
                if((aucTCPBuf[13]==0x00)&&(aucTCPBuf[14]==0x01))//��̫���򴮿�ת����
                {
                    if((usStartAddr-USARTCAN.addr)%USARTCAN.datalen==0)//ֻ�������ʼ��ַ��˿ڶ�Ӧ��ƫ�Ƶ�ַ��ʼ�����򱨴�
                    {
                        chn=(usStartAddr-USARTCAN.addr)/USARTCAN.datalen;
                        if(chn<NUM_UARTCAN)//�˿��Ƿ���Ч
                        {
                            ETH2Usartcan_send(chn,&aucTCPBuf[17],aucTCPBuf[16]);//��mb��˵����ֻ��һ���ֽ�
                            aucTCPBuf[14] = 0x00;
                            memcpy(&TCP_TR_data[(usStartAddr-USARTCAN.addr)*2],&aucTCPBuf[13],usNumber*2);
                        }
                        else
                        {
                            LED_BLINK_ONCE(S_FAULT);
                            //errorcode
                        }
                    }
                    else
                    {
                        LED_BLINK_ONCE(S_FAULT);
                        //errorcode
                    }
                }
            }
        }
        else
        {
            aucTCPBuf[4] = 0;
            aucTCPBuf[5] = 3;
            aucTCPBuf[7] = aucTCPBuf[7] | 0x80;
            aucTCPBuf[8] = 0x01;
            LED_BLINK_ONCE(S_FAULT);
            //errorcode
        }
        OSMboxPost(EV_MBTCPsendbuf,(void *)&aucTCPBuf[0]);//��������
    }
}

void Task_ModbusTCP(void *p_arg)
{
    
    struct wiz_NetInfo_t *mbtcpparm;
    unsigned int len = 0;
    int32_t ret = 0;
    int8_t sock_status,tmp;
    
    mbtcpparm = (struct wiz_NetInfo_t *)p_arg;
    MBTCP_Init();

    if(mbtcpparm->session_mode == S_mb_server)
    {
        //�½�һ��Socket���󶨱��ض˿�5000
        ret = socket(mbtcpparm->tcpsocket, Sn_MR_TCP, mbtcpparm->portlocal, Sn_MR_ND);
        if(ret != mbtcpparm->tcpsocket) {
            ///printf("%d:Socket Error\r\n",SOCK_TCPS);
            //		while(1);
            LED_BLINK_ONCE(S_FAULT);
            //errorcode
        } else {
            ///printf("%d:Opened\r\n",SOCK_TCPS);
        }
    }
    else
    {
			  //�½�һ��Socket���󶨱��ض˿�5000	
        //Ethernet_OpenClient(SOCK_MODBUSTCP,(uint16_t)MBTCP_Network.local_port,MBTCP_Network.remote_ip,(uint16_t)MBTCP_Network.remote_port);
    }
    
    OSTaskCreate(MBTCP_Send_task, (void *)p_arg, (OS_STK*)&STK_MBTCP_SEND[STKSIZE_MBTCP_SEND-1], TASK_PRIO_MBTCP_SEND);
    
    while (1)
    {
        ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);
        if (tmp == PHY_LINK_OFF)
        {/*������ߴ���*/
            mbtcpparm->conn_status = OFF;
            close(mbtcpparm->tcpsocket);
            //errorcode
        }
        
        switch(getSn_SR(mbtcpparm->tcpsocket))	//��ȡsocket0��״̬
        {
            //socket��ʼ�����
            case SOCK_INIT:
                mbtcpparm->conn_status = OFF;
                //errorcode
	            if(mbtcpparm->session_mode == S_mb_server) 
                {
	                listen(mbtcpparm->tcpsocket);	//����TCP�ͻ��˳�������
	            }
                else
                {
	                ret = connect(mbtcpparm->tcpsocket, mbtcpparm->ipgoal, mbtcpparm->portgoal);
                    if(ret != ON)
                    {
                        LED_BLINK_ONCE(S_FAULT);
                        //errorcode
                    }
                    OSTimeDlyHMSM(0, 0, 0, 100);
	            }
	            break;
            //socket���ӽ���
            case SOCK_ESTABLISHED:
                mbtcpparm->conn_status = ON;
                if(getSn_IR(mbtcpparm->tcpsocket) & Sn_IR_CON) 
                {
                  setSn_IR(mbtcpparm->tcpsocket, Sn_IR_CON);
                }
                              
                if(getSn_RX_RSR(mbtcpparm->tcpsocket) > 0)	//�±ߵĽ��պ���Ϊ�����ͣ����жϳ����Ƿ�Ϊ0������0���ܽ����պ�����
                {
                    len = recv(mbtcpparm->tcpsocket, aucTCPBuf, sizeof(aucTCPBuf));
                    if(mbtcpparm->session_mode == S_mb_server) 
                    {
                        if((u8)mbtcpparm->stationID == aucTCPBuf[6]) //MB վ��ַֻ��һ���ֽڣ��Ҳ����㲻���κλظ�
                        {
                            Modbus_Server(aucTCPBuf,len);//��serverʱ��������������
                        }
                        else
                        {
                            LED_BLINK_ONCE(S_FAULT);
                            //errorcode
                        }
                    }
                    else
                    {
                        if(MBTCP_SendStatus == TCPSEND_WAIT_ACK)//����ǰΪ���������ݺ��յ���Ӧ
                        {
                            MBTCP_SendStatus = TCPSEND_IDLE;
                            Polltimer = mbtcpparm->polltime;
                            if(((MBTCP_Count>>8)==aucTCPBuf[0])&&((u8)MBTCP_Count==aucTCPBuf[1]))
                            {
                                //��˵�±ߵĳ�����Ӧ�÷��ڴ˴��ˣ���ֹ�ֳ�Э��ƫ����ɲ��������������Ƚ��������ɴ���
                            }
                            else
                            {
                                 LED_BLINK_ONCE(S_FAULT);
                                //errorcode
                            }
                            if (0x80 != (aucTCPBuf[7]&0x80))
                            {//�϶���Ӧ
                                if((aucTCPBuf[9]==0x00)&&(aucTCPBuf[10]==0x01))//��̫���򴮿�ת����
                                {//���յ�server�������ݸ��µı�ʶ������д����������
                                    ETH2Usartcan_send(usartcan_chn,&aucTCPBuf[13],aucTCPBuf[12]);//��mb��˵����ֻ��һ���ֽ�
                                    //MBTCP ֡������
                                    memset(aucTCPBuf,0,sizeof(aucTCPBuf));
                                    MBTCP_Count++;
                                    aucTCPBuf[0] = (MBTCP_Count >> 8) & 0xFF;
                                    aucTCPBuf[1] = (MBTCP_Count) & 0xFF;
                                    //MBTCP �̶�Ϊ0
                                    aucTCPBuf[2] = 0;
                                    aucTCPBuf[3] = 0;
                                    //MBTCP ֡���ٴ����ĳ���
                                    aucTCPBuf[4] = 0x00;
                                    aucTCPBuf[5] = 6;//�̶�ֵ
                                    //MBTCP ջ��ID
                                    aucTCPBuf[6] = mbtcpparm->stationID;
                                    //MBTCP �����룺06������д����Ĵ������
                                    aucTCPBuf[7] = 0x06;
                                    //MBTCP ֡��д����ʼ��ַ��
                                    aucTCPBuf[8] = ((USARTCAN.addr+(USARTCAN.datalen*usartcan_chn)) >> 8) & 0XFF;
                                    aucTCPBuf[9] =  (USARTCAN.addr+(USARTCAN.datalen*usartcan_chn)) & 0XFF;
                                    //MBTCP ֡��д�ļĴ�������
                                    aucTCPBuf[10] = 0x00;
                                    aucTCPBuf[11] =	0x00;
                                    MBTCP_SendStatus = TCPSEND_WAIT_ACK;
                                    TO_Timer = TCPSEND_TIMEOUT;
                                    OSMboxPost(EV_MBTCPsendbuf,(void *)&aucTCPBuf[0]);//��������
                                }
                            }
                            else
                            {//����Ӧ
                                LED_BLINK_ONCE(S_FAULT);
                                //errorcode
                            }
                        }
                        else
                        {//�쳣��Ӧ
                            LED_BLINK_ONCE(S_FAULT);
                                //errorcode
                        }
                    }
                }
                
                if(mbtcpparm->session_mode == S_mb_client) 
                {
                    if((MBTCP_SendStatus == TCPSEND_IDLE)&&(Polltimer==0))
                    {//��clientʱ��Ҫ��server�����ڵĽ���poll
                        //MBTCP ֡������
                        memset(aucTCPBuf,0,sizeof(aucTCPBuf));
                        MBTCP_Count++;
                        aucTCPBuf[0] = (MBTCP_Count >> 8) & 0xFF;
                        aucTCPBuf[1] = (MBTCP_Count) & 0xFF;
                        //MBTCP �̶�Ϊ0
                        aucTCPBuf[2] = 0;
                        aucTCPBuf[3] = 0;
                        //MBTCP ֡���ٴ����ĳ��ȣ���ʵ����*2+7
                        aucTCPBuf[4] = 0x00;
                        aucTCPBuf[5] = 6;//�̶�ֵ
                        //MBTCP ջ��ID����ʵû�ã������01
                        aucTCPBuf[6] = mbtcpparm->stationID;
                        //MBTCP �����룺16������д����Ĵ������
                        aucTCPBuf[7] = 0x03;
                        //MBTCP ֡��д����ʼ��ַ��
                        usartcan_chn++;
                        if(usartcan_chn>=NUM_UARTCAN)
                        {
                            usartcan_chn = 0;
                        }
                        aucTCPBuf[8] = ((USARTCAN.addr+(USARTCAN.datalen*usartcan_chn)) >> 8) & 0XFF;
                        aucTCPBuf[9] =  (USARTCAN.addr+(USARTCAN.datalen*usartcan_chn)) & 0XFF;
                        //MBTCP ֡�ж��ļĴ�������
                        aucTCPBuf[10] = (USARTCAN.datalen >> 8) & 0XFF;
                        aucTCPBuf[11] =	USARTCAN.datalen & 0XFF;
                        MBTCP_SendStatus = TCPSEND_WAIT_ACK;
                        TO_Timer = TCPSEND_TIMEOUT;
                        OSMboxPost(EV_MBTCPsendbuf,(void *)&aucTCPBuf[0]);//��������
                    }
                }
                
            	break;
        //socket�ȴ��ر�״̬
            case SOCK_CLOSE_WAIT:
                //errorcode
                mbtcpparm->conn_status = OFF;
                close(mbtcpparm->tcpsocket);
            	break;
        //socket�ر�
            case SOCK_CLOSED:
                //errorcode
                mbtcpparm->conn_status = OFF;
                sock_status=0xff;
                while(sock_status!=mbtcpparm->tcpsocket)
                {
                    sock_status = socket(mbtcpparm->tcpsocket, Sn_MR_TCP, mbtcpparm->portlocal, Sn_MR_ND);
                }
                
                break;
//             case SOCK_LISTEN:
//                 break;

//             case SOCK_SYNSENT:
//                 break;

//             case SOCK_SYNRECV:
//                 break;

//             case SOCK_FIN_WAIT:
//                 break;

//             case SOCK_CLOSING:
//                 break;

//             case SOCK_TIME_WAIT:
//                 break;

//             case SOCK_LAST_ACK:
//                 break;

//             case SOCK_UDP:
//                 break;
//                 
//             case SOCK_MACRAW:
//                 break;
            default:
                mbtcpparm->conn_status = OFF;
                //errorcode
            //connect(mbtcpparm->tcpsocket, gWIZNETINFO.ipgoal, gWIZNETINFO.portgoal);
                break;
        }
        OSTimeDlyHMSM(0, 0, 0, 10);
    }
}


void MBTCP_DataHandler(u8 chnidx,struct USARTCAN_Recv_t *recv,void *arg)
{
    struct wiz_NetInfo_t *mbtcpparm;
    mbtcpparm = (struct wiz_NetInfo_t *)arg;
    u16 data_off;
    data_off=USARTCAN.datalen * 2 * chnidx;
    memset(&TCP_TR_data[data_off],0,recv->lenth);
    TCP_TR_data[data_off++] = 0x00;
    TCP_TR_data[data_off++] = ON;    //��ͨ�������ݸ���ʱ��һ���̶ֹ�Ϊ0x0001����Ϊserver��Ҫclientд0���
    TCP_TR_data[data_off++] = (recv->lenth)>>8;
    TCP_TR_data[data_off++] = (recv->lenth)&0xff;    //�����ڻ�CAN�յ������ݳ���
    memcpy(&TCP_TR_data[data_off],recv->databuf,recv->lenth);
    
    if(mbtcpparm->session_mode == S_mb_client)
    {//�����ڻ�CAN�յ�������ͨ��д������server�ˣ�ֻ��дһ��
        //MBTCP ֡������
        memset(aucTCPBuf,0,sizeof(aucTCPBuf));
        MBTCP_Count++;
        aucTCPBuf[0] = (MBTCP_Count >> 8) & 0xFF;
        aucTCPBuf[1] = (MBTCP_Count) & 0xFF;
        //MBTCP �̶�Ϊ0
        aucTCPBuf[2] = 0;
        aucTCPBuf[3] = 0;
        //MBTCP ֡���ٴ����ĳ��ȣ���ʵ����*2+7
        aucTCPBuf[4] = ((USARTCAN.datalen * 2 + 7) >> 8) & 0XFF;
        aucTCPBuf[5] = (USARTCAN.datalen * 2 + 7) & 0XFF;
        //MBTCP ջ��ID
        aucTCPBuf[6] = mbtcpparm->stationID;
        //MBTCP �����룺16������д����Ĵ������
        aucTCPBuf[7] = 0x10;
        //MBTCP ֡��д����ʼ��ַ��
        aucTCPBuf[8] = ((USARTCAN.addr+(USARTCAN.datalen*chnidx)) >> 8) & 0XFF;
        aucTCPBuf[9] =  (USARTCAN.addr+(USARTCAN.datalen*chnidx)) & 0XFF;
        //MBTCP ֡��д�ļĴ�������
        aucTCPBuf[10] = (USARTCAN.datalen >> 8) & 0XFF;
        aucTCPBuf[11] =	USARTCAN.datalen & 0XFF;
        //MBTCP ֡��д���ֽ�����
        aucTCPBuf[12] = (USARTCAN.datalen * 2) & 0XFF;
        TCP_TR_data[(USARTCAN.datalen * 2 * chnidx)+1]=0;//Ϊ��ʵ��˫��ͨѶ�ҹ���ַ���˴��������⴦������־λǿ���������Ϊֻ��дһ�Σ���Ӱ�칦�ܵ�ʶ��
        memcpy(&aucTCPBuf[13],&TCP_TR_data[USARTCAN.datalen * 2 * chnidx],recv->lenth+4);
        MBTCP_SendStatus = TCPSEND_WAIT_ACK;
        TO_Timer = TCPSEND_TIMEOUT;
        OSMboxPost(EV_MBTCPsendbuf,(void *)&aucTCPBuf[0]);//��������
    }		
}

void MBTCP_Send_task(void *arg)
{
	u8 *sedmsg;
	u8 err;
    struct wiz_NetInfo_t *mbtcpparm;

    mbtcpparm = (struct wiz_NetInfo_t *)arg;
	while(1)
	{
		sedmsg=(u8*)OSMboxPend(EV_MBTCPsendbuf,0,&err);
		if(err == OS_ERR_NONE)//��Ϣ���䷢��
		{
            while(!mbtcpparm->conn_status)
            {
                if(MBTCP_SendStatus == TCPSEND_IDLE)
                {
                    LED_BLINK_ONCE(S_FAULT);
                    //errorcode
                    break;
                }
                OSTimeDlyHMSM(0, 0, 0, 2);
            }
            if(mbtcpparm->conn_status)
            {
                send(mbtcpparm->tcpsocket, sedmsg, sedmsg[5]+6);
            }
		}
        OSTimeDlyHMSM(0, 0, 0, 1);
	}
}















