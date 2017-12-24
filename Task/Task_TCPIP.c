
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

#define TCPSEND_TIMEOUT                    5000//超时时间为5s
#define TCP_HEARTBEAT                      6000//心跳周期

#define STKSIZE_TCPIP_SEND            STK_SIZE_384
OS_STK STK_TCPIP_SEND[STKSIZE_TCPIP_SEND];

static uint8_t aucTCPBuf[TCPIPDATA_BUF_SIZE];	 //接收数据缓存
static uint8_t TCPIP_TR_data[TCPIPDATA_BUF_SIZE];//发送数据缓存
static u16 TCPIP_Count;//帧计数器
static u16 HeartbeatTimer;//心跳计时
static u16 TO_Timer;//超时计时

volatile Tdef_Byte _TCPIP_Status;
#define TCPIP_Status                     _TCPIP_Status.Byte
#define TCPIP_SendStatus                 _TCPIP_Status.Bits.bit0  //发送状态

OS_EVENT *EV_TCPIPsendbuf;           //tcpip报文发送

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
            if(HeartbeatTimer>0)//心跳计时
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

void TCPIP_Recv(u8 *TCPBuf,u16 usRecBufLen)//做client或server时收到对方发过来的报文,均用同一方式处理
{
    u16 cnt;
    u8 chn;
    unsigned short int usNumber;
   
    if(usRecBufLen > 0) //如果接受数据长度为0.则返回
    {      
        cnt = ((TCPBuf[0])<<8)+TCPBuf[1];
        if(cnt==TCPIP_Count)
        {
            //按说下边的程序是应该放在此处了，防止现场协议偏差造成不能正常工作，先将条件放松处理
        }
        else
        {
             LED_BLINK_ONCE(S_FAULT);
            //errorcode
        }
        TCPIP_Count = (++cnt);
        
        if(ON == TCPBuf[2]) //请求标识，当此字节为1时表示有数据转换请求
        {    
            usNumber = (u16)(TCPBuf[4]<<8);
            usNumber +=(u16)(TCPBuf[5]);
            
            chn = TCPBuf[3];
            if(chn<NUM_UARTCAN)
            {
                ETH2Usartcan_send(chn,&TCPBuf[6],usNumber);//按照tcpip端请求发送数据至串口或CAN
                TCPBuf[2] = 0x00;//将请求标识清除
            }
            else
            {
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
            OSMboxPost(EV_TCPIPsendbuf,(void *)&TCPBuf[0]);//启动发送
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
        //新建一个Socket并绑定本地端口
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
			  //新建一个Socket并绑定本地端口5000	
        //Ethernet_OpenClient(SOCK_TCPIP,(uint16_t)TCPIP_Network.local_port,TCPIP_Network.remote_ip,(uint16_t)TCPIP_Network.remote_port);
    }
    
    
    OSTaskCreate(TCPIP_Send_task, (void *)p_arg, (OS_STK*)&STK_TCPIP_SEND[STKSIZE_TCPIP_SEND-1], TASK_PRIO_TCPIP_SEND);
    
    while (1)
    {
        ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);
        if (tmp == PHY_LINK_OFF)
        {/*物理断线处理*/
            tcpipparm->conn_status = OFF;
            close(tcpipparm->tcpsocket);
            //errorcode
        }
        
        switch(getSn_SR(tcpipparm->tcpsocket))	//获取socket0的状态
        {
            //socket初始化完成
            case SOCK_INIT:
                tcpipparm->conn_status = OFF;
                //errorcode
	            if(tcpipparm->session_mode == S_tcpip_server) 
                {
	                listen(tcpipparm->tcpsocket);	//监听TCP客户端程序连接
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
            //socket连接建立
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
                    {//发送心跳报文
                        //TCP 帧号序列
                        memset(aucTCPBuf,0,sizeof(aucTCPBuf));
                        TCPIP_Count++;
                        aucTCPBuf[0] = (TCPIP_Count >> 8) & 0xFF;
                        aucTCPBuf[1] = (TCPIP_Count) & 0xFF;
                        //固定为0
                        aucTCPBuf[2] = 00;
                        aucTCPBuf[3] = 0xAA;
                        //MBTCP 帧中再次向后的长度：真实数据*2+7
                        aucTCPBuf[4] = 0x00;
                        aucTCPBuf[5] = 0x00;//固定值
                        
                        TCPIP_SendStatus = TCPSEND_WAIT_ACK;//心跳报文
                        TO_Timer = TCPSEND_TIMEOUT;
                        OSMboxPost(EV_TCPIPsendbuf,(void *)&aucTCPBuf[0]);//启动发送
                        TCPIP_SendStatus = TCPSEND_IDLE;
                        HeartbeatTimer = TCP_HEARTBEAT;//心跳无需等待回复，直接认为发送正常
                    }
                }
            	break;
        //socket等待关闭状态
            case SOCK_CLOSE_WAIT:
                //errorcode
                tcpipparm->conn_status = OFF;
                close(tcpipparm->tcpsocket);
            	break;
        //socket关闭
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
    TCPIP_TR_data[dataidx++] = ON;    //该通道有数据更新时第一个字固定为0x0001，
    TCPIP_TR_data[dataidx++] = chnidx;    //
    TCPIP_TR_data[dataidx++] = (recv->lenth)>>8;
    TCPIP_TR_data[dataidx++] = (recv->lenth)&0xff;    //
    memcpy(&TCPIP_TR_data[dataidx],recv->databuf,recv->lenth);
    
    if((tcpipparm->session_mode == S_tcpip_client)||(tcpipparm->session_mode == S_tcpip_server))//两种模式使用相同机制，均满足双向传输
    {
        //MBTCP 帧号序列
        memset(aucTCPBuf,0,sizeof(aucTCPBuf));
        TCPIP_Count++;
        aucTCPBuf[0] = (TCPIP_Count >> 8) & 0xFF;
        aucTCPBuf[1] = (TCPIP_Count) & 0xFF;        
        memcpy(&aucTCPBuf[2],&TCPIP_TR_data[0],recv->lenth+4);
        TCPIP_SendStatus = TCPSEND_WAIT_ACK;
        TO_Timer = TCPSEND_TIMEOUT;
        OSMboxPost(EV_TCPIPsendbuf,(void *)&aucTCPBuf[0]);//启动发送
        TCPIP_SendStatus = TCPSEND_IDLE;
        HeartbeatTimer = TCP_HEARTBEAT;//目前协议里未做交互，直接认为发送正常
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















