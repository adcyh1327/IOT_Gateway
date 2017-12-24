
#include "main.h"
#include "ucos_ii.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "Task_MQTT.h"
#include "Task_Main.h"
#include "socket.h"
#include "Task_LED.h"


#define STKSIZE_MQTTSUB                  STK_SIZE_512
#define STKSIZE_MQTTPUB                  STK_SIZE_1024


#define FEED_PINREQ(n)         {PinReqTime=n*700;}    //n*7000/10

OS_EVENT *EV_MQTTRECV;           //

OS_STK STK_MQTTSUB[STKSIZE_MQTTSUB];
OS_STK STK_MQTTPUB[STKSIZE_MQTTPUB];

static unsigned char Recvbuf[200];//���ջ�����
static char clientid[40]={"ProtocolTranslator/ClientID[IDnm]"};//ͬһ������ÿ���ڵ�Ĵ�ID������ͬ
static char pubtopic[TOPIC_NUMMAX][TOPICNAME_MAXLEN]=
{
    {"ProtocolTranslator/[IDnm]/RS232_1/Publish"},
    {"ProtocolTranslator/[IDnm]/RS232_2/Publish"},
    {"ProtocolTranslator/[IDnm]/RS485/Publish"},
    {"ProtocolTranslator/[IDnm]/Can/Publish"},
    {"ProtocolTranslator/[IDnm]/Heartbeat"}
};
static char subtopic[TOPIC_NUMMAX][TOPICNAME_MAXLEN]=
{
    {"ProtocolTranslator/[IDnm]/RS232_1/Subscribe"},
    {"ProtocolTranslator/[IDnm]/RS232_2/Subscribe"},
    {"ProtocolTranslator/[IDnm]/RS485/Subscribe"},
    {"ProtocolTranslator/[IDnm]/Can/Subscribe"},
    {"ProtocolTranslator/[IDnm]/Heartbeat"}
};
struct MQTT_Topic_Info_t MQTT_PubInfo[TOPIC_NUMMAX]=
{
    {
        .topic_name = MQTTString_initializer,
        .qos = 1,
        .packid = 0x01,
        .msgdata = NULL,
        .msglen = 10,
        .msgsta = ON
    },
    {
        .topic_name = MQTTString_initializer,
        .qos = 1,
        .packid = 0x02,
        .msgdata = NULL,
        .msglen = 10,
        .msgsta = ON
    },
    {
        .topic_name = MQTTString_initializer,
        .qos = 1,
        .packid = 0x03,
        .msgdata = NULL,
        .msglen = 10,
        .msgsta = ON
    },
    {
        .topic_name = MQTTString_initializer,
        .qos = 1,
        .packid = 0x04,
        .msgdata = NULL,
        .msglen = 10,
        .msgsta = ON
    },
    {
        .topic_name = MQTTString_initializer,
        .qos = 1,
        .packid = 0x05,
        .msgdata = NULL,
        .msglen = 10,
        .msgsta = ON
    }
};

#ifdef CJSON_ENABLE
cJSON *mqttMsg[TOPIC_NUMMAX] ;
#else
u8 *mqttMsg[TOPIC_NUMMAX] ;//��Ϣ����
#endif
struct MQTT_Topic_Info_t MQTT_SubInfo[TOPIC_NUMMAX];//����������Ϣ
struct mqtt_recv_t mqtt_recv_msg;//����������Ϣ

struct MQTT_CFG_Connect_t MQTT_CFG_Connect=
{
    .UserName = {""},
    .PassWard = {""},
    .Client_id = MQTTString_initializer,
    .con_qos = 0,
    .kpalivespace = 8//KEEPALIVE jjj
};
struct PublishReq_Block_t *PublishReq_Block[N_PUBQUEUE];//��������
OS_EVENT *PublishQueue;//��Ϣ����

