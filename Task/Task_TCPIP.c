
#include "main.h"
#include "Task_TCPIP.h"
#include "socket.h"	// Just include one header for WIZCHIP
#include "dhcp.h"
#include "wizchip_conf.h"
#include "Task_LED.h"
/* -------------------------------- web --------------------------------------*/
#include "httputil.h"
#define TCPIPDATA_BUF_SIZE                       1024


#define TCPSEND_IDLE                         0u
#define TCPSEND_WAIT_ACK                     1u

#define TCPSEND_TIMEOUT                    5000//��ʱʱ��Ϊ5s
#define TCP_HEARTBEAT                      6000//��������

#define STKSIZE_TCPIP_SEND            STK_SIZE_384
OS_STK STK_TCPIP_SEND[STKSIZE_TCPIP_SEND];

static uint8_t aucTCPBuf[TCPIPDATA_BUF_SIZE];	 //�������ݻ���
static uint8_t TCPIP_TR_data[TCPIPDATA_BUF_SIZE];//�������ݻ���
static u16 TCPIP_Count;//֡������
static u16 HeartbeatTimer;//������ʱ
static u16 TO_Timer;//��ʱ��ʱ

volatile Tdef_Byte _TCPIP_Status;
#define TCPIP_Status                     _TCPIP_Status.Byte
#define TCPIP_SendStatus                 _TCPIP_Status.Bits.bit0  //����״̬

OS_EVENT *EV_TCPIPsendbuf;           //tcpip���ķ���

void TCPIP_Send_task(void *arg);


void TCPIP_Init(void)
{
    EV_TCPIPsendbuf = OSMboxCreate((void*)0);
    TCPIP_Count = 1;
    HeartbeatTimer = TCP_HEARTBEAT;
}

