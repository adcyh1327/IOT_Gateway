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
            LED_R(OFF);  //网口连接成功则常灭
            OSMboxPend(mBOX_LED_R,0,&err);
            if(OS_ERR_NONE == err)
            {
                IO_TRIGGER(LEDR_PORT,LEDR_PIN);
                OSTimeDlyHMSM(0, 0, 0, 80);   //出现故障的时候闪烁一次，持续80ms
            }
            
        }
        else
        {
            OSTimeDlyHMSM(0, 0, 0, 500);
            IO_TRIGGER(LEDR_PORT,LEDR_PIN);//LED灯闪烁，周期为1s
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
        LED_G(ON);  // 正常状态是亮的
        OSMboxPend(mBOX_LED_G,0,&err);
        if(OS_ERR_NONE == err)
        {
            IO_TRIGGER(LEDG_PORT,LEDG_PIN);
            OSTimeDlyHMSM(0, 0, 0, 40);   //有数据传输时时候闪烁一次，持续40ms
        }
        OSTimeDlyHMSM(0, 0, 0, 1);
    }
    
}



