#ifndef __TASK_MQTT_H
#define __TASK_MQTT_H
#include "main.h"

#ifdef CJSON_ENABLE
#include "cJSON.h"
#endif

#define N_PUBQUEUE          10   //消息队列的长度

/*
enum c{
    RS232_1,RS232_2,RS485,CAN_CHN
};*/

//此枚举前边4项必须和上述的一致，上边的在variable.h文件中定义
enum MQTT_Topic
{
    t_rs232_1=0,t_rs232_2,t_rs485,t_can,t_heartbeat,
    TOPIC_NUMMAX
};

struct string_t
{
    char *cstring;
    u16 str_len;
};

struct MQTT_CFG_Connect_t
{
	char UserName[20];//用户名
	char PassWard[20];//密码
	MQTTString Client_id;//连接ID
    char con_qos;
    u16 kpalivespace;
};
extern struct MQTT_CFG_Connect_t MQTT_CFG_Connect;

struct PublishReq_Block_t
{
    char *Topic;
    char *Pubmsg;
    u16 msglen;
    u16 packid;
};

extern struct PublishReq_Block_t *PublishReq_Block[N_PUBQUEUE];
extern OS_EVENT *PublishQueue;

enum Pub_Msgldx {
    TOC_VOLTAGE,
    TOC_TEMPERATURE
};

#ifdef CJSON_ENABLE
extern cJSON *mqttMsg[TOPIC_NUMMAX] ;
#else
extern u8 *mqttMsg[TOPIC_NUMMAX] ;
#endif

#define TOPICNAME_MAXLEN                60  //TOPIC 名称最大字节数
#define MSG_MAXBYTENUM              200  //发布消息的内容：最大的字节数

struct MQTT_Topic_Info_t
{
    MQTTString topic_name;   //主题名称
    u16 qos;
    u16 packid;         //发布或订阅的消息ID
    u8 *msgdata;        //消息内容
    u16 msglen;         //消息长度
    u8 msgsta;          //主题订阅或发布的状态
    u16 msgcnt;         //发布或订阅主题的计数
};

extern struct MQTT_Topic_Info_t MQTT_PubInfo[TOPIC_NUMMAX];
extern struct MQTT_Topic_Info_t MQTT_SubInfo[TOPIC_NUMMAX];

struct mqtt_recv_t
{
    u16 recv_type;  //CONNACK/PUBACK/SUBACK....
    struct MQTT_Topic_Info_t mqttrecvmsg;
};
extern struct mqtt_recv_t mqtt_recv_msg;

void MQTT_Timer1ms(void);
void MQTT_task(void *p_arg);
void MQTT_DataHandler(u8 chnidx,struct USARTCAN_Recv_t *recv);
void MQTT_PublishMsg(struct MQTT_Topic_Info_t* Pmsg);

#endif


