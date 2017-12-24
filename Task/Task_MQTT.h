#ifndef __TASK_MQTT_H
#define __TASK_MQTT_H
#include "main.h"

#ifdef CJSON_ENABLE
#include "cJSON.h"
#endif

#define N_PUBQUEUE          10   //��Ϣ���еĳ���

/*
enum c{
    RS232_1,RS232_2,RS485,CAN_CHN
};*/

//��ö��ǰ��4������������һ�£��ϱߵ���variable.h�ļ��ж���
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
	char UserName[20];//�û���
	char PassWard[20];//����
	MQTTString Client_id;//����ID
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

#define TOPICNAME_MAXLEN                60  //TOPIC ��������ֽ���
#define MSG_MAXBYTENUM              200  //������Ϣ�����ݣ������ֽ���

struct MQTT_Topic_Info_t
{
    MQTTString topic_name;   //��������
    u16 qos;
    u16 packid;         //�������ĵ���ϢID
    u8 *msgdata;        //��Ϣ����
    u16 msglen;         //��Ϣ����
    u8 msgsta;          //���ⶩ�Ļ򷢲���״̬
    u16 msgcnt;         //������������ļ���
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