void TCPIP_Timer1ms(void)
{
    if((gWIZNETINFO.conn_status)&&(gWIZNETINFO.session_mode==S_tcpip_client))
    {
        if(TCPIP_SendStatus == TCPSEND_IDLE)
        {
            if(HeartbeatTimer>0)//������ʱ
            {
                HeartbeatTimer--;
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
                TCPIP_SendStatus = TCPSEND_IDLE;
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
        }
    }
    
}

void TCPIP_Recv(u8 *TCPBuf,u16 usRecBufLen)//��client��serverʱ�յ��Է��������ı���,����ͬһ��ʽ����
{
    u16 cnt;
    u8 chn;
    unsigned short int usNumber;
   
    if(usRecBufLen > 0) //����������ݳ���Ϊ0.�򷵻�
    {      
        cnt = ((TCPBuf[0])<<8)+TCPBuf[1];
        if(cnt==TCPIP_Count)
        {
            //��˵�±ߵĳ�����Ӧ�÷��ڴ˴��ˣ���ֹ�ֳ�Э��ƫ����ɲ��������������Ƚ��������ɴ���
        }
        else
        {
             LED_BLINK_ONCE(S_FAULT);
            //errorcode
        }
        TCPIP_Count = (++cnt);
        
        if(ON == TCPBuf[2]) //�����ʶ�������ֽ�Ϊ1ʱ��ʾ������ת������
        {    
            usNumber = (u16)(TCPBuf[4]<<8);
            usNumber +=(u16)(TCPBuf[5]);
            
            chn = TCPBuf[3];
            if(chn<NUM_UARTCAN)
            {
                ETH2Usartcan_send(chn,&TCPBuf[6],usNumber);//����tcpip�����������������ڻ�CAN
                TCPBuf[2] = 0x00;//�������ʶ���
            }
            else
            {
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
            OSMboxPost(EV_TCPIPsendbuf,(void *)&TCPBuf[0]);//��������
        }
        else
        {
            LED_BLINK_ONCE(S_FAULT);
            //errorcode
        }
    }
}

void Task_TCPIP(void *p_arg)
{
    
    struct wiz_NetInfo_t *tcpipparm;
    unsigned int len = 0;
    int32_t ret = 0;
    int8_t sock_status,tmp;
	tcpipparm = (struct wiz_NetInfo_t *)p_arg;
    TCPIP_Init();
    if(tcpipparm->session_mode == S_tcpip_server)
    {
        //�½�һ��Socket���󶨱��ض˿�
        ret = socket(tcpipparm->tcpsocket, Sn_MR_TCP, tcpipparm->portlocal, Sn_MR_ND);
        if(ret != tcpipparm->tcpsocket) {
            ///printf("%d:Socket Error\r\n",SOCK_TCPS);
            //		while(1);
        } else {
            ///printf("%d:Opened\r\n",SOCK_TCPS);
        }
    }
    else
    {
			  //�½�һ��Socket���󶨱��ض˿�5000	
        //Ethernet_OpenClient(SOCK_TCPIP,(uint16_t)TCPIP_Network.local_port,TCPIP_Network.remote_ip,(uint16_t)TCPIP_Network.remote_port);
    }
    
    
    OSTaskCreate(TCPIP_Send_task, (void *)p_arg, (OS_STK*)&STK_TCPIP_SEND[STKSIZE_TCPIP_SEND-1], TASK_PRIO_TCPIP_SEND);
    
    while (1)
    {
        ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);
        if (tmp == PHY_LINK_OFF)
        {/*������ߴ���*/
            tcpipparm->conn_status = OFF;
            close(tcpipparm->tcpsocket);
            //errorcode
        }
        
        switch(getSn_SR(tcpipparm->tcpsocket))	//��ȡsocket0��״̬
        {
            //socket��ʼ�����
            case SOCK_INIT:
                tcpipparm->conn_status = OFF;
                //errorcode
	            if(tcpipparm->session_mode == S_tcpip_server) 
                {
	                listen(tcpipparm->tcpsocket);	//����TCP�ͻ��˳�������
	            }
                else
                {
	                ret = connect(tcpipparm->tcpsocket, tcpipparm->ipgoal, tcpipparm->portgoal);
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
                tcpipparm->conn_status = ON;
                if(getSn_IR(tcpipparm->tcpsocket) & Sn_IR_CON) 
                {
                  setSn_IR(tcpipparm->tcpsocket, Sn_IR_CON);
                }
                              
                if(getSn_RX_RSR(tcpipparm->tcpsocket) > 0)	
                {
                    len = recv(tcpipparm->tcpsocket, aucTCPBuf, sizeof(aucTCPBuf));
                    TCPIP_Recv(aucTCPBuf,len);
                }
                
                if(tcpipparm->session_mode == S_tcpip_client) 
                {
                    if((TCPIP_SendStatus == TCPSEND_IDLE)&&(HeartbeatTimer==0))
                    {//������������
                        //TCP ֡������
                        memset(aucTCPBuf,0,sizeof(aucTCPBuf));
                        TCPIP_Count++;
                        aucTCPBuf[0] = (TCPIP_Count >> 8) & 0xFF;
                        aucTCPBuf[1] = (TCPIP_Count) & 0xFF;
                        //�̶�Ϊ0
                        aucTCPBuf[2] = 00;
                        aucTCPBuf[3] = 0xAA;
                        //MBTCP ֡���ٴ����ĳ��ȣ���ʵ����*2+7
                        aucTCPBuf[4] = 0x00;
                        aucTCPBuf[5] = 0x00;//�̶�ֵ
                        
                        TCPIP_SendStatus = TCPSEND_WAIT_ACK;//��������
                        TO_Timer = TCPSEND_TIMEOUT;
                        OSMboxPost(EV_TCPIPsendbuf,(void *)&aucTCPBuf[0]);//��������
                        TCPIP_SendStatus = TCPSEND_IDLE;
                        HeartbeatTimer = TCP_HEARTBEAT;//��������ȴ��ظ���ֱ����Ϊ��������
                    }
                }
            	break;
        //socket�ȴ��ر�״̬
            case SOCK_CLOSE_WAIT:
                //errorcode
                tcpipparm->conn_status = OFF;
                close(tcpipparm->tcpsocket);
            	break;
        //socket�ر�
            case SOCK_CLOSED:
                //errorcode
                tcpipparm->conn_status = OFF;
                sock_status=0xff;
                while(sock_status!=tcpipparm->tcpsocket)
                {
                    sock_status = socket(tcpipparm->tcpsocket, Sn_MR_TCP, tcpipparm->portlocal, Sn_MR_ND);
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
                tcpipparm->conn_status = OFF;
                //errorcode
            //connect(tcpipparm->tcpsocket, tcpipparm->ipgoal, tcpipparm->portgoal);
                break;
        }
        OSTimeDlyHMSM(0, 0, 0, 10);
    }
}


void TCPIP_DataHandler(u8 chnidx,struct USARTCAN_Recv_t *recv,void *arg)
{
    struct wiz_NetInfo_t *tcpipparm;
    tcpipparm = (struct wiz_NetInfo_t *)arg;
    u8 dataidx;
    dataidx=0;
    memset(&TCPIP_TR_data[dataidx],0,recv->lenth);
    TCPIP_TR_data[dataidx++] = ON;    //��ͨ�������ݸ���ʱ��һ���̶ֹ�Ϊ0x0001��
    TCPIP_TR_data[dataidx++] = chnidx;    //
    TCPIP_TR_data[dataidx++] = (recv->lenth)>>8;
    TCPIP_TR_data[dataidx++] = (recv->lenth)&0xff;    //
    memcpy(&TCPIP_TR_data[dataidx],recv->databuf,recv->lenth);
    
    if((tcpipparm->session_mode == S_tcpip_client)||(tcpipparm->session_mode == S_tcpip_server))//����ģʽʹ����ͬ���ƣ�������˫����
    {
        //MBTCP ֡������
        memset(aucTCPBuf,0,sizeof(aucTCPBuf));
        TCPIP_Count++;
        aucTCPBuf[0] = (TCPIP_Count >> 8) & 0xFF;
        aucTCPBuf[1] = (TCPIP_Count) & 0xFF;        
        memcpy(&aucTCPBuf[2],&TCPIP_TR_data[0],recv->lenth+4);
        TCPIP_SendStatus = TCPSEND_WAIT_ACK;
        TO_Timer = TCPSEND_TIMEOUT;
        OSMboxPost(EV_TCPIPsendbuf,(void *)&aucTCPBuf[0]);//��������
        TCPIP_SendStatus = TCPSEND_IDLE;
        HeartbeatTimer = TCP_HEARTBEAT;//ĿǰЭ����δ��������ֱ����Ϊ��������
    }		
}

void TCPIP_Send_task(void *arg)
{
	u8 *sedmsg;
	u8 err;
    struct wiz_NetInfo_t *tcpipparm;

    tcpipparm = (struct wiz_NetInfo_t *)arg;
	while(1)
	{
		sedmsg=(u8*)OSMboxPend(EV_TCPIPsendbuf,0,&err);
		if(err == OS_ERR_NONE)
		{
            while(!tcpipparm->conn_status)
            {
                if(TCPIP_SendStatus == TCPSEND_IDLE)
                {
                    LED_BLINK_ONCE(S_FAULT);
                    //errorcode
                    break;
                }
                OSTimeDlyHMSM(0, 0, 0, 2);
            }
            if(tcpipparm->conn_status)
            {
                send(tcpipparm->tcpsocket, sedmsg, (sedmsg[4]<<8)+sedmsg[5]+6);
            }
		}
        OSTimeDlyHMSM(0, 0, 0, 1);
	}
}