static u16 HeartbeatTime;//������ʱ
static u16 HeartbeatSpace;//��������
static u16  PinReqTime;//mqtt�ڷ�����Ъ��Ҫ���ڷ���ping�����Ա���alive�������ڿ��Ź�ι�����ܣ��˱��������ڶ�ʱ

volatile Tdef_Byte _MQTT_Status;
#define MQTT_Status                 _MQTT_Status.Byte
#define MQTT_Established            _MQTT_Status.Bits.bit0
#define MQTT_SubStatus              _MQTT_Status.Bits.bit1

int MQTT_MsgHandler(struct mqtt_recv_t* recvmsg);
void MQTTSub_task(void *p_arg);
void MQTTPub_task(void *p_arg);
void MQTTRecv_task(void *p_arg);

//��ʼ������
void MQTT_Init(u16 sid)
{
    u8 i,j;
    MQTT_Status=0;
    FEED_PINREQ(MQTT_CFG_Connect.kpalivespace);
    memset(Recvbuf,0,sizeof(Recvbuf));
    HeartbeatSpace=6000;
    MQTT_CFG_Connect.Client_id.cstring = clientid;
    for(j=0;j<4;j++)
    {
        MQTT_CFG_Connect.Client_id.cstring[28+j] = HexToAsc((sid>>(4*(4-1-j)))&0x0f);
    }
    
    for(i=0;i<TOPIC_NUMMAX;i++)
    {
        MQTT_PubInfo[i].topic_name.cstring = pubtopic[i];
        MQTT_PubInfo[i].qos = 0x01;//�ٴ�ָ����ʵ�ʷ���ʱ�������޸�
        MQTT_PubInfo[i].packid = i; //ÿ�������������IDΪ��0��ʼ�����ۼӵ�ֵ����Ϊ��Ϣ��������SN����ֵ�����ο�
        MQTT_PubInfo[i].msgdata = NULL;
        MQTT_PubInfo[i].msglen = 1,
        MQTT_PubInfo[i].msgsta = ON;
        MQTT_PubInfo[i].msgcnt = 0x0001;

        MQTT_SubInfo[i].topic_name.cstring = subtopic[i];
        MQTT_SubInfo[i].qos = 0x01;
        MQTT_SubInfo[i].packid = i; //ÿ�������������IDΪ��0��ʼ�����ۼӵ�ֵ����Ϊ��Ϣ��������SN����ֵ�����ο�
        MQTT_SubInfo[i].msgdata = NULL;
        MQTT_SubInfo[i].msglen = 1;
        MQTT_SubInfo[i].msgsta = OFF;
        MQTT_SubInfo[i].msgcnt = 0x0001;
        
        for(j=0;j<4;j++)//�����õ�ID���뵽�����ڣ�ʹ��ͬһ�����д�IDΨһ
        {
            MQTT_PubInfo[i].topic_name.cstring[20+j] = HexToAsc((sid>>(4*(4-1-j)))&0x0f);
            MQTT_SubInfo[i].topic_name.cstring[20+j] = HexToAsc((sid>>(4*(4-1-j)))&0x0f);
        }

    }
    EV_MQTTRECV = OSMboxCreate((void*)0);
    PublishQueue = OSQCreate((void *)&PublishReq_Block[0],N_PUBQUEUE);
    
}

