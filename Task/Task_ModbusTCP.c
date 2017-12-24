#include "Task_ModbusTCP.h"
#include "socket.h"	// Just include one header for WIZCHIP
#include "dhcp.h"
#include "main.h"
#include "wizchip_conf.h"
#include "Task_LED.h"
/* -------------------------------- web --------------------------------------*/
#include "httputil.h"
#define MBDATA_BUF_SIZE                  280//mb操作的读最大为127个字，单帧最大字节数不可能超过该值，不建议再增大

#define TCPSEND_IDLE                         0u
#define TCPSEND_WAIT_ACK                     1u

#define TCPSEND_TIMEOUT                    5000//超时时间为5s，一旦满足则报警一次

#define STKSIZE_MBTCP_SEND            STK_SIZE_384

OS_STK STK_MBTCP_SEND[STKSIZE_MBTCP_SEND];

static uint8_t aucTCPBuf[MBDATA_BUF_SIZE];	 //接收数据缓存
static uint8_t TCP_TR_data[MBDATA_BUF_SIZE]; //发送缓冲区
static u16 MBTCP_Count; //帧计数器，正常情况下没发一帧计数加1
static u8 usartcan_chn;//通道号
static u16 Polltimer;//poll定时器，做client时需按照配置时间周期读server的寄存器
static u16 TO_Timer;//超时计数器

volatile Tdef_Byte _ModbusTCP_Status;
#define ModbusTCP_Status                 _ModbusTCP_Status.Byte
#define MBTCP_SendStatus                 _ModbusTCP_Status.Bits.bit0  //发送状态，0-空闲  1-发送完数据，等待对方响应

