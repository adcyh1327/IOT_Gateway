#include "main.h"

const unsigned long SystemFrequency = 72000000; /* ��Ƶ72Mhz */

#define STKSIZE_MAIN                     STK_SIZE_64  //main�����Ķ�ջ��С������ֵ���޸�������
OS_STK STK_Buffer_main[STKSIZE_MAIN];


/*
** main������
*/
int main(void)
{   
    Config_DrCommon();  //��������ĳ�ʼ�����������ڡ���ʱ����W5500��IO��
    
    SysTick_Config(SystemFrequency/OS_TICKS_PER_SEC);  //TICK��ʼ��
	
    OSInit();  //OS��ʼ����
    
	OSTaskCreate(Task_Main,(void *)0,&STK_Buffer_main[STKSIZE_MAIN-1], TASK_PRIO_MAIN);//������task
    
	OSStart();//����OS��ϵͳ����OS�ӹܣ���������²���ִ�е���һ��
    return 0;
}


/*
** UCOSII��������֮ǰ�ĳ�ʼ��
*/
void Config_DrCommon(void)
{
    //RCC_DeInit();
    SystemInit();
    //NVIC_DeInit();
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);  //�����ж��������ƫ�Ƶ�ַ������bootloaderģʽ��
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3); //�жϼ���Ϊ1��һ��λ��ʾ��ռ����3��λ��ʾ�ж����ȼ�������ԽС����Խ��
    ReadFlashCfg();//��FLASH����
    DrLed();
    DrTimer_Init();
    DrUsart_Init();
    DrCAN_Init();
    #ifdef W5500_ENABLE
        W5500_SPI_Config();
        Ethernet_Init();
    #endif
    
    /* �������Ź���ʱ��=64/40k*312=500ms */
    
}