//1ms�Ķ�ʱ����������1ms�Ķ�ʱ�ж���
void MQTT_Timer1ms(void)
{
    #ifdef CJSON_ENABLE
    char *msg;
    #endif
    if((gWIZNETINFO.conn_status)&&(gWIZNETINFO.session_mode==S_mqtt))
    {
        if(PinReqTime>0)
        {
            PinReqTime--;
        }
        if(HeartbeatTime>0)
        {
            HeartbeatTime--;
        }
        else
        {//�������ⷢ��
            #ifdef CJSON_ENABLE
            mqttMsg[t_heartbeat] = cJSON_CreateObject();
            cJSON_AddNumberToObject(mqttMsg[t_heartbeat],"SN",MQTT_PubInfo[t_heartbeat].msgcnt++);
            cJSON_AddStringToObject(mqttMsg[t_heartbeat],"DType",DataType[t_ASCII]);
            cJSON_AddStringToObject(mqttMsg[t_heartbeat],"Data","Heartbeat");
            msg = cJSON_Print(mqttMsg[t_heartbeat]);
            if(msg == NULL)
            {
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
            else
            {
                MQTT_PubInfo[t_heartbeat].msgdata = (u8 *)msg;
                MQTT_PubInfo[t_heartbeat].msglen = strlen((const char *)MQTT_PubInfo[t_heartbeat].msgdata);
                //MQTT_PubInfo[t_heartbeat].packid=0x1122;
                mqtt_publish(gWIZNETINFO.tcpsocket,&MQTT_PubInfo[t_heartbeat]); //���������͵���Ϣ 
                FEED_PINREQ(MQTT_CFG_Connect.kpalivespace);
            }
            free(msg);
            cJSON_Delete(mqttMsg[t_heartbeat]);
            #else
            mqttMsg[t_heartbeat] = "Hello MQTT! Current status is normal.\n";
            MQTT_PubInfo[t_heartbeat].msgdata = (u8 *) mqttMsg[t_heartbeat];
            MQTT_PubInfo[t_heartbeat].msglen = 38;
            //MQTT_PubInfo[t_heartbeat].packid=0x1122;
            MQTT_PublishMsg(&MQTT_PubInfo[t_heartbeat]);
            #endif
            
            HeartbeatTime = HeartbeatSpace;
        }
    }
}

/*
 * ��������MQTTPub_task
 * ����  ��������ΪMQTTЭ��ķ�����
 * ����  ����
 * ���  ����
 * ˵��  ����
 */
void MQTT_task(void *p_arg)
{		
    int ret;
    struct wiz_NetInfo_t *mqttparm;
    int8_t sock_status;
    u8 tmp,i;
    
    mqttparm = (struct wiz_NetInfo_t *)p_arg;
    MQTT_Init(mqttparm->stationID);
    
    /*MQTT���ݷ�������*/
    OSTaskCreate(MQTTPub_task, (void *)p_arg, &STK_MQTTPUB[STKSIZE_MQTTPUB-1], TASK_PRIO_MQTTPUB);  
    /*MQTT���ݶ��ķ���*/
    OSTaskCreate(MQTTSub_task, (void *)p_arg, &STK_MQTTSUB[STKSIZE_MQTTSUB-1], TASK_PRIO_MQTTSUB); 
    
    while(1) {
        ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);
        if (tmp == PHY_LINK_OFF)
        {/*������ߴ���*/
            mqttparm->conn_status = OFF;
            LED_BLINK_ONCE(S_FAULT);
            //errorcode
            for(i=0;i<TOPIC_NUMMAX;i++)
            {
                MQTT_SubInfo[i].msgsta = OFF;
            }
            transport_close(mqttparm->tcpsocket);
        } 
        OSTimeDlyHMSM(0, 0, 0, 1); 
        switch(getSn_SR(mqttparm->tcpsocket))	//��ȡSOCK_MQTT��״̬
        {
            //socket��ʼ�����
            case SOCK_INIT:
                //errorcode
                for(i=0;i<TOPIC_NUMMAX;i++)
                {
                    MQTT_SubInfo[i].msgsta = OFF;
                }
                ret = connect(mqttparm->tcpsocket,mqttparm->ipgoal,mqttparm->portgoal);
                if(ret != ON)
                {
                    LED_BLINK_ONCE(S_FAULT);
                    //errorcode
                }
                OSTimeDlyHMSM(0, 0, 0, 100);
	            break;
            //socket���ӽ���
            case SOCK_ESTABLISHED:
                if(getSn_IR(mqttparm->tcpsocket) & Sn_IR_CON) 
                {
                    setSn_IR(mqttparm->tcpsocket, Sn_IR_CON);
                }
                if(!mqttparm->conn_status)
                {
                    tmp = MQTT_Connect(mqttparm->tcpsocket); 
                    if(tmp == ON)
                    {
                        OSTimeDlyHMSM(0, 0, 0, 100);//���ʱ�������
                        mqttparm->conn_status = tmp;
                    }
                    FEED_PINREQ(MQTT_CFG_Connect.kpalivespace);//ping����ʱ������
                    
                }
                else
                {
                    if((getSn_RX_RSR(mqttparm->tcpsocket))> 0)
                    {
                         MQTT_MsgHandler(&mqtt_recv_msg);//������Ϣ
                    }
                    for(i=0;i<TOPIC_NUMMAX;i++)
                    {
                        if(!MQTT_SubInfo[i].msgsta)
                        {//������Ϣ
                            LED_BLINK_ONCE(S_FAULT);
                            //errorcode
                            //MQTT_SubInfo[i].topic_name="board_sub";
                            MQTT_SubInfo[i].packid=0x1122;
                            MQTT_SubInfo[i].msgsta = mqtt_subscrib(mqttparm->tcpsocket,&MQTT_SubInfo[i],1);
                        }
                    }
                    
                    if(PinReqTime==0)
                    {
                        if(mqtt_pingreq(mqttparm->tcpsocket))//PING����
                        {
                            FEED_PINREQ(MQTT_CFG_Connect.kpalivespace);
                        }
                        else
                        {
                            LED_BLINK_ONCE(S_FAULT);
                            //errorcode
                        }
                    }
                }
            	break;
        //socket�ȴ��ر�״̬
            case SOCK_CLOSE_WAIT:
                
                mqttparm->conn_status = OFF;
                close(mqttparm->tcpsocket);
            	break;
        //socket�ر�
            case SOCK_CLOSED:
                mqttparm->conn_status = OFF;
                sock_status=0xff;
                while(sock_status!=mqttparm->tcpsocket)
                {
                    sock_status = socket(mqttparm->tcpsocket, Sn_MR_TCP, mqttparm->portlocal, Sn_MR_ND);
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
                mqttparm->conn_status = OFF;
            //connect(mqttparm->tcpsocket, gWIZNETINFO.ipgoal, gWIZNETINFO.portgoal);
                break;
        }       
    }
}


int MQTT_MsgHandler(struct mqtt_recv_t* recv)
{
	int rc,buflen = sizeof(Recvbuf);
    memset(Recvbuf,0,buflen);
	unsigned short msgid;
	recv->recv_type = MQTTPacket_read(Recvbuf, 500, transport_getdata);
    
    switch(recv->recv_type)
	{
        case CONNACK:
            break;
        case PUBACK:				  
            {  //���������а�������ӦPublish�еı��ı�ʶ��packetid ,return 2
                unsigned char packet_type,dup;                  
                rc = MQTTDeserialize_ack(&packet_type, &dup, &recv->mqttrecvmsg.packid,Recvbuf, buflen);   

            }
            break;
        case SUBACK:
            {    //SUBACK�Ƿ���������subscribe����������⼰QOS����ȷ�Ϻͻظ�
                int subcount;
                int granted_qos;
                rc = MQTTDeserialize_suback(&recv->mqttrecvmsg.packid, 1, &subcount, &granted_qos, Recvbuf, buflen);
                if(rc != buflen)
                {
                    LED_BLINK_ONCE(S_FAULT);
                    //errorcode
                }
                if (granted_qos != 0)
                {
                    //prinf("granted qos != 0, %d\n\r", granted_qos);
                }
            }
            break;
        case PINGRESP:
            break;
        case PUBLISH:
            {  //�ȴ� subscribe ���ĵ����� ���ݰ� Ӧ��,return 5
                int qos;
                unsigned char dup;
                unsigned char retained;
                int32_t lenPub,rcPub;                 
                unsigned char bufPub[100];
                int buflenPub = sizeof(bufPub);                                      

                rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &recv->mqttrecvmsg.topic_name,
                //*MQTTDeserialize_publish�ú�����ָ���ѽ��յ�xx���ݷ����л�
                &recv->mqttrecvmsg.msgdata, (int*)&recv->mqttrecvmsg.msglen, Recvbuf, buflen);
                                  
                lenPub = MQTTSerialize_puback(bufPub, buflenPub, msgid);
                rcPub = Ethernet_SendBuffer(gWIZNETINFO.tcpsocket, bufPub, lenPub);		
                if(rcPub != lenPub)
                {
                    LED_BLINK_ONCE(S_FAULT);
                    //errorcode
                }
            }			     
            break;
        default:
            break;				
    }
    
    if((recv->recv_type==PUBACK)||(recv->recv_type==SUBACK)||(recv->recv_type==PUBLISH))
    {
        OSMboxPost(EV_MQTTRECV,(void *)recv);//
    }
    return rc;
}

