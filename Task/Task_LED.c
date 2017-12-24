#include "main.h"

#include "Task_MQTT.h"

OS_EVENT *mBOX_LED_R;
OS_EVENT *mBOX_LED_G;
u8 led_blink=ON;

INT8U err;

void Task_LedR(void *p_arg)
{
    
    struct wiz_NetInfo_t *ethparm;
    
    ethparm = (struct wiz_NetInfo_t *)p_arg;
   
    mBOX_LED_R = OSMboxCreate((void *)0);
    
    while (1)
    {
        if(ethparm->conn_status)
        {
            LED_R(OFF);  //�������ӳɹ�����
            OSMboxPend(mBOX_LED_R,0,&err);
            if(OS_ERR_NONE == err)
            {
                IO_TRIGGER(LEDR_PORT,LEDR_PIN);
                OSTimeDlyHMSM(0, 0, 0, 80);   //���ֹ��ϵ�ʱ����˸һ�Σ�����80ms
            }
            
        }
        else
        {
            OSTimeDlyHMSM(0, 0, 0, 500);
            IO_TRIGGER(LEDR_PORT,LEDR_PIN);//LED����˸������Ϊ1s
            OSTimeDlyHMSM(0, 0, 0, 500);
            IO_TRIGGER(LEDR_PORT,LEDR_PIN);
        }
        OSTimeDlyHMSM(0, 0, 0, 1);
    }
    
}

void Task_LedG(void *p_arg)
{
//     struct wiz_NetInfo_t *ethparm;
//     ethparm = (struct wiz_NetInfo_t *)p_arg;
    mBOX_LED_G = OSMboxCreate((void *)0);
    
    while (1)
    {
        LED_G(ON);  // ����״̬������
        OSMboxPend(mBOX_LED_G,0,&err);
        if(OS_ERR_NONE == err)
        {
            IO_TRIGGER(LEDG_PORT,LEDG_PIN);
            OSTimeDlyHMSM(0, 0, 0, 40);   //�����ݴ���ʱʱ����˸һ�Σ�����40ms
        }
        OSTimeDlyHMSM(0, 0, 0, 1);
    }
    
}