OS_EVENT *EV_MBTCPsendbuf;           //以太网发送

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
            if(Polltimer>0)//该变量在应用中赋值，一般为从flash内读取的配置
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
   
    if(usRecBufLen > 0) //如果接受数据长度为0.则返回
    {
        usStartAddr = (u16)(aucTCPBuf[8]<<8);
        usStartAddr +=(u16)(aucTCPBuf[9]);
        
        cnt = ((aucTCPBuf[0])<<8)+aucTCPBuf[1];//获取收到的帧计数器
        if(cnt==MBTCP_Count)
        {
            //按说下边的程序是应该放在此处了，防止现场协议偏差造成不能正常工作，先将条件放松处理
        }
        else
        {
             LED_BLINK_ONCE(S_FAULT);
            //errorcode
        }
        MBTCP_Count = (++cnt);

        if(0x03 == aucTCPBuf[7]) //读寄存器功能
        {    
            usNumber = (u16)(aucTCPBuf[10]<<8);
            usNumber +=(u16)(aucTCPBuf[11]);//获取读请求的数量
            usTxLen=usNumber<<1;
            if(usStartAddr<USARTCAN.addr)//读取地址是否在正常范围内
            {
                aucTCPBuf[4] = 0;
                aucTCPBuf[5] = 3;
                aucTCPBuf[7] = aucTCPBuf[7] | 0x80;
                aucTCPBuf[8] = 0x02;
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
            else if((usStartAddr+usNumber)>(USARTCAN.addr+(USARTCAN.datalen*NUM_UARTCAN)))//请求一部分超出范围
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
                memcpy(&aucTCPBuf[9],&TCP_TR_data[(usStartAddr-USARTCAN.addr)*2],usNumber*2);//请求合理
			
                aucTCPBuf[8]=(u8)usTxLen;
                usTxLen += 3;	 
                aucTCPBuf[4]=(u8)(usTxLen>>8);	 
                aucTCPBuf[5]=(u8)usTxLen;
            }
        }
        else if(0x06 == aucTCPBuf[7])//写单个寄存器，主要用于串口或CAN转以太网时有数据更新后，client处理完写该标志位为0
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
        else if(0x10 == aucTCPBuf[7])//写多个寄存器，主要用于client发数据转换成串口或CAN
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
                if((aucTCPBuf[13]==0x00)&&(aucTCPBuf[14]==0x01))//以太网向串口转数据
                {
                    if((usStartAddr-USARTCAN.addr)%USARTCAN.datalen==0)//只允许从起始地址或端口对应的偏移地址开始，否则报错
                    {
                        chn=(usStartAddr-USARTCAN.addr)/USARTCAN.datalen;
                        if(chn<NUM_UARTCAN)//端口是否有效
                        {
                            ETH2Usartcan_send(chn,&aucTCPBuf[17],aucTCPBuf[16]);//对mb来说长度只有一个字节
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
        OSMboxPost(EV_MBTCPsendbuf,(void *)&aucTCPBuf[0]);//启动发送
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
        //新建一个Socket并绑定本地端口5000
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
			  //新建一个Socket并绑定本地端口5000	
        //Ethernet_OpenClient(SOCK_MODBUSTCP,(uint16_t)MBTCP_Network.local_port,MBTCP_Network.remote_ip,(uint16_t)MBTCP_Network.remote_port);
    }
    
    OSTaskCreate(MBTCP_Send_task, (void *)p_arg, (OS_STK*)&STK_MBTCP_SEND[STKSIZE_MBTCP_SEND-1], TASK_PRIO_MBTCP_SEND);
    
    while (1)
    {
        ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);
        if (tmp == PHY_LINK_OFF)
        {/*物理断线处理*/
            mbtcpparm->conn_status = OFF;
            close(mbtcpparm->tcpsocket);
            //errorcode
        }
        
        switch(getSn_SR(mbtcpparm->tcpsocket))	//获取socket0的状态
        {
            //socket初始化完成
            case SOCK_INIT:
                mbtcpparm->conn_status = OFF;
                //errorcode
	            if(mbtcpparm->session_mode == S_mb_server) 
                {
	                listen(mbtcpparm->tcpsocket);	//监听TCP客户端程序连接
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
            //socket连接建立
            case SOCK_ESTABLISHED:
                mbtcpparm->conn_status = ON;
                if(getSn_IR(mbtcpparm->tcpsocket) & Sn_IR_CON) 
                {
                  setSn_IR(mbtcpparm->tcpsocket, Sn_IR_CON);
                }
                              
                if(getSn_RX_RSR(mbtcpparm->tcpsocket) > 0)	//下边的接收函数为阻塞型，先判断长度是否为0，大于0才能进接收函数内
                {
                    len = recv(mbtcpparm->tcpsocket, aucTCPBuf, sizeof(aucTCPBuf));
                    if(mbtcpparm->session_mode == S_mb_server) 
                    {
                        if((u8)mbtcpparm->stationID == aucTCPBuf[6]) //MB 站地址只有一个字节，且不满足不做任何回复
                        {
                            Modbus_Server(aucTCPBuf,len);//做server时，被动接收请求
                        }
                        else
                        {
                            LED_BLINK_ONCE(S_FAULT);
                            //errorcode
                        }
                    }
                    else
                    {
                        if(MBTCP_SendStatus == TCPSEND_WAIT_ACK)//若当前为发送完数据后收到响应
                        {
                            MBTCP_SendStatus = TCPSEND_IDLE;
                            Polltimer = mbtcpparm->polltime;
                            if(((MBTCP_Count>>8)==aucTCPBuf[0])&&((u8)MBTCP_Count==aucTCPBuf[1]))
                            {
                                //按说下边的程序是应该放在此处了，防止现场协议偏差造成不能正常工作，先将条件放松处理
                            }
                            else
                            {
                                 LED_BLINK_ONCE(S_FAULT);
                                //errorcode
                            }
                            if (0x80 != (aucTCPBuf[7]&0x80))
                            {//肯定响应
                                if((aucTCPBuf[9]==0x00)&&(aucTCPBuf[10]==0x01))//以太网向串口转数据
                                {//若收到server端有数据更新的标识，启动写请求将其清零
                                    ETH2Usartcan_send(usartcan_chn,&aucTCPBuf[13],aucTCPBuf[12]);//对mb来说长度只有一个字节
                                    //MBTCP 帧号序列
                                    memset(aucTCPBuf,0,sizeof(aucTCPBuf));
                                    MBTCP_Count++;
                                    aucTCPBuf[0] = (MBTCP_Count >> 8) & 0xFF;
                                    aucTCPBuf[1] = (MBTCP_Count) & 0xFF;
                                    //MBTCP 固定为0
                                    aucTCPBuf[2] = 0;
                                    aucTCPBuf[3] = 0;
                                    //MBTCP 帧中再次向后的长度
                                    aucTCPBuf[4] = 0x00;
                                    aucTCPBuf[5] = 6;//固定值
                                    //MBTCP 栈号ID
                                    aucTCPBuf[6] = mbtcpparm->stationID;
                                    //MBTCP 功能码：06，用于写多个寄存器命令。
                                    aucTCPBuf[7] = 0x06;
                                    //MBTCP 帧中写的起始地址，
                                    aucTCPBuf[8] = ((USARTCAN.addr+(USARTCAN.datalen*usartcan_chn)) >> 8) & 0XFF;
                                    aucTCPBuf[9] =  (USARTCAN.addr+(USARTCAN.datalen*usartcan_chn)) & 0XFF;
                                    //MBTCP 帧中写的寄存器数量
                                    aucTCPBuf[10] = 0x00;
                                    aucTCPBuf[11] =	0x00;
                                    MBTCP_SendStatus = TCPSEND_WAIT_ACK;
                                    TO_Timer = TCPSEND_TIMEOUT;
                                    OSMboxPost(EV_MBTCPsendbuf,(void *)&aucTCPBuf[0]);//启动发送
                                }
                            }
                            else
                            {//否定响应
                                LED_BLINK_ONCE(S_FAULT);
                                //errorcode
                            }
                        }
                        else
                        {//异常响应
                            LED_BLINK_ONCE(S_FAULT);
                                //errorcode
                        }
                    }
                }
                
                if(mbtcpparm->session_mode == S_mb_client) 
                {
                    if((MBTCP_SendStatus == TCPSEND_IDLE)&&(Polltimer==0))
                    {//做client时需要向server端周期的进行poll
                        //MBTCP 帧号序列
                        memset(aucTCPBuf,0,sizeof(aucTCPBuf));
                        MBTCP_Count++;
                        aucTCPBuf[0] = (MBTCP_Count >> 8) & 0xFF;
                        aucTCPBuf[1] = (MBTCP_Count) & 0xFF;
                        //MBTCP 固定为0
                        aucTCPBuf[2] = 0;
                        aucTCPBuf[3] = 0;
                        //MBTCP 帧中再次向后的长度：真实数据*2+7
                        aucTCPBuf[4] = 0x00;
                        aucTCPBuf[5] = 6;//固定值
                        //MBTCP 栈号ID，其实没用，就添的01
                        aucTCPBuf[6] = mbtcpparm->stationID;
                        //MBTCP 功能码：16，用于写多个寄存器命令。
                        aucTCPBuf[7] = 0x03;
                        //MBTCP 帧中写的起始地址，
                        usartcan_chn++;
                        if(usartcan_chn>=NUM_UARTCAN)
                        {
                            usartcan_chn = 0;
                        }
                        aucTCPBuf[8] = ((USARTCAN.addr+(USARTCAN.datalen*usartcan_chn)) >> 8) & 0XFF;
                        aucTCPBuf[9] =  (USARTCAN.addr+(USARTCAN.datalen*usartcan_chn)) & 0XFF;
                        //MBTCP 帧中读的寄存器数量
                        aucTCPBuf[10] = (USARTCAN.datalen >> 8) & 0XFF;
                        aucTCPBuf[11] =	USARTCAN.datalen & 0XFF;
                        MBTCP_SendStatus = TCPSEND_WAIT_ACK;
                        TO_Timer = TCPSEND_TIMEOUT;
                        OSMboxPost(EV_MBTCPsendbuf,(void *)&aucTCPBuf[0]);//启动发送
                    }
                }
                
            	break;
        //socket等待关闭状态
            case SOCK_CLOSE_WAIT:
                //errorcode
                mbtcpparm->conn_status = OFF;
                close(mbtcpparm->tcpsocket);
            	break;
        //socket关闭
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
    TCP_TR_data[data_off++] = ON;    //该通道有数据更新时第一个字固定为0x0001，若为server需要client写0清除
    TCP_TR_data[data_off++] = (recv->lenth)>>8;
    TCP_TR_data[data_off++] = (recv->lenth)&0xff;    //处理串口或CAN收到的数据长度
    memcpy(&TCP_TR_data[data_off],recv->databuf,recv->lenth);
    
    if(mbtcpparm->session_mode == S_mb_client)
    {//将串口或CAN收到的数据通过写操作到server端，只会写一次
        //MBTCP 帧号序列
        memset(aucTCPBuf,0,sizeof(aucTCPBuf));
        MBTCP_Count++;
        aucTCPBuf[0] = (MBTCP_Count >> 8) & 0xFF;
        aucTCPBuf[1] = (MBTCP_Count) & 0xFF;
        //MBTCP 固定为0
        aucTCPBuf[2] = 0;
        aucTCPBuf[3] = 0;
        //MBTCP 帧中再次向后的长度：真实数据*2+7
        aucTCPBuf[4] = ((USARTCAN.datalen * 2 + 7) >> 8) & 0XFF;
        aucTCPBuf[5] = (USARTCAN.datalen * 2 + 7) & 0XFF;
        //MBTCP 栈号ID
        aucTCPBuf[6] = mbtcpparm->stationID;
        //MBTCP 功能码：16，用于写多个寄存器命令。
        aucTCPBuf[7] = 0x10;
        //MBTCP 帧中写的起始地址，
        aucTCPBuf[8] = ((USARTCAN.addr+(USARTCAN.datalen*chnidx)) >> 8) & 0XFF;
        aucTCPBuf[9] =  (USARTCAN.addr+(USARTCAN.datalen*chnidx)) & 0XFF;
        //MBTCP 帧中写的寄存器数量
        aucTCPBuf[10] = (USARTCAN.datalen >> 8) & 0XFF;
        aucTCPBuf[11] =	USARTCAN.datalen & 0XFF;
        //MBTCP 帧中写的字节数量
        aucTCPBuf[12] = (USARTCAN.datalen * 2) & 0XFF;
        TCP_TR_data[(USARTCAN.datalen * 2 * chnidx)+1]=0;//为了实现双向通讯且共地址，此处做了特殊处理，将标志位强行清除，因为只会写一次，不影响功能的识别
        memcpy(&aucTCPBuf[13],&TCP_TR_data[USARTCAN.datalen * 2 * chnidx],recv->lenth+4);
        MBTCP_SendStatus = TCPSEND_WAIT_ACK;
        TO_Timer = TCPSEND_TIMEOUT;
        OSMboxPost(EV_MBTCPsendbuf,(void *)&aucTCPBuf[0]);//启动发送
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
		if(err == OS_ERR_NONE)//消息邮箱发送
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