/*
 * ��������MQTTSub_task
 * ����  ��������ΪMQTTЭ��Ķ��Ķ�
 * ����  ����
 * ���  ����
 * ˵��  ����
 */

void MQTT_PublishMsg(struct MQTT_Topic_Info_t* Pmsg)
{
    OSQPost(PublishQueue,(void *)&Pmsg[0]);
}


/*
 * ��������MQTTSub_task
 * ����  ��������ΪMQTTЭ��Ķ��Ķ�
 * ����  ����
 * ���  ����
 * ˵��  ����
 */

u8 subbuf[MSG_MAXBYTENUM];//������Ϣ

void MQTTSub_task(void *p_arg)
{
	u8 err,i,matchflg;
    u8 topicidx;
    u16 j,canid;
    #ifdef CJSON_ENABLE
    u16 cnt;
    #endif
//     struct wiz_NetInfo_t *mqttparm;
    struct mqtt_recv_t* recvmsg;
    #ifdef CJSON_ENABLE
    cJSON *subjson = NULL;
    cJSON *node = NULL;
    #endif
    
//     mqttparm=(struct wiz_NetInfo_t*)p_arg;
    while(1)
    {
        OSTimeDlyHMSM(0, 0, 0, 2); 
        recvmsg=(struct mqtt_recv_t *)OSMboxPend(EV_MQTTRECV,0,&err);
		if(err == OS_ERR_NONE)
		{
            if(recvmsg->recv_type==PUBLISH)
            {
                matchflg=OFF;
                for(i=0;i<TOPIC_NUMMAX-1;i++)//�������ⲻ����
                {
                    for(j=0;j<strlen(MQTT_SubInfo[i].topic_name.cstring);j++)
                    {
                        if(recvmsg->mqttrecvmsg.topic_name.lenstring.data[j]!=MQTT_SubInfo[i].topic_name.cstring[j])
                        {
                            break;
                        }
                    }
                    if(j==strlen(MQTT_SubInfo[i].topic_name.cstring))
                    {
                        matchflg=ON;
                        topicidx = i;
                        break;
                    }

                }
                
                if(matchflg)//�������ã������˵���������Ҫ�������ݵ���Ϣ
                {
                    matchflg=OFF;
                    #ifdef CJSON_ENABLE
                        node = cJSON_Parse((const char*)recvmsg->mqttrecvmsg.msgdata);//����JSON
                        if(node == NULL)
                        {
                            LED_BLINK_ONCE(S_FAULT);
                            //errorcode  
                            return;
                        }
                        subjson = cJSON_GetObjectItem(node, "SN");//����SN��
                        if(subjson == NULL)
                        {
                            LED_BLINK_ONCE(S_FAULT);
                            //errorcode
                            return;
                        }
                        if(subjson->valueint == MQTT_SubInfo[topicidx].msgcnt)//�˴����˷��ɴ����������ϵ���һ��
                        {
                            MQTT_SubInfo[topicidx].msgcnt++;
                        }
                        else
                        {
                            MQTT_SubInfo[topicidx].msgcnt = subjson->valueint + 1;
                            LED_BLINK_ONCE(S_FAULT);
                            //errorcode
                        }
                        subjson = cJSON_GetObjectItem(node, "DType");//�������ͽ���
                        if(subjson == NULL)
                        {
                            LED_BLINK_ONCE(S_FAULT);
                            //errorcode
                            return;
                        }
                        
                        memset(subbuf,0,sizeof(subbuf));
                        if(strcmp(subjson->valuestring,"t_HEX")==0)
                        {
                            if((USARTCAN.Usart[topicidx][uartDatatype] == t_HEX)||((USARTCAN.can[canDatatype] == t_HEX)&&(topicidx == t_can)))
                            {//��ΪCAN�ĸ�ʽ�ʹ��ڵĲ�һ����������ʱ����Ҫ���⴦��һ��
                                cnt=0;
                                if(topicidx == t_can)
                                {
                                    subjson = cJSON_GetObjectItem(node, "CAN_ID");
                                
                                    if(subjson == NULL)
                                    {
                                        LED_BLINK_ONCE(S_FAULT);
                                        //errorcode
                                        return;
                                    }
                                    cnt = strlen(subjson->valuestring); 
                                    if(cnt>4)//CAN-IDֻ�������ֽ�
                                    {
                                        LED_BLINK_ONCE(S_FAULT);
                                        //errorcode
                                        return;
                                    }
                                    canid=0;
                                    for(j=0;j<cnt;j++)
                                    {
                                        canid |= (AscToHex(subjson->valuestring[j]))<<((cnt-j-1)*4);
                                    }
                                    subbuf[0] = canid/256;
                                    subbuf[1] = canid%256;//�Ƚ�IDȡ����
                                    
                                    subjson = cJSON_GetObjectItem(node, "Data");//�ٴ�����������
                                
                                    if(subjson == NULL)
                                    {
                                        LED_BLINK_ONCE(S_FAULT);
                                        //errorcode
                                        return;
                                    }
                                    cnt = strlen(subjson->valuestring); 
                                    if(cnt%2 != 0)
                                    {
                                        LED_BLINK_ONCE(S_FAULT);
                                        //errorcode
                                        return;
                                    }
                                    else if(cnt/2 > MSG_MAXBYTENUM)
                                    {
                                        LED_BLINK_ONCE(S_FAULT);
                                        //errorcode
                                        return;
                                    }
                                    
                                    for(j=0;j<cnt;j++)
                                    {//ǰ�������ֽ���ID
                                        subbuf[2+j/2] |= (j%2==0) ? (AscToHex(subjson->valuestring[j])<<4):AscToHex(subjson->valuestring[j]);
                                    }//��Ϊ�������ֽ�ΪID�����Ա���2
                                }
                                else
                                {
                                    subjson = cJSON_GetObjectItem(node, "Data");//�������ֱ�ӽ������ݼ���
                                    
                                    if(subjson == NULL)
                                    {
                                        LED_BLINK_ONCE(S_FAULT);
                                        //errorcode
                                        return;
                                    }
                                    cnt = strlen(subjson->valuestring); 
                                    if(cnt%2 != 0)
                                    {
                                        LED_BLINK_ONCE(S_FAULT);
                                        //errorcode
                                        return;
                                    }
                                    else if(cnt/2 > MSG_MAXBYTENUM)
                                    {
                                        LED_BLINK_ONCE(S_FAULT);
                                        //errorcode
                                        return;
                                    }
                                    
                                    for(j=0;j<cnt;j++)
                                    {
                                        subbuf[j/2] |= (j%2==0) ? (AscToHex(subjson->valuestring[j])<<4):AscToHex(subjson->valuestring[j]);
                                    }
                                }
                                
                                ETH2Usartcan_send(topicidx,&subbuf[0],cnt/2);//�ַ�תhex��ʱ��������һ��
                            }
                            else
                            {
                                LED_BLINK_ONCE(S_FAULT);
                                //errorcode
                            }
                        }
                        else if(strcmp(subjson->valuestring,"t_ASCII")==0)
                        {
                            if((USARTCAN.Usart[topicidx][uartDatatype] == t_ASCII)||((USARTCAN.can[canDatatype] == t_ASCII)&&(topicidx == t_can)))
                            {
                                if(topicidx == t_can)
                                {
                                    subjson = cJSON_GetObjectItem(node, "CAN_ID");
                                
                                    if(subjson == NULL)
                                    {
                                        LED_BLINK_ONCE(S_FAULT);
                                        //errorcode
                                        return;
                                    }
                                    cnt = strlen(subjson->valuestring); 
                                    if(cnt>4)//CAN-IDֻ�������ֽ�
                                    {
                                        LED_BLINK_ONCE(S_FAULT);
                                        //errorcode
                                        return;
                                    }
                                    canid=0;
                                    for(j=0;j<cnt;j++)
                                    {
                                        canid |= (AscToHex(subjson->valuestring[j]))<<((cnt-j-1)*4);
                                    }
                                    subbuf[0] = canid/256;
                                    subbuf[1] = canid%256;
                                    
                                    subjson = cJSON_GetObjectItem(node, "Data");
                                    
                                    if(subjson == NULL)
                                    {
                                        LED_BLINK_ONCE(S_FAULT);
                                        //errorcode
                                        return;
                                    }
                                    cnt = strlen(subjson->valuestring); 
                                    memcpy(&subbuf[2],&subjson->valuestring[0],cnt);
                                }
                                else
                                {    
                                    subjson = cJSON_GetObjectItem(node, "Data");//�������ֱ�ӽ�������
                                    
                                    if(subjson == NULL)
                                    {
                                        LED_BLINK_ONCE(S_FAULT);
                                        //errorcode
                                        return;
                                    }
                                    cnt = strlen(subjson->valuestring); 
                                    memcpy(subbuf,&subjson->valuestring[0],cnt);
                                }
                                if(topicidx<t_heartbeat)
                                {
                                    ETH2Usartcan_send(topicidx,&subbuf[0],cnt);//ͨ�����ڻ�CAN��������
                                }
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
                            return;
                        }
                        
                        cJSON_Delete(node);
                    #endif                      

                }
                
            }
        }
    }
}

void MQTT_DataHandler(u8 chnidx,struct USARTCAN_Recv_t *recv)
{
    
    #ifdef CJSON_ENABLE
    char buf[MSG_MAXBYTENUM*2];//һ���ֽ�ռ�������ַ����ʷŴ�һ��
    char *msg;
    u16 j,k,cnt;
    char canid[5];
    #endif
    
    #ifdef CJSON_ENABLE
    memset(&buf,'\0',sizeof(buf));
    mqttMsg[chnidx] = cJSON_CreateObject();
    cJSON_AddNumberToObject(mqttMsg[chnidx],"SN",MQTT_PubInfo[chnidx].msgcnt++);
    if(recv->lenth>MSG_MAXBYTENUM)
    {
        LED_BLINK_ONCE(S_FAULT);
        //errorcode
        return;
    }
    cJSON_AddStringToObject(mqttMsg[chnidx],"DType",DataType[recv->datatype]);//������������
    if(recv->datatype == t_HEX)
    {
        cnt=0;
        k=0;
        for(j=0;j<recv->lenth;j++)
        {
            k = sprintf( buf+cnt, "%02X", recv->databuf[j]);//��ʽ�����ַ���
            cnt += k;
        }
        
        if(chnidx == t_can)
        {//can�����ݸ�ʽ���⣬������������
            for(j=0;j<4;j++)
            {
                canid[j] = buf[j];
            }
            canid[4] = '\0';
            cJSON_AddStringToObject(mqttMsg[chnidx],"CAN_ID",canid);
            cJSON_AddStringToObject(mqttMsg[chnidx],"Data",&buf[4]);
        }
        else
        {
            cJSON_AddStringToObject(mqttMsg[chnidx],"Data",buf);
        }
    }
    else if(recv->datatype == t_ASCII)
    {        
        if(chnidx == t_can)
        {//can�����ݸ�ʽ���⣬������������
            cnt=0;
            k=0;
            for(j=0;j<4;j++)
            {
                k = sprintf( canid+cnt, "%02X", recv->databuf[j]);//��ʽ�����ַ���
                cnt += k;
            }
            canid[4] = '\0';
            cJSON_AddStringToObject(mqttMsg[chnidx],"CAN_ID",canid);
            memcpy(&buf,&recv->databuf[2],recv->lenth-2);
            cJSON_AddStringToObject(mqttMsg[chnidx],"Data",&buf[0]);
        }
        else
        {
            memcpy(&buf,&recv->databuf[0],recv->lenth);
            cJSON_AddStringToObject(mqttMsg[chnidx],"Data",buf);
        }
    }
    else
    {
        LED_BLINK_ONCE(S_FAULT);
        //errorcode
    }
    
    
    
    msg = cJSON_Print(mqttMsg[chnidx]);
    if(msg == NULL)
    {
        LED_BLINK_ONCE(S_FAULT);
        //errorcode
    }
    else
    {
        MQTT_PubInfo[chnidx].msgdata = (u8 *)msg;
        MQTT_PubInfo[chnidx].msglen = strlen((const char *)MQTT_PubInfo[chnidx].msgdata);
        //MQTT_PubInfo[idx].packid=0x1122;
        mqtt_publish(gWIZNETINFO.tcpsocket,&MQTT_PubInfo[chnidx]); //���������͵���Ϣ 
        FEED_PINREQ(MQTT_CFG_Connect.kpalivespace);
    }
    free(msg);
    cJSON_Delete(mqttMsg[chnidx]);//�����ͷſռ䣬����ᵼ��ջ���
    #else
    mqttMsg[chnidx] = recv->databuf;
    MQTT_PubInfo[chnidx].msgdata = (u8 *) mqttMsg[chnidx];
    MQTT_PubInfo[chnidx].msglen = recv->lenth;
    //MQTT_PubInfo[chnidx].packid=0x1122;
    MQTT_PublishMsg(&MQTT_PubInfo[chnidx]);
    #endif
}

/*
 * ��������MQTTSub_task
 * ����  ��������ΪMQTTЭ��Ķ��Ķ�
 * ����  ����
 * ���  ����
 * ˵��  ��֮ǰΪ��ʵ��������ʱ�Թ���ʱʹ����Ϣ�������洢δ�ɹ����͵�������ʱ��ʹ��
 */


void MQTTPub_task(void *p_arg)
{	
    struct wiz_NetInfo_t *mqttparm;
    struct MQTT_Topic_Info_t *pubmsg;
	u8 err;
    mqttparm = (struct wiz_NetInfo_t *)p_arg;
    OSQFlush(PublishQueue);
    
    while(1) 
    {
        OSTimeDlyHMSM(0, 0, 0, 2); 
        pubmsg=(struct MQTT_Topic_Info_t *)OSQPend(PublishQueue,0,&err);
		if(err == OS_ERR_NONE)
		{
            while(!mqttparm->conn_status)
            {
                OSTimeDlyHMSM(0, 0, 0, 2);
            }
            mqtt_publish(mqttparm->tcpsocket,pubmsg); //���������͵���Ϣ 
            #ifdef CJSON_ENABLE
            free(pubmsg->msgdata);
            #endif
            FEED_PINREQ(MQTT_CFG_Connect.kpalivespace);
		}
    }
}

